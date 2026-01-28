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
    int maxSteps;
    float stepSizeMin;
    float stepSizeMax;
    float escape_r;
    float rotationSpeed;
    float mass;

    int diskEnable;
    float diskInnerRadius;
    float diskOuterRadius;
    float diskH;
    float diskEdgeK;

	float diskDensity;

    float sigmaT;
    float dsVolScale;
    int volSubsteps;

    float time;
    float sampleScale;
    float spin;
    float gravAcceleration;
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

layout(set=0, binding = 4) uniform sampler3D noiseMap;
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

// Simplified gravitational acceleration (for Kerr metric) (omitting energy, angular momentum, and carter constant)
vec3 computeGravitationalAcceleration(vec3 position, float a, float M) {
    float r = length(position);
    if (r < 1e-3) return vec3(0.0, 0.0, 0.0);
    
    vec2 spherical = cartesianToSpherical(position);
    float theta = spherical.x;
    
    KerrMetricComponents metric = computeKerrMetric(r, theta, a, M);
    
    vec3 radialDir = normalize(position);
    float gravitationalAccel = inputData.gravAcceleration * -1.5 * M / (r * r);
    
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
float horizonRadius = 0;
float adaptiveStepSize(float r) {
    // Smaller steps near the Schwarzschild radius
    float minStep = inputData.stepSizeMin;   // Very small near BH
    float maxStep = inputData.stepSizeMax;   // MAX STEP SIZE
    
    // Distance from event horizon in units of rs
    float distFromHorizon = (r - horizonRadius) / horizonRadius;
    
    // Smoothly interpolate step size
    float t = clamp(distFromHorizon / 10.0, 0.0, 1.0);  // 10x rs = full step
    return mix(minStep, maxStep, t);
}

bool crossesEquatorialPlane(vec3 oldPos, vec3 newPos) {
    bool crossed = (oldPos.y * newPos.y < 0.0);
    float r = length(vec2(newPos.x, newPos.z));
    return crossed && (r >= inputData.diskInnerRadius && r <= inputData.diskOuterRadius);
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
        
    color = texture(sphereMapTexture, uv) - vec4(0.1, 0.1, 0.1, 0.1);


    return color;
}

float outDiskDensity;
vec3 outDiskEmissive;

void sampleDisk(vec3 p, vec3 rayDir, float M, float a, float time)
{
    // cylindrical coords
    vec2 xz = vec2(p.x, p.z);
    float r = length(xz);
    float h = p.y;
    float theta = atan(xz.y, xz.x);

    // ----- density calc -----
    float innerRadius = inputData.diskInnerRadius * M;
    float outerRadius = inputData.diskOuterRadius * M;

    float innerFade = smoothstep(innerRadius, innerRadius + M, r);
    float outerFade = 1.0 - smoothstep(outerRadius - inputData.diskEdgeK, outerRadius, r);
    float radialMask = innerFade * outerFade;

    float diskHeight = inputData.diskH * (1.0 + 0.3 * sqrt(r / (10.0 * M))); 
    float verticalProfile = exp(-(h * h) / (2.0 * diskHeight * diskHeight));

    float orbitalVelocity = sqrt(M / max(r, 0.001));

    float rotationRate = orbitalVelocity / r;
    float windingNumber = inputData.spin;

    float spiralAngle = theta - rotationRate * time * 2.0;

    spiralAngle += windingNumber * log(r / innerRadius);

    vec3 spiralCoords;
    spiralCoords.x = cos(spiralAngle) * (r - innerRadius) / (outerRadius - innerRadius);
    spiralCoords.y = h / diskHeight;
    spiralCoords.z = sin(spiralAngle) * (r - innerRadius) / (outerRadius - innerRadius);

    float noise1 = textureLod(noiseMap, spiralCoords * 1.0 * inputData.sampleScale, 0).r;
    float noise2 = textureLod(noiseMap, spiralCoords * 3.0 * inputData.sampleScale, 0).r;
    float noise3 = textureLod(noiseMap, spiralCoords * 7.0 * inputData.sampleScale, 0).r;

    float spiralPattern = sin(spiralAngle * windingNumber) * 0.5 + 0.5;
    spiralPattern = pow(spiralPattern, 2.0);

    float turbulence = noise1 + 0.5 * noise2 + 0.3 * noise3;
    turbulence = turbulence / 1.4;

    float structure = mix(turbulence, spiralPattern * turbulence, 0.5);

    float radialDensityBoost = 2.0 + 2.0 * exp(-(r - innerRadius) / (3.0 * M));
    
    outDiskDensity = inputData.diskDensity * verticalProfile * structure * radialDensityBoost * radialMask;

    float clumpiness = textureLod(noiseMap, spiralCoords * 0.5 + time * 0.1, 0).r;
    if (clumpiness > 0.7) {
        outDiskDensity *= 1.0 + (clumpiness - 0.7) * 3.0;
    }

    // ----- color/emission calculation -----

    float temperatureParam = 1.0 - clamp((r - innerRadius) / (outerRadius - innerRadius), 0.0, 1.0);
    temperatureParam = pow(temperatureParam, 2.0);

    // MAKE UI
    vec3 coolColor = vec3(0.1, 0.3, 0.9); 
    vec3 warmColor = vec3(0.1, 0.3, 0.9);
    vec3 hotColor = vec3(0.4, 0.6, 1.0);
    vec3 veryHotColor = vec3(0.7, 0.8, 1.0); 

    vec3 diskColor;

    if (temperatureParam < 0.25) {
        diskColor = mix(coolColor, warmColor, temperatureParam * 4.0);
    } else if (temperatureParam < 0.5) {
        diskColor = mix(warmColor, hotColor, (temperatureParam - 0.25) * 4.0);
    } else if (temperatureParam < 0.75) {
        diskColor = mix(hotColor, veryHotColor, (temperatureParam - 0.5) * 4.0);
    } else {
        diskColor = mix(veryHotColor, vec3(0.7, 0.8, 1.0), (temperatureParam - 0.75) * 4.0);
    }

    // ----- doppler effects -----
    vec3 tangent = normalize(vec3(-xz.y, 0, xz.x));
    vec3 velocity = tangent * orbitalVelocity;
    
    float beta = length(velocity);
    float gamma = 1.0 / sqrt(max(1.0 - beta * beta, 0.001));

    float cosAngle = dot(normalize(velocity), -rayDir);
    float dopplerFactor = gamma * (1.0 - beta * cosAngle);
    // relativistic beaming
    float dopplerBoost = pow(1.0 / max(dopplerFactor, 0.1), 2.0f);  

    if (cosAngle > 0) {
        // moving toward observer - blueshift
        diskColor = mix(diskColor, diskColor * vec3(0.7, 0.85, 1.2), cosAngle * 0.3);
        outDiskEmissive = diskColor * (1.0 + cosAngle * 0.5); // Brighter
    } else {
        // moving away - redshift
        diskColor = mix(diskColor, diskColor * vec3(1.2, 0.85, 0.7), -cosAngle * 0.3);
        outDiskEmissive = diskColor * (1.0 + cosAngle * 0.3); // Dimmer
    }

    // ----- gravitational effects -----

    float gravitationalRedshift = sqrt(1.0 - 2.0 * M / max(r, 2.0 * M));
    float compressionBoost = 1.0 + 1.0 * exp(-r / (5.0 * M));
    
    outDiskEmissive *= 2.0 * outDiskDensity * dopplerBoost * compressionBoost * gravitationalRedshift;
    
    // Hot spots and flares
    float flareNoise = texture(noiseMap, spiralCoords * 0.3 + time * 0.5).r;
    if (flareNoise > 0.85 && temperatureParam > 0.6) {
        outDiskEmissive *= 1.0 + (flareNoise - 0.85) * 5.0;
    }
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
    horizonRadius = M + sqrt(M * M - a * a);

    vec2 noiseCoord = vec2(pixelCoord) / 64.0;
    float blueNoise = fract(sin(dot(noiseCoord, vec2(12.9898, 78.233))) * 43758.5453);
    float jitter = (blueNoise - 0.5) * inputData.stepSizeMin * 0.5;

    RayState previousState = rayState;
    rayState.position += rayState.direction * jitter;
    float totalDist = 0.0;

    
    vec3 L = vec3(0.0);  // Accumulated radiance
    float T = 1.0;       // Transmittance

    for (int i = 0; i < inputData.maxSteps; ++i) {
        float radius = length(rayState.position);
        float stepSize = adaptiveStepSize(radius);
        totalDist += stepSize;
        if (radius > inputData.escape_r * M) break;
        if (totalDist > inputData.escape_r * 2.0 * M) break;
        
        if (radius <= horizonRadius * 1.001) {hitBlackHole = true; break;}

        RayState nextState = integrateGeodesicRK4(rayState, stepSize, a, M);

        if (inputData.diskEnable != 0 && T > 0.01) {
            vec3 seg = nextState.position - rayState.position;
            float segLen = length(seg);

            if (segLen > 0.0) {
                vec3 segDir = seg / segLen;
                int sub = max(1, inputData.volSubsteps);
                float ds = (segLen * inputData.dsVolScale) / float(sub);

                // March through the segment
                for (int s = 0; s < sub; ++s) {
                    float tMid = (float(s) + 0.5) / float(sub);
                    vec3 pMid = rayState.position + seg * tMid;
                    
                    vec2 xz = vec2(pMid.x, pMid.z);
                    float rperp = length(xz);
                    float h = abs(pMid.y);

                    // Check if we're in the disk region
                    float innerR = inputData.diskInnerRadius * 0.8;
                    float outerR = inputData.diskOuterRadius * 1.2;
                    float maxH = inputData.diskH * 5.0;

                    if (rperp >= innerR && rperp <= outerR && h <= maxH) {
                        // Sample the disk density and emission
                        sampleDisk(pMid, segDir, M, a, inputData.time);
                        
                        float density = outDiskDensity;
                        vec3 emissive = outDiskEmissive;

                        if (density > 0.001) {
                            // Calculate light extinction
                            float extinction = inputData.sigmaT * density * ds;
                            float atten = exp(-extinction);

                            // Emission-absorption integration
                            vec3 src = emissive * density;

                            if (extinction > 1e-5) {
                                // Analytical integration
                                float w = (1.0 - atten) / extinction;
                                L += T * src * inputData.sigmaT * w * ds;
                            } else {
                                // Direct contribution (low extinction)
                                L += T * src * inputData.sigmaT * ds;
                            }

                            // Update transmittance
                            T *= atten;
                            if (T < 0.01) break;
                        }
                    }
                }
            }
        }
        if (interceptObject(rayState)) { hitObject = true; break; }
        
        previousState = rayState;
        rayState = nextState;
    }


    vec3 finalColor = vec3(0.0, 0.0, 0.0);
    if (hitBlackHole) {
        if (inputData.diskEnable == 0)
        {
            finalColor = vec3(0.0, 0.0, 0.0);
        }else{
            finalColor = L;
        }

    } else if (hitObject) {
        // Compute shading
        vec3 P = rayState.position;
        vec3 N = normalize(P - hitCenter);
        vec3 V = normalize(inputData.camPos - P);
        float ambient = 0.1;
        float diff = max(dot(N, V), 0.0);
        float intensity = ambient + (1.0 - ambient) * diff;
        vec3 shaded = objectColor.rgb * intensity;
        
        color = vec4(L + T * shaded, objectColor.a);
        imageStore(outputImage, pixelCoord, color);
        return;
    }
    else {
        vec3 bg = sampleBackground(rayState.direction).rgb;
        finalColor = L + T * bg;
    }
    color = vec4(finalColor, 1.0);
    imageStore(outputImage, pixelCoord, color);
}