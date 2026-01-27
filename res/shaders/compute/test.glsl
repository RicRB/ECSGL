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
} inputData;


layout(set = 0, binding = 1, rgba8) uniform writeonly image2D outputImage;

struct Object {
    vec3 position;
    float radius;
};

layout(std430, binding = 2) readonly buffer objectDataBlock {
    int objectCount;
    Object objects[];
} objectData;

struct Ray {
    float x, y, z, r, theta, phi;
    float dr, dtheta, dphi;
    float E, L;
};

const float SagA_rs = 1.269e10;
const float D_LAMBDA = 1e7;
const double ESCAPE_R = 1e30;

Ray initRay(vec3 pos, vec3 dir) {
    Ray ray;
    ray.x = pos.x; ray.y = pos.y; ray.z = pos.z;
    ray.r = length(pos);
    ray.theta = acos(pos.z / ray.r);
    ray.phi = atan(pos.y, pos.x);

    float dx = dir.x, dy = dir.y, dz = dir.z;
    ray.dr     = sin(ray.theta)*cos(ray.phi)*dx + sin(ray.theta)*sin(ray.phi)*dy + cos(ray.theta)*dz;
    ray.dtheta = (cos(ray.theta)*cos(ray.phi)*dx + cos(ray.theta)*sin(ray.phi)*dy - sin(ray.theta)*dz) / ray.r;
    ray.dphi   = (-sin(ray.phi)*dx + cos(ray.phi)*dy) / (ray.r * sin(ray.theta));

    ray.L = ray.r * ray.r * sin(ray.theta) * ray.dphi;
    float f = 1.0 - SagA_rs / ray.r;
    float dt_dL = sqrt((ray.dr*ray.dr)/f + ray.r*ray.r*(ray.dtheta*ray.dtheta + sin(ray.theta)*sin(ray.theta)*ray.dphi*ray.dphi));
    ray.E = f * dt_dL;

    return ray;
}

bool intercept(Ray ray, float rs) {
    return ray.r <= rs * 1.001;
}

void geodesicRHS(Ray ray, out vec3 d1, out vec3 d2) {
    float r = ray.r, theta = ray.theta;
    float dr = ray.dr, dtheta = ray.dtheta, dphi = ray.dphi;
    float f = 1.0 - SagA_rs / r;
    float dt_dL = ray.E / f;

    d1 = vec3(dr, dtheta, dphi);
    d2.x = - (SagA_rs / (2.0 * r*r)) * f * dt_dL * dt_dL
         + (SagA_rs / (2.0 * r*r * f)) * dr * dr
         + r * (dtheta*dtheta + sin(theta)*sin(theta)*dphi*dphi);
    d2.y = -2.0*dr*dtheta/r + sin(theta)*cos(theta)*dphi*dphi;
    d2.z = -2.0*dr*dphi/r - 2.0*cos(theta)/(sin(theta)) * dtheta * dphi;
}

bool crossesEquatorialPlane(vec3 oldPos, vec3 newPos) {
    bool crossed = (oldPos.y * newPos.y < 0.0);
    float r = length(vec2(newPos.x, newPos.z));
    return crossed && (r >= inputData.disk_r1 && r <= inputData.disk_r2);
}


void rk4Step(inout Ray ray, float dL) {
    // Save initial state
    float r0 = ray.r, theta0 = ray.theta, phi0 = ray.phi;
    float dr0 = ray.dr, dtheta0 = ray.dtheta, dphi0 = ray.dphi;
    
    // k1
    vec3 k1a, k1b;
    geodesicRHS(ray, k1a, k1b);
    
    // k2 (evaluate at midpoint with k1)
    ray.r = r0 + 0.5 * dL * k1a.x;
    ray.theta = theta0 + 0.5 * dL * k1a.y;
    ray.phi = phi0 + 0.5 * dL * k1a.z;
    ray.dr = dr0 + 0.5 * dL * k1b.x;
    ray.dtheta = dtheta0 + 0.5 * dL * k1b.y;
    ray.dphi = dphi0 + 0.5 * dL * k1b.z;
    vec3 k2a, k2b;
    geodesicRHS(ray, k2a, k2b);
    
    // k3 (evaluate at midpoint with k2)
    ray.r = r0 + 0.5 * dL * k2a.x;
    ray.theta = theta0 + 0.5 * dL * k2a.y;
    ray.phi = phi0 + 0.5 * dL * k2a.z;
    ray.dr = dr0 + 0.5 * dL * k2b.x;
    ray.dtheta = dtheta0 + 0.5 * dL * k2b.y;
    ray.dphi = dphi0 + 0.5 * dL * k2b.z;
    vec3 k3a, k3b;
    geodesicRHS(ray, k3a, k3b);
    
    // k4 (evaluate at endpoint with k3)
    ray.r = r0 + dL * k3a.x;
    ray.theta = theta0 + dL * k3a.y;
    ray.phi = phi0 + dL * k3a.z;
    ray.dr = dr0 + dL * k3b.x;
    ray.dtheta = dtheta0 + dL * k3b.y;
    ray.dphi = dphi0 + dL * k3b.z;
    vec3 k4a, k4b;
    geodesicRHS(ray, k4a, k4b);
    
    // Final RK4 combination
    ray.r = r0 + (dL / 6.0) * (k1a.x + 2.0*k2a.x + 2.0*k3a.x + k4a.x);
    ray.theta = theta0 + (dL / 6.0) * (k1a.y + 2.0*k2a.y + 2.0*k3a.y + k4a.y);
    ray.phi = phi0 + (dL / 6.0) * (k1a.z + 2.0*k2a.z + 2.0*k3a.z + k4a.z);
    ray.dr = dr0 + (dL / 6.0) * (k1b.x + 2.0*k2b.x + 2.0*k3b.x + k4b.x);
    ray.dtheta = dtheta0 + (dL / 6.0) * (k1b.y + 2.0*k2b.y + 2.0*k3b.y + k4b.y);
    ray.dphi = dphi0 + (dL / 6.0) * (k1b.z + 2.0*k2b.z + 2.0*k3b.z + k4b.z);
    
    // Update Cartesian coordinates
    ray.x = ray.r * sin(ray.theta) * cos(ray.phi);
    ray.y = ray.r * sin(ray.theta) * sin(ray.phi);
    ray.z = ray.r * cos(ray.theta);
}



// Globals to store hit info
vec4 objectColor = vec4(0.0);
vec3 hitCenter = vec3(0.0);
float hitRadius = 0.0;


float disk_num = 2;


float adaptiveStepSize(float r) {
    // Smaller steps near the Schwarzschild radius
    float minStep = 1e6;   // Very small near BH
    float maxStep = 1e7;   // MAX STEP SIZE
    
    // Distance from event horizon in units of rs
    float distFromHorizon = (r - SagA_rs) / SagA_rs;
    
    // Smoothly interpolate step size
    float t = clamp(distFromHorizon / 10.0, 0.0, 1.0);  // 10x rs = full step
    return mix(minStep, maxStep, t);
}

// Returns true on hit, captures center, radius, and base color
bool interceptObject(Ray ray) {
    vec3 P = vec3(ray.x, ray.y, ray.z);
    for (int i = 0; i < objectData.objectCount; ++i) {
        vec3 center = objectData.objects[i].position;
        float radius = objectData.objects[i].radius;
        if (distance(P, center) <= radius) {
            objectColor = vec4(1.0, 0.1, 0.2, 1.0);
            hitCenter = center;
            hitRadius = radius;
            return true;
        }
    }
    return false;
}

void main() {

    ivec2 pixelCoord = ivec2(gl_GlobalInvocationID.xy);

    ivec2 size = imageSize(outputImage);

    if (pixelCoord.x >= size.x || pixelCoord.y >= size.y) return;

    // Init Ray
    float u = (2.0 * (pixelCoord.x + 0.5) / inputData.width - 1.0) * inputData.aspect * inputData.tanHalfFov;
    float v = (1.0 - 2.0 * (pixelCoord.y + 0.5) / inputData.height) * inputData.tanHalfFov;
    vec3 dir = normalize(u * inputData.camRight - v * inputData.camUp + inputData.camForward);
    Ray ray = initRay(inputData.camPos, dir);

    vec4 color = vec4(0.0);
    vec3 prevPos = vec3(ray.x, ray.y, ray.z);
    float lambda = 0.0;

    bool hitBlackHole = false;
    bool hitDisk      = false;
    bool hitObject    = false;

    int steps = 60000;


    for (int i = 0; i < steps; ++i) {
        if (intercept(ray, SagA_rs)) { hitBlackHole = true; break; }
        float stepSize = adaptiveStepSize(ray.r);
        rk4Step(ray, stepSize);
        lambda += stepSize;

        vec3 newPos = vec3(ray.x, ray.y, ray.z);
        if (crossesEquatorialPlane(prevPos, newPos)) { hitDisk = true; break; }
        if (interceptObject(ray)) { hitObject = true; break; }
        prevPos = newPos;
        if (ray.r > ESCAPE_R) break;
    }


    if (hitDisk) {
    float r = length(vec2(ray.x, ray.z)); 
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
        vec3 P = vec3(ray.x, ray.y, ray.z);
        vec3 N = normalize(P - hitCenter);
        vec3 V = normalize(inputData.camPos - P);
        float ambient = 0.1;
        float diff = max(dot(N, V), 0.0);
        float intensity = ambient + (1.0 - ambient) * diff;
        vec3 shaded = objectColor.rgb * intensity;
        color = vec4(1.0, 1.0, 1.0, 1.0);
    }
    else {
        color = vec4(0.0, 0.0, 0.0, 1.0);
    }

    imageStore(outputImage, pixelCoord, color);
}