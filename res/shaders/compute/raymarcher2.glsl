#version 450

layout (local_size_x = 16, local_size_y = 16) in;

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
    int maxSteps;
    float stepSizeMin;
    float stepSizeMax;
    float rotationSpeed;
    float mass;

    float disk_InnerRadius; // x = inner radius, y outer radius, z = height
    float disk_OuterRadius;
    float disk_Height;
    float disk_Density;
    float disk_ExpoFactor;
    float disk_AnisotropyForward;
    float disk_AnisotropyBackward;
    float disk_LobeWeight;
    float disk_Brightness;
    float disk_LightIntensity;
    float disk_RotationSpeed;
    float disk_VerticalFadeStart;
    float disk_VerticalFadeEnd;
    float disk_OuterFadeStart;
    float disk_OuterFadeEnd;
    float disk_InnerFadeRadius;
    float disk_InnerFadeWidth;
    float disk_LightFalloff;
    float disk_DopplerStrength;

    int disk_MaxShadowIterations;
    float disk_ShadowStepSize;

    float dsVolScale;
    int volSubsteps;

    float time;
} inputData;

struct Object {
    vec3 position;
    float radius;
    vec4 color;
};

layout(std430, binding = 1) readonly buffer objectDataBlock {
    int objectCount;
    Object objects[];
} objectData;

layout(set = 0, binding = 2) uniform sampler2D sphereMapTexture;

layout(set=0, binding = 3) uniform sampler3D noiseMap;

layout(set=0, binding = 4, rg16f) uniform writeonly image2D directionTexture;
layout(set=0, binding = 5, rgba16) uniform writeonly image2D volumeTexture;
layout(set=0, binding = 6) uniform sampler2D blueNoiseTexture;

const float PI = 3.1415926538;
const float eps = 0.01;  // Hamiltonian gradient step
const vec2 dq = vec2(0, eps);
const float cdist = 120;
const float diskBaseRotationSpeed = 1;
const vec3 hotColor = vec3(0.4, 0.6, 1.0);

float a;
float m;
float dt = 0.1;
float stepNoise;
float horizonRadius = 0;
// Globals to store hit info
vec4 objectColor = vec4(0.0);
vec3 hitCenter = vec3(0.0);
float hitRadius = 0.0;


// Helper function to convert Cartesian to spherical coordinates
vec2 cartesianToSpherical(vec3 pos) {
    float r = length(pos);
    float theta = acos(pos.y / r);
    float phi = atan(pos.z, pos.x);
    return vec2(theta, phi);
}

float distanceFromCenter(vec4 position)
{
    vec3 pos = vec3(position.y, position.z, position.w);
    return length(pos);
}

vec3 getRealPosition(vec4 position)
{
    return vec3(position.y, position.z, position.w);
}


mat4 diag(vec4 a)
{
    return 
    mat4(a.x,0,0,0,
         0,a.y,0,0,
         0,0,a.z,0,
         0,0,0,a.w);
}
float sphIntersect(in vec3 ro, in vec3 rd, in vec4 sph)
{
    vec3 oc = ro - sph.xyz;
    float b = dot(oc, rd);
    float c = dot(oc, oc) - sph.w * sph.w;
    float h = b * b - c;
    if (h < 0.0)
        return 1e10;
    return -b - sqrt(h);
}
/*
//optimized inverse of symmetric matrix
mat4 inverse_sym(mat4 m) {
	float n11 = m[0][0], n12 = m[1][0], n13 = m[2][0], n14 = m[3][0];
	float n22 = m[1][1], n23 = m[2][1], n24 = m[3][1];
	float n33 = m[2][2], n34 = m[3][2];
	float n44 = m[3][3];

	float t11 = 2.0 * n23 * n34 * n24 - n24 * n33 * n24 - n22 * n34 * n34 - n23 * n23 * n44 + n22 * n33 * n44;
	float t12 = n14 * n33 * n24 - n13 * n34 * n24 - n14 * n23 * n34 + n12 * n34 * n34 + n13 * n23 * n44 - n12 * n33 * n44;
	float t13 = n13 * n24 * n24 - n14 * n23 * n24 + n14 * n22 * n34 - n12 * n24 * n34 - n13 * n22 * n44 + n12 * n23 * n44;
	float t14 = n14 * n23 * n23 - n13 * n24 * n23 - n14 * n22 * n33 + n12 * n24 * n33 + n13 * n22 * n34 - n12 * n23 * n34;

	float det = n11 * t11 
    + n12 * t12 
    + n13 * t13 
    + n14 * t14;
	float idet = 1.0f / det;

	mat4 ret;

	ret[0][0] = t11 * idet;
	ret[0][1] = (n24 * n33 * n14 - n23 * n34 * n14 - n24 * n13 * n34 + n12 * n34 * n34 + n23 * n13 * n44 - n12 * n33 * n44) * idet;
	ret[0][2] = (n22 * n34 * n14 - n24 * n23 * n14 + n24 * n13 * n24 - n12 * n34 * n24 - n22 * n13 * n44 + n12 * n23 * n44) * idet;
	ret[0][3] = (n23 * n23 * n14 - n22 * n33 * n14 - n23 * n13 * n24 + n12 * n33 * n24 + n22 * n13 * n34 - n12 * n23 * n34) * idet;

	ret[1][0] = ret[0][1];
	ret[1][1] = (2.0 * n13 * n34 * n14 - n14 * n33 * n14 - n11 * n34 * n34 - n13 * n13 * n44 + n11 * n33 * n44) * idet;
	ret[1][2] = (n14 * n23 * n14 - n12 * n34 * n14 - n14 * n13 * n24 + n11 * n34 * n24 + n12 * n13 * n44 - n11 * n23 * n44) * idet;
	ret[1][3] = (n12 * n33 * n14 - n13 * n23 * n14 + n13 * n13 * n24 - n11 * n33 * n24 - n12 * n13 * n34 + n11 * n23 * n34) * idet;

	ret[2][0] = ret[0][2];
	ret[2][1] = ret[1][2];
    ret[2][2] = (2.0 * n12 * n24 * n14 - n14 * n22 * n14 - n11 * n24 * n24 - n12 * n12 * n44 + n11 * n22 * n44) * idet;
	ret[2][3] = (n13 * n22 * n14 - n12 * n23 * n14 - n13 * n12 * n24 + n11 * n23 * n24 + n12 * n12 * n34 - n11 * n22 * n34) * idet;

	ret[3][0] = ret[0][3];
	ret[3][1] = ret[1][3];
	ret[3][2] = ret[2][3];
	ret[3][3] = (2.0 * n12 * n23 * n13 - n13 * n22 * n13 - n11 * n23 * n23 - n12 * n12 * n33 + n11 * n22 * n33) * idet;

	return ret;
}
*/


//optimized inverse of symmetric matrix
mat4 inverse_sym(mat4 m) {
	float n11 = m[0][0], n12 = m[1][0], n13 = m[2][0], n14 = m[3][0];
	float n22 = m[1][1], n23 = m[2][1], n24 = m[3][1];
	float n33 = m[2][2], n34 = m[3][2];
	float n44 = m[3][3];

	float t11 = 2.0 * n23 * n34 * n24 - n24 * n33 * n24 - n22 * n34 * n34 - n23 * n23 * n44 + n22 * n33 * n44;

	float idet = 1.0f / (
        n11 * t11
        + n12 * (n14 * n33 * n24 - n13 * n34 * n24 - n14 * n23 * n34 + n12 * n34 * n34 + n13 * n23 * n44 - n12 * n33 * n44)
        + n13 * (n13 * n24 * n24 - n14 * n23 * n24 + n14 * n22 * n34 - n12 * n24 * n34 - n13 * n22 * n44 + n12 * n23 * n44) 
        + n14 * (n14 * n23 * n23 - n13 * n24 * n23 - n14 * n22 * n33 + n12 * n24 * n33 + n13 * n22 * n34 - n12 * n23 * n34)
    );

	mat4 ret;

	ret[0][0] = t11 * idet;
	ret[0][1] = (n24 * n33 * n14 - n23 * n34 * n14 - n24 * n13 * n34 + n12 * n34 * n34 + n23 * n13 * n44 - n12 * n33 * n44) * idet;
	ret[0][2] = (n22 * n34 * n14 - n24 * n23 * n14 + n24 * n13 * n24 - n12 * n34 * n24 - n22 * n13 * n44 + n12 * n23 * n44) * idet;
	ret[0][3] = (n23 * n23 * n14 - n22 * n33 * n14 - n23 * n13 * n24 + n12 * n33 * n24 + n22 * n13 * n34 - n12 * n23 * n34) * idet;

	ret[1][0] = ret[0][1];
	ret[1][1] = (2.0 * n13 * n34 * n14 - n14 * n33 * n14 - n11 * n34 * n34 - n13 * n13 * n44 + n11 * n33 * n44) * idet;
	ret[1][2] = (n14 * n23 * n14 - n12 * n34 * n14 - n14 * n13 * n24 + n11 * n34 * n24 + n12 * n13 * n44 - n11 * n23 * n44) * idet;
	ret[1][3] = (n12 * n33 * n14 - n13 * n23 * n14 + n13 * n13 * n24 - n11 * n33 * n24 - n12 * n13 * n34 + n11 * n23 * n34) * idet;

	ret[2][0] = ret[0][2];
	ret[2][1] = ret[1][2];
    ret[2][2] = (2.0 * n12 * n24 * n14 - n14 * n22 * n14 - n11 * n24 * n24 - n12 * n12 * n44 + n11 * n22 * n44) * idet;
	ret[2][3] = (n13 * n22 * n14 - n12 * n23 * n14 - n13 * n12 * n24 + n11 * n23 * n24 + n12 * n12 * n34 - n11 * n22 * n34) * idet;

	ret[3][0] = ret[0][3];
	ret[3][1] = ret[1][3];
	ret[3][2] = ret[2][3];
	ret[3][3] = (2.0 * n12 * n23 * n13 - n13 * n22 * n13 - n11 * n23 * n23 - n12 * n12 * n33 + n11 * n22 * n33) * idet;

	return ret;
}

// Valid from 1000 to 40000 K (and additionally 0 for pure full white)
vec3 colorTemperatureToRGB(const in float temperature){
  // Values from: http://blenderartists.org/forum/showthread.php?270332-OSL-Goodness&p=2268693&viewfull=1#post2268693   
  mat3 m = (temperature <= 6500.0) ? mat3(vec3(0.0, -2902.1955373783176, -8257.7997278925690),
	                                      vec3(0.0, 1669.5803561666639, 2575.2827530017594),
	                                      vec3(1.0, 1.3302673723350029, 1.8993753891711275)) : 
	 								 mat3(vec3(1745.0425298314172, 1216.6168361476490, -8257.7997278925690),
   	                                      vec3(-2666.3474220535695, -2173.1012343082230, 2575.2827530017594),
	                                      vec3(0.55995389139931482, 0.70381203140554553, 1.8993753891711275)); 
  return mix(clamp(vec3(m[0] / (vec3(clamp(temperature, 1000.0, 40000.0)) + m[1]) + m[2]), vec3(0.0), vec3(1.0)), vec3(1.0), smoothstep(1000.0, 0.0, temperature));
}

mat3 rotY(float ang)
{
    float c = cos(ang);
    float s = sin(ang);
    return mat3(
        c, 0, -s,
        0, 1,  0,
        s, 0,  c
    );
}


////////////////////////////////////
/// KERR
////////////////////////////////////

mat4 G(vec4 q)
{
    float Q = 0.0; // charge
    //Kerr metric in Kerr-Schild coordinates with spin along Y axis
    vec3 p = q.yzw;
    float rho = dot(p,p) - a*a;
    float r2 = 0.5*(rho + sqrt(rho*rho + 4.0*a*a*p.y*p.y));  // Changed p.z → p.y
    float r = sqrt(r2);
    
    vec4 k = vec4(1, 
        (r*p.x + a*p.z)/(r2 + a*a),   // Changed p.y → p.z
        p.y/r,                         // Changed p.z/r → p.y/r
        (r*p.z - a*p.x)/(r2 + a*a));  // Changed (r*p.y - a*p.x) → (r*p.z - a*p.x)
    
    float f = smoothstep(cdist*0.5, 0.0, r)*r2*(2.0*m*r - Q*Q)/(r2*r2 + a*a*p.y*p.y);  // Changed p.z → p.y
    return f*mat4(k.x*k, k.y*k, k.z*k, k.w*k)+diag(vec4(-1,1,1,1));    
}

//vector to momentum at point q
vec4 Vec2P(vec3 v, vec3 x)
{
    return 2.0*G(vec4(0.0, x))*vec4(1.0, v); 
}

vec4 P2Vec(vec4 p, vec4 q)
{
    return inverse_sym(G(q))*p;
}
bool variDT(vec4 P, vec4 q, out bool blackholeHit)
{
    //Kerr metric in Kerr-Schild coordinates 
    vec3 p = q.yzw;
    float rho = dot(p,p) - a*a;
    float r2 = 0.5*(rho + sqrt(rho*rho + 4.0*a*a*p.y*p.y));  // Changed p.z → p.y
    float r = sqrt(r2);
    float diskInfluence = horizonRadius;
    dt = mix(inputData.stepSizeMin, inputData.stepSizeMax, pow(max(r - diskInfluence, 0.0)/cdist, 1.0));
    
    if(r < horizonRadius * 0.8 && a <= 1.0)
    {
        blackholeHit = true;
        return true;
    }

    if(length(q.yzw) > cdist || length(P) > 45.0) 
    {
        blackholeHit = false;
        return true;
    }
    return false;
}
//lagrangian
float L(vec4 qt, mat4 g)
{
    return dot(g*qt,qt);
}

float L(vec4 qt, vec4 q)
{
    mat4 g = G(q);
    return L(qt, g);
}


bool getRay(vec2 uv, out vec3 ro, out vec3 rd)
{
    // Ray origin = camera position
    ro = inputData.camPos;

    // Convert NDC uv → camera direction
    float u = (2.0 * uv.x - 1.0) * inputData.aspect * inputData.tanHalfFov;
    float v = (1.0 - 2.0 * uv.y) * inputData.tanHalfFov;

    rd = normalize(
          u * inputData.camRight
        - v * inputData.camUp
        + inputData.camForward
    );

    return true;
}

//step
mat2x4 dhstep(in mat2x4 s, float dt)
{
    vec4 p = s[0], q = s[1];
    
    mat4 g = G(q);
    mat4 ginv = inverse_sym(g);//slow
    vec4 qt = ginv*p;
    
    vec4 dHdq = -(vec4(L(qt,q+dq.yxxx),L(qt,q+dq.xyxx),L(qt,q+dq.xxyx),L(qt,q+dq.xxxy))-L(qt, g))/eps; 
    
    mat2x4 dqp;
    dqp[0] = -dHdq*dt;
    dqp[1] = 2.0*ginv*p*dt;
    
    return dqp;
}


float adaptiveStepSize(float r) {
    // Smaller steps near the Schwarzschild radius
    float minStep = inputData.stepSizeMin;   // Very small near BH
    float maxStep = inputData.stepSizeMax;   // MAX STEP SIZE
    
    // Distance from event horizon in units of rs
    float distFromHorizon = (r - horizonRadius) / horizonRadius;
    
    // Smoothly interpolate step size
    float t = clamp(distFromHorizon / 5.0, 0.0, 1.0);  // 10x rs = full step
    return mix(minStep, maxStep, t);
}


//runge kutta 4th order integration scheme
void hstepRK4(inout vec4 p, inout vec4 q)
{    
    float dt1 = dt*clamp(1.0 / length(p),inputData.stepSizeMin,inputData.stepSizeMax);
    mat2x4 state = mat2x4(p, q); 

    mat2x4 k1 = dhstep(state, dt1);
    mat2x4 k2 = dhstep(state + 0.5*k1, dt1);
    mat2x4 k3 = dhstep(state + 0.5*k2, dt1);
    mat2x4 k4 = dhstep(state + k3, dt1);
    
    state += (k1 + 2.0*k2 + 2.0*k3 + k4)/6.0;

    p = state[0];
    q = state[1];
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
float sampleDensity(vec3 p)
{
    float density = 0.0;
    // cylindrical coords
    const vec2 xz = vec2(p.x, p.z);
    const float r = length(xz);
    const float theta = atan(xz.y, xz.x);

    // ----- density calc -----
    const float innerRadius = inputData.disk_InnerRadius * inputData.mass;
    const float outerRadius = inputData.disk_OuterRadius * inputData.mass;

    float orbitalSpeed = inputData.disk_RotationSpeed / pow(r, 1.5) + diskBaseRotationSpeed;
    float angleOffset = theta + orbitalSpeed * inputData.time;

    vec2 polar = vec2(
        r / inputData.disk_InnerRadius,
        (p.y + inputData.disk_Height * 0.5) / inputData.disk_Height
    );
    
    float u = cos(angleOffset) * polar.x * 0.5 + 0.5;
    float w = sin(angleOffset) * polar.x * 0.5 + 0.5;
    vec3 diskUVW = vec3(u, polar.y, w);

    float noise = texture(noiseMap, diskUVW).r;

    float radiusNorm = r / inputData.disk_InnerRadius;
    float thicknessAtR = inputData.disk_Height * (1.0 + sqrt(radiusNorm) * 0.5);
    
    float yNorm = abs(p.y) / thicknessAtR;
    float verticalFade = 1.0 - smoothstep(inputData.disk_VerticalFadeStart, inputData.disk_VerticalFadeEnd, yNorm);

    float innerFade = smoothstep(inputData.disk_InnerFadeRadius, inputData.disk_InnerFadeRadius + inputData.disk_InnerFadeWidth, radiusNorm);

    float outerFade = 1.0 - smoothstep(inputData.disk_OuterFadeStart, inputData.disk_OuterFadeEnd, radiusNorm);
    float radialFade = innerFade * outerFade;

    density = noise * inputData.disk_Density * verticalFade * radialFade;

    return density;
}
// ==================== Transmittance ====================
float computeTransmittance(vec3 position, vec3 Ld) {
    float tau = 0.0;
    int steps = inputData.disk_MaxShadowIterations;
    
    for (int i = 0; i < steps; i++) {
        vec3 sp = position - Ld * (float(i) * inputData.disk_ShadowStepSize);
        float d = sampleDensity(sp);
        tau += pow(d, inputData.disk_ExpoFactor) * inputData.disk_MaxShadowIterations;
    }
    
    float T = exp(-tau);
    return T;
}

// Double-lobed Henyey-Greenstein phase function
float doubleLobedHG(float cosTheta, float gForward, float gBackward, float lobeWeight) {
    float g1 = gForward;
    float g2 = gBackward;
    
    float denom1 = 1.0 + g1 * g1 - 2.0 * g1 * cosTheta;
    float phase1 = (1.0 - g1 * g1) / (4.0 * PI * pow(denom1, 1.5));
    
    float denom2 = 1.0 + g2 * g2 - 2.0 * g2 * cosTheta;
    float phase2 = (1.0 - g2 * g2) / (4.0 * PI * pow(denom2, 1.5));
    
    return mix(phase2, phase1, lobeWeight);
}

void sampleDisk(vec3 p, vec3 rayDir, float stepSize, float time, inout vec3 emission, inout float T)
{
    
    float density = sampleDensity(p);
    if (density < 0.1) return;

    vec3 lightDir = normalize(p);
    float Tsh = computeTransmittance(p, lightDir);

    float phase = doubleLobedHG(dot(lightDir, -rayDir), inputData.disk_AnisotropyForward, inputData.disk_AnisotropyBackward, inputData.disk_LobeWeight);

    float softenedDensity = pow(density, inputData.disk_Brightness);
    float scat = softenedDensity * stepSize;

    float r = length(p);

    float dist2 = dot(-p, -p);
    float atten = inputData.disk_LightIntensity / (max(dist2, 0.001) * 0.06);

    float tRad = clamp((r - inputData.disk_InnerRadius) / (inputData.disk_OuterRadius - inputData.disk_InnerRadius), 0.0, 1.0);
    vec3 sampleColor = hotColor * (1.0 - tRad) + (vec3(3.0, 6.0, 12.0) * 2.0); // BLUE???

    float saturation = pow(1.0 - tRad, inputData.disk_LightFalloff);
    sampleColor *= saturation;

    vec3 toCam = normalize(inputData.camPos - p);
    vec3 tangentDir = normalize(vec3(-p.z, 0.0, p.x));

    float v = inputData.disk_RotationSpeed / pow(r, 1.5);
    vec3 vel = tangentDir * v;

    float relVel = -dot(normalize(vel), toCam);
    float dopplerBoost = 1.0 + inputData.disk_DopplerStrength * relVel;

    sampleColor *= dopplerBoost;

    emission += T * Tsh * atten * sampleColor * phase * scat * density;

    T *= exp(-pow(density, inputData.disk_ExpoFactor) * stepSize);
   
  /*  
    // Hot spots and flares
    float flareNoise = texture(noiseMap, p * 0.3 + time * 0.5).r;
    if (flareNoise > 0.85 && temperatureParam > 0.6) {
        density *= 1.0 + (flareNoise - 0.85) * 5.0;
    }
    */
}

bool segmentIntersectsSphere(vec3 segStart, vec3 segEnd, vec3 sphereCenter, float sphereRadius) {
    vec3 d = segEnd - segStart;
    vec3 f = segStart - sphereCenter;
    
    float a = dot(d, d);
    
    // Degenerate segment (start == end)
    if (a < 1e-8) {
        return dot(f, f) <= sphereRadius * sphereRadius;
    }
    
    float b = 2.0 * dot(f, d);
    float c = dot(f, f) - sphereRadius * sphereRadius;
    float discriminant = b * b - 4.0 * a * c;
    
    if (discriminant < 0.0) {
        return false; // No intersection with infinite ray
    }
    
    discriminant = sqrt(discriminant);
    float t1 = (-b - discriminant) / (2.0 * a);
    float t2 = (-b + discriminant) / (2.0 * a);
    
    // Check if ANY part of the intersection overlaps [0, 1]
    // This is equivalent to: NOT (both intersections are before 0 OR both are after 1)
    return !(t2 < 0.0 || t1 > 1.0);
}

bool interceptObject(vec3 P, vec3 dir) {
    for (int i = 0; i < objectData.objectCount; ++i) {
        vec3 center = objectData.objects[i].position;
        float radius = objectData.objects[i].radius;
        if (segmentIntersectsSphere(P, P+dir, center, radius)) {
            // Store color - you'll need to add color to Object struct!
            objectColor = objectData.objects[i].color;  // Temp orange color
            hitCenter = center;
            hitRadius = radius;
            return true;
        }
    }
    return false;
}

float raySphereIntersect(vec3 ro, vec3 rd, vec3 sphereCenter, float sphereRadius)
{
    vec3 oc = ro - sphereCenter;
    
    float b = dot(oc, rd);
    float c = dot(oc, oc) - sphereRadius * sphereRadius;
    float discriminant = b * b - c;
    
    if (discriminant < 0.0) return -1.0;
    
    float t = -b - sqrt(discriminant);
    if (t < 0.0) t = -b + sqrt(discriminant);
    
    return t < 0.0 ? -1.0 : t;
}

void computeVolumetric(vec4 Q, vec3 seg, vec3 segDir, float ds, inout vec3 L, inout float T)
{
        // Check if we're in the disk region
    const float innerR = inputData.disk_InnerRadius * 0.8;
    const float outerR = inputData.disk_OuterRadius * 1.2;
    const float maxH = inputData.disk_Height * 5.0;

    // March through the segment
    for (int s = 0; s < inputData.volSubsteps; ++s) {
        float tMid = (float(s) + stepNoise) / float(inputData.volSubsteps);
        vec3 pMid = Q.yzw + seg * tMid;
        
        vec2 xz = vec2(pMid.x, pMid.z);
        float rperp = length(xz);
        float h = abs(pMid.y);

        if (rperp >= innerR && rperp <= outerR && h <= maxH) {
            sampleDisk(pMid, segDir, ds, inputData.time, L, T);
        }
    }
}
void traceRay(vec3 cp, vec3 rd, out vec2 warp, out vec3 L, out float T)
{
    {
        float t0 = sphIntersect(cp, rd, vec4(0,0,0,cdist));
        if(t0 > 0.0 && t0 < 1e10) cp += rd*t0;
    }


    vec4 Q = vec4(0., cp);
    vec4 P = normalize(Vec2P(rd, Q.yzw));

    //float p0 = P.x;

    
    L = vec3(0.0);  // Accumulated radiance
    T = 1.0;       // Transmittance
    bool shouldTerminate = false;
    bool tempBlackHoleHit = false;
    vec4 prevQ;

    for (int i = 0; i < inputData.maxSteps; ++i) {

        prevQ = Q;
        hstepRK4(P, Q);

        vec3 seg = Q.yzw - prevQ.yzw;
        float segLen = length(seg);

        if (inputData.disk_Density > 0 && T > 0.01) {

            if (segLen > 0.0) {
                vec3 segDir = seg / segLen;
                int sub = max(1, inputData.volSubsteps);
                float ds = (segLen * inputData.dsVolScale) / float(sub);
                computeVolumetric(Q, seg, segDir, ds, L, T);
            }
        }

        if (interceptObject(Q.yzw, seg)) {
            T = 0;
            L += objectColor.rgb;
            break;
        }

        if (variDT(P,Q, tempBlackHoleHit)) 
        {   
            shouldTerminate = true;
            break;
        }

    }

    vec4 qt = P2Vec(P,Q);
    rd = normalize(qt.yzw);
    vec2 warpedUV = directionToEquirectangularUV(rd);
    vec2 finalWarpSend = shouldTerminate && tempBlackHoleHit ? vec2(-1.0, 0.0) : warpedUV;
    

    warp = finalWarpSend;
    
}

void main() {
    ivec2 pixelCoord = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(directionTexture);
    
    if (pixelCoord.x >= size.x || pixelCoord.y >= size.y) return;

    // Initialize global variables
    m = inputData.mass;
    a = inputData.rotationSpeed * m;
    horizonRadius = m + sqrt(m * m - a * a);
    
    // Check if this block needs high resolution (sample from block center)
    vec2 uv = (vec2(pixelCoord) + 0.5) / vec2(size);
    vec3 cp, rd;
    if (!getRay(uv, cp, rd)) return;
    
    {
        vec2 noiseUV = fract(pixelCoord.xy / 64.0 + inputData.time * 0.1);
        stepNoise = texture(blueNoiseTexture, noiseUV).r;
    }
    vec3 L;
    float T;
    vec2 warp;
        
    if (!getRay(uv, cp, rd)) return;
    traceRay(cp, rd, warp, L, T);
        
    imageStore(directionTexture, pixelCoord, vec4(warp, 0.0, 0.0));
    //if (needsHighRes) {imageStore(volumeTexture, pixelCoord, vec4(1.0, 0.0, 0.0, 1.0)); return;}

    imageStore(volumeTexture, pixelCoord, vec4(L.rgb, T));
}