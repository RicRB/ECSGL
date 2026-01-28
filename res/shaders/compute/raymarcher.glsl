#version 450

layout (local_size_x = 8, local_size_y = 8) in;

//layout(std430, binding = 0) readonly buffer inputBuffer {
//    int numbers[];
//} inputData;
layout(std430, binding = 0) readonly buffer inputDataBlock {
    vec3 camPos;     float _pad0;
    vec3 camRight;   float _pad1;
    vec3 camUp;      float _pad2;
    vec3 camForward; float _pad3;
    float tanHalfFov;
    float aspect;
    int width;
    int height;
    float disk_r1;
    float disk_r2;
    int maxSteps;
    float stepSizeMin;
    float stepSizeMax;
    float r_s;
    float escape_r;
    float rotationSpeed;
    float mass;
} inputData;


layout(set = 0, binding = 1, rgba8) uniform writeonly image2D outputImage;

struct Object {
    vec3 position;
    float radius;
    vec4 color;
};

layout(std430, binding = 2) readonly buffer objectDataBlock {
    int objectCount;
    Object objects[];
} objectData;

layout(set = 0, binding = 3) uniform sampler2D sphereMapTexture;

float disk_num = 2;

// Helper function to convert Cartesian to spherical coordinates
vec2 cartesianToSpherical(vec3 pos) {
    float r = length(pos);
    float theta = acos(pos.y / r);
    float phi = atan(pos.z, pos.x);
    return vec2(theta, phi);
}

// Kerr metric components (if you want rotating black hole support)
struct KerrMetricComponents {
    float alpha;    // lapse function
    float omega;    // frame dragging
    float varpi;    // cylindrical radius
    float rho2;
    float Delta;
    float Sigma;
};

KerrMetricComponents computeKerrMetric(float r, float theta, float a, float M) {
    KerrMetricComponents comp;
    
    comp.rho2 = r * r + a * a * cos(theta) * cos(theta);
    comp.Delta = r * r - 2.0 * M * r + a * a;
    comp.Sigma = sqrt(pow(r * r + a * a, 2.0) - a * a * comp.Delta * sin(theta) * sin(theta));
    
    comp.alpha = sqrt(comp.rho2 * comp.Delta) / comp.Sigma;
    comp.omega = 2.0 * a * r / (comp.Sigma * comp.Sigma);
    comp.varpi = comp.Sigma * sin(theta) / sqrt(comp.rho2);
    
    return comp;
}

// Simplified gravitational acceleration (for Kerr metric)
vec3 computeGravitationalAcceleration(vec3 position, float a, float M) {
    float r = length(position);
    if (r < 1e-3) return vec3(0.0, 0.0, 0.0);
    
    vec2 spherical = cartesianToSpherical(position);
    float theta = spherical.x;
    
    KerrMetricComponents metric = computeKerrMetric(r, theta, a, M);
    
    vec3 radialDir = normalize(position);
    float gravitationalAccel = -1.5 * M / (r * r);
    
    // Frame dragging
    vec3 spinAxis = vec3(0.0, 1.0, 0.0);
    vec3 tangentialDir = normalize(cross(spinAxis, radialDir));
    float frameDragAccel = a * M / (r * r * r);
    
    return gravitationalAccel * radialDir + frameDragAccel * tangentialDir;
}

// Ray state structure
struct RayState {
    vec3 position;
    vec3 direction;
};

// RK4 geodesic integration
RayState integrateGeodesicRK4(RayState state, float stepSize, float a, float M) {
    vec3 pos = state.position;
    vec3 dir = state.direction;
    
    // k1 = f(t, y)
    vec3 k1_pos = dir;
    vec3 k1_dir = computeGravitationalAcceleration(pos, a, M);
    
    // k2 = f(t + h/2, y + h*k1/2)
    vec3 k2_pos = dir + 0.5 * stepSize * k1_dir;
    vec3 k2_dir = computeGravitationalAcceleration(pos + 0.5 * stepSize * k1_pos, a, M);
    
    // k3 = f(t + h/2, y + h*k2/2)
    vec3 k3_pos = dir + 0.5 * stepSize * k2_dir;
    vec3 k3_dir = computeGravitationalAcceleration(pos + 0.5 * stepSize * k2_pos, a, M);
    
    // k4 = f(t + h, y + h*k3)
    vec3 k4_pos = dir + stepSize * k3_dir;
    vec3 k4_dir = computeGravitationalAcceleration(pos + stepSize * k3_pos, a, M);
    
    state.position += (stepSize / 6.0) * (k1_pos + 2.0*k2_pos + 2.0*k3_pos + k4_pos);
    state.direction += (stepSize / 6.0) * (k1_dir + 2.0*k2_dir + 2.0*k3_dir + k4_dir);
    state.direction = normalize(state.direction);
    
    return state;
}

// Simplified Euler integration (faster)
RayState integrateGeodesicSimple(RayState state, float stepSize, float a, float M) {
    vec3 acceleration = computeGravitationalAcceleration(state.position, a, M);
    
    // Update direction first (Euler integration)
    state.direction += acceleration * stepSize;
    state.direction = normalize(state.direction);
    
    // Then update position
    state.position += state.direction * stepSize;
    
    return state;
}

float adaptiveStepSize(float r) {
    // Smaller steps near the Schwarzschild radius
    float minStep = inputData.stepSizeMin;   // Very small near BH
    float maxStep = inputData.stepSizeMax;   // MAX STEP SIZE
    
    // Distance from event horizon in units of rs
    float distFromHorizon = (r - inputData.r_s) / inputData.r_s;
    
    // Smoothly interpolate step size
    float t = clamp(distFromHorizon / 10.0, 0.0, 1.0);  // 10x rs = full step
    return mix(minStep, maxStep, t);
}

bool crossesEquatorialPlane(vec3 oldPos, vec3 newPos) {
    bool crossed = (oldPos.y * newPos.y < 0.0);
    float r = length(vec2(newPos.x, newPos.z));
    return crossed && (r >= inputData.disk_r1 && r <= inputData.disk_r2);
}



// Globals to store hit info
vec4 objectColor = vec4(0.0);
vec3 hitCenter = vec3(0.0);
float hitRadius = 0.0;

// Returns true on hit
bool interceptObject(RayState rayState) {
    vec3 P = rayState.position;
    for (int i = 0; i < objectData.objectCount; ++i) {
        vec3 center = objectData.objects[i].position;
        float radius = objectData.objects[i].radius;
        if (distance(P, center) <= radius) {
            // Store color - you'll need to add color to Object struct!
            objectColor = objectData.objects[i].color;  // Temp orange color
            hitCenter = center;
            hitRadius = radius;
            return true;
        }
    }
    return false;
}

vec2 directionToEquirectangularUV(vec3 direction)
{
    const float PI = 3.14159265359;

    vec3 d = normalize(direction);
    float theta = asin(clamp(d.y, -1.0, 1.0));
    float phi = atan(d.z, d.x);

    float u = phi / (2.0 * PI) + 0.5;
    float v = theta / PI + 0.5;
    
    return vec2(u, v);
}

vec4 sampleBackground(vec3 direction)
{
    vec4 color = vec4(0.0);

    vec2 uv = directionToEquirectangularUV(direction);
        
    color = texture(sphereMapTexture, uv);


    return color;
}

void main() {

    ivec2 pixelCoord = ivec2(gl_GlobalInvocationID.xy);

    ivec2 size = imageSize(outputImage);

    if (pixelCoord.x >= size.x || pixelCoord.y >= size.y) return;

    // Init Ray
    float u = (2.0 * (pixelCoord.x + 0.5) / inputData.width - 1.0) * inputData.aspect * inputData.tanHalfFov;
    float v = (1.0 - 2.0 * (pixelCoord.y + 0.5) / inputData.height) * inputData.tanHalfFov;
    vec3 dir = normalize(u * inputData.camRight - v * inputData.camUp + inputData.camForward);
    RayState rayState;
    rayState.position = inputData.camPos;
    rayState.direction = dir;

    vec4 color = vec4(0.0);

    bool hitBlackHole = false;
    bool hitDisk      = false;
    bool hitObject    = false;

    const float M = inputData.mass;
    const float a = inputData.rotationSpeed * M;
    RayState previousState = rayState;
    rayState.position += rayState.direction * inputData.stepSizeMin;
    float totalDist = 0.0;

    if (objectData.objects[0].radius == 10.0) {
        color = vec4(1.0, 0.0, 0.0, 1.0);
        imageStore(outputImage, pixelCoord, color);
    }
    for (int i = 0; i < inputData.maxSteps; ++i) {
        float radius = length(rayState.position);
        float stepSize = adaptiveStepSize(radius);
        totalDist += stepSize;
        if (radius <= inputData.r_s * 1.001) {hitBlackHole = true; break;}

        RayState nextState = integrateGeodesicRK4(rayState, stepSize, a, M);

        if (crossesEquatorialPlane(previousState.position, nextState.position)) { hitDisk = true; break; }


        if (interceptObject(rayState)) { hitObject = true; break; }
        
        previousState = rayState;
        rayState = nextState;
        if (radius > inputData.escape_r) break;
        if (totalDist > inputData.escape_r * 2.0) break;
    }


    if (hitDisk) {
    float r = length(vec2(rayState.position.x, rayState.position.z)); 
    float normalizedR = (r - inputData.disk_r1) / (inputData.disk_r2 - inputData.disk_r1);
    
    // Smooth edges
    float innerFade = smoothstep(0.0, 0.05, normalizedR);
    float outerFade = smoothstep(1.0, 0.95, normalizedR);
    float alpha = innerFade * outerFade;
    
    vec3 diskColor = vec3(1.0, normalizedR, 0.2);
    color = vec4(diskColor, alpha);

    } else if (hitBlackHole) {
        color = vec4(0.0, 0.0, 0.0, 1.0);
    } else if (hitObject) {
        // Compute shading
        vec3 P = rayState.position;
        vec3 N = normalize(P - hitCenter);
        vec3 V = normalize(inputData.camPos - P);
        float ambient = 0.1;
        float diff = max(dot(N, V), 0.0);
        float intensity = ambient + (1.0 - ambient) * diff;
        vec3 shaded = objectColor.rgb * intensity;
        color = vec4(shaded, objectColor.a);
    }
    else {
        color = sampleBackground(rayState.direction);

    }

    imageStore(outputImage, pixelCoord, color);
}