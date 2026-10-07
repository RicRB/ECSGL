#version 450

layout (local_size_x = 16, local_size_y = 16) in;

//layout(std430, binding = 0) readonly buffer inputBuffer {
//    int numbers[];
//} inputData;


layout(set=0, binding = 0) uniform sampler2D directionTexture;
layout(set=0, binding = 1) uniform sampler2D volumeTexture;
layout(set = 0, binding = 2) uniform sampler2D sphereMapTexture;

layout(set=0, binding = 3, rgba8) uniform writeonly image2D outputImage;

layout(std430, binding = 4) readonly buffer inputDataBlock {
    vec3 camPos;     float _pad0;
    vec3 camRight;   float _pad1;
    vec3 camUp;      float _pad2;
    vec3 camForward; float _pad3;
    vec4 color;
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
    float offsetDensity;
    float noiseScale;
    float noiseOffset;
    float intersectLow;
    float intersectHigh;
} inputData;

struct Object {
    vec3 position;
    float radius;
    vec4 color;
};

layout(std430, binding = 5) readonly buffer objectDataBlock {
    int objectCount;
    Object objects[];
} objectData;

layout(set=0, binding = 6) uniform sampler3D noiseMap;
layout(set=0, binding = 7) uniform sampler2D blueNoiseTexture;

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








struct KerrSchildData
{
    vec4 kCov;
    vec4 kCon;
    float f;
    float r;
};

vec4 minkowskiMul(vec4 v)
{
    return vec4(-v.x, v.y, v.z, v.w);
}

KerrSchildData getKerrSchildData(vec4 q)
{
    KerrSchildData data;

    vec3 p = q.yzw;

    float a2 = a * a;
    float rho = dot(p, p) - a2;

    float r2 = 0.5 * (
        rho +
        sqrt(max(
            rho * rho +
            4.0 * a2 * p.y * p.y,
            0.0
        ))
    );

    r2 = max(r2, 1e-10);

    float r = sqrt(r2);
    float denom = r2 + a2;

    vec4 kCov = vec4(
        1.0,
        (r * p.x + a * p.z) / denom,
        p.y / r,
        (r * p.z - a * p.x) / denom
    );

    vec4 kCon = vec4(
        -kCov.x,
         kCov.y,
         kCov.z,
         kCov.w
    );

    // Keep the Kerr-Newman charge term.
    // Replace this with inputData.charge when charge is exposed in the SSBO.
    float charge = 0.0;
    float charge2 = charge * charge;

    float sigma =
        r2 * r2 +
        a2 * p.y * p.y;

    float metricWindow =
        1.0 - smoothstep(0.0, cdist * 0.5, r);

    float f =
        metricWindow *
        r2 *
        (2.0 * m * r - charge2) /
        sigma;

    data.kCov = kCov;
    data.kCon = kCon;
    data.f = f;
    data.r = r;

    return data;
}

vec4 metricMul(KerrSchildData data, vec4 v)
{
    vec4 etaV = minkowskiMul(v);
    float kv = dot(data.kCov, v);

    return etaV +
           data.f * data.kCov * kv;
}

vec4 inverseMetricMul(KerrSchildData data, vec4 p)
{
    vec4 etaP = minkowskiMul(p);
    float kp = dot(data.kCon, p);

    return etaP -
           data.f * data.kCon * kp;
}

float metricQuadratic(KerrSchildData data, vec4 v)
{
    float minkowski =
        -v.x * v.x +
         v.y * v.y +
         v.z * v.z +
         v.w * v.w;

    float kv = dot(data.kCov, v);

    return minkowski +
           data.f * kv * kv;
}

vec4 Vec2P(vec3 v, vec3 x)
{
    vec4 q = vec4(0.0, x);

    KerrSchildData data =
        getKerrSchildData(q);

    return 2.0 *
        metricMul(
            data,
            vec4(1.0, v)
        );
}

vec4 P2Vec(vec4 p, vec4 q)
{
    KerrSchildData data = getKerrSchildData(q);
    return inverseMetricMul(data, p);
}

bool variDT(vec4 P, vec4 q, out bool blackholeHit)
{
    KerrSchildData data = getKerrSchildData(q);
    float r = data.r;

    float t =
        max(r - horizonRadius, 0.0) /
        cdist;

    dt = mix(
        inputData.stepSizeMin,
        inputData.stepSizeMax,
        t
    );

    if (r < horizonRadius * 0.8)
    {
        blackholeHit = true;
        return true;
    }

    const float activeRadius = cdist * 0.5;

    if (
        length(q.yzw) > activeRadius ||
        length(P) > 45.0
    )
    {
        blackholeHit = false;
        return true;
    }

    return false;
}

mat2x4 dhstep(in mat2x4 s, float dt)
{
    vec4 p = s[0];
    vec4 q = s[1];

    KerrSchildData base = getKerrSchildData(q);

    vec4 qt = inverseMetricMul(base, p);

    float L0 = metricQuadratic(base, qt);

    // Kerr/Kerr-Newman is stationary, so dH/dt = 0.
    // Only spatial finite differences are needed.
    KerrSchildData metricX =
        getKerrSchildData(
            q + vec4(0.0, eps, 0.0, 0.0)
        );

    KerrSchildData metricY =
        getKerrSchildData(
            q + vec4(0.0, 0.0, eps, 0.0)
        );

    KerrSchildData metricZ =
        getKerrSchildData(
            q + vec4(0.0, 0.0, 0.0, eps)
        );

    float Lx = metricQuadratic(metricX, qt);
    float Ly = metricQuadratic(metricY, qt);
    float Lz = metricQuadratic(metricZ, qt);

    vec4 dHdq = vec4(
        0.0,
        -(Lx - L0) / eps,
        -(Ly - L0) / eps,
        -(Lz - L0) / eps
    );

    mat2x4 dqp;

    dqp[0] = -dHdq * dt;
    dqp[1] = 2.0 * qt * dt;

    return dqp;
}


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
        r / innerRadius,
        (p.y + inputData.disk_Height * 0.25) / inputData.disk_Height
    );
    polar.x *= orbitalSpeed;
    float uvScale = inputData.noiseScale;
    float u = cos(angleOffset) * polar.x * uvScale;
    float w = sin(angleOffset) * polar.x * uvScale;
    vec3 diskUVW = vec3(u, polar.y, w);
    vec3 centeredUVW = (diskUVW + vec3(inputData.noiseOffset, 0.0, inputData.noiseOffset));
    float noise = texture(noiseMap, centeredUVW).r + inputData.offsetDensity;

    float radiusNorm = r / innerRadius;
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
        tau += pow(d, inputData.disk_ExpoFactor) * inputData.disk_ShadowStepSize;
    }
    
    float T = exp(-tau);
    return T;
}

float doubleLobedHG(float cosTheta, float gForward, float gBackward, float lobeWeight) {
    float g1 = gForward;
    float g2 = gBackward;
    
    float denom1 = 1.0 + g1 * g1 - 2.0 * g1 * cosTheta;
    float phase1 = (1.0 - g1 * g1) / (4.0 * PI * pow(denom1, 1.5));
    
    float denom2 = 1.0 + g2 * g2 - 2.0 * g2 * cosTheta;
    float phase2 = (1.0 - g2 * g2) / (4.0 * PI * pow(denom2, 1.5));
    
    return mix(phase2, phase1, lobeWeight);
}


void sampleDisk(
    vec3 p,
    vec3 rayDir,
    float stepSize,
    float time,
    float density,
    float Tsh,
    inout vec3 emission,
    inout float T
)
{
    vec3 lightDir = normalize(p);

    float phase = doubleLobedHG(
        dot(lightDir, -rayDir),
        inputData.disk_AnisotropyForward,
        inputData.disk_AnisotropyBackward,
        inputData.disk_LobeWeight
    );

    float softenedDensity = pow(
        density,
        inputData.disk_Brightness
    );

    float scat = softenedDensity * stepSize;

    float r = length(p);

    float dist2 = dot(-p, -p);

    float atten =
        inputData.disk_LightIntensity /
        (max(dist2, 0.001) * 0.06);

    float tRad = clamp(
        (r - inputData.disk_InnerRadius) /
        (inputData.disk_OuterRadius - inputData.disk_InnerRadius),
        0.0,
        1.0
    );

    vec3 sampleColor =
        inputData.color.rgb *
        (1.0 - tRad);

    float saturation = pow(
        1.0 - tRad,
        inputData.disk_LightFalloff
    );

    sampleColor *= saturation;

    vec3 toCam =
        normalize(inputData.camPos - p);

    vec3 tangentDir =
        normalize(vec3(-p.z, 0.0, p.x));

    float v =
        inputData.disk_RotationSpeed /
        pow(r, 1.5);

    vec3 vel =
        tangentDir * v;

    float relVel =
        -dot(normalize(vel), toCam);

    float dopplerBoost =
        1.0 +
        inputData.disk_DopplerStrength *
        relVel;

    sampleColor *= dopplerBoost;

    emission +=
        T *
        Tsh *
        atten *
        sampleColor *
        phase *
        scat;

    T *= exp(
        -pow(
            density,
            inputData.disk_ExpoFactor
        ) *
        stepSize
    );
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


void computeVolumetric(
    vec3 start,
    vec3 seg,
    vec3 segDir,
    float ds,
    inout vec3 L,
    inout float T
)
{
    const float innerR =
        inputData.disk_InnerRadius * 0.8;

    const float outerR =
        inputData.disk_OuterRadius * 1.2;

    const float maxH =
        inputData.disk_Height * 5.0;

    // Compute the expensive shadow march at most once
    // for all useful volume samples in this geodesic segment.
    float cachedShadow = -1.0;

    for (int s = 0; s < inputData.volSubsteps; ++s)
    {
        float tMid =
            (float(s) + stepNoise) /
            float(inputData.volSubsteps);

        vec3 pMid =
            start + seg * tMid;

        vec2 xz =
            vec2(pMid.x, pMid.z);

        float rperp =
            length(xz);

        float h =
            abs(pMid.y);

        if (
            rperp >= innerR &&
            rperp <= outerR &&
            h <= maxH
        )
        {
            float density =
                sampleDensity(pMid);

            if (density < 0.1)
            {
                continue;
            }

            if (cachedShadow < 0.0)
            {
                vec3 lightDir =
                    normalize(pMid);

                cachedShadow =
                    computeTransmittance(
                        pMid,
                        lightDir
                    );
            }

            sampleDisk(
                pMid,
                segDir,
                ds,
                inputData.time,
                density,
                cachedShadow,
                L,
                T
            );
        }
    }
}



void traceRay(vec3 cp, vec3 rd, out vec2 warp)
{
    const float activeRadius = cdist * 0.5;

    float t0 =
        sphIntersect(
            cp,
            rd,
            vec4(0.0, 0.0, 0.0, activeRadius)
        );

    if (t0 > 0.0 && t0 < 1e10)
    {
        cp += rd * t0;
    }

    vec4 Q = vec4(0.0, cp);
    vec4 P = normalize(Vec2P(rd, Q.yzw));

    bool shouldTerminate = false;
    bool tempBlackHoleHit = false;

    for (int i = 0; i < inputData.maxSteps; ++i)
    {
        hstepRK4(P, Q);

        if (variDT(P, Q, tempBlackHoleHit))
        {
            shouldTerminate = true;
            break;
        }
    }

    vec4 qt = P2Vec(P, Q);
    rd = normalize(qt.yzw);

    vec2 warpedUV =
        directionToEquirectangularUV(rd);

    vec2 finalWarpSend =
        shouldTerminate && tempBlackHoleHit
            ? vec2(-1.0, 0.0)
            : warpedUV;

    warp = finalWarpSend;
}


vec4 sampleBackground(vec3 direction)
{
    vec4 color = vec4(0.0);
    vec2 uv = directionToEquirectangularUV(direction);
    color = texture(sphereMapTexture, uv);
    return color;
}

vec4 cubic(float v){
    vec4 n = vec4(1.0, 2.0, 3.0, 4.0) - v;
    vec4 s = n * n * n;
    float x = s.x;
    float y = s.y - 4.0 * s.x;
    float z = s.z - 4.0 * s.y + 6.0 * s.x;
    float w = 6.0 - x - y - z;
    return vec4(x, y, z, w) * (1.0/6.0);
}

vec4 textureBicubic(sampler2D tex, vec2 texCoords){
    vec2 texSize = vec2(textureSize(tex, 0));
    vec2 invTexSize = 1.0 / texSize;
    texCoords = texCoords * texSize - 0.5;
    vec2 fxy = fract(texCoords);
    texCoords -= fxy;
    vec4 xcubic = cubic(fxy.x);
    vec4 ycubic = cubic(fxy.y);
    vec4 c = texCoords.xxyy + vec2 (-0.5, +1.5).xyxy;
    vec4 s = vec4(xcubic.xz + xcubic.yw, ycubic.xz + ycubic.yw);
    vec4 offset = c + vec4 (xcubic.yw, ycubic.yw) / s;
    offset *= invTexSize.xxyy;

    // Clamp to valid texture coordinates
    offset = clamp(offset, vec4(0.0), vec4(1.0).xxyy);

    vec4 sample0 = texture(tex, offset.xz);
    vec4 sample1 = texture(tex, offset.yz);
    vec4 sample2 = texture(tex, offset.xw);
    vec4 sample3 = texture(tex, offset.yw);

    float sx = s.x / (s.x + s.y);

    float sy = s.z / (s.z + s.w);
    return mix(

       mix(sample3, sample2, sx), mix(sample1, sample0, sx)

    , sy);

}

vec4 sampleDirectionBilinear(sampler2D tex, vec2 uv) {
    // Standard bilinear is fine - just sample the texture
    return texture(tex, uv);

}

bool isAtBlackHoleEdge(vec4 q, out float r) {
    vec3 p = q.yzw;
    float rho = dot(p, p) - a * a;
    float r2 = 0.5 * (rho + sqrt(rho * rho + 4.0 * a * a * p.y * p.y));
    r = sqrt(r2);

    // Define "edge" as within 5-10% of the event horizon
    float edgeThickness = horizonRadius * 0.1; // 10% tolerance
    float edgeMin = horizonRadius - edgeThickness * 0.5;
    float edgeMax = horizonRadius + edgeThickness * 0.5;

    return r >= edgeMin && r <= edgeMax;
}

vec3 srgbToLinear(vec3 c)
{
    return pow(c, vec3(2.2));
}

void main() {

    ivec2 pixelCoord = ivec2(gl_GlobalInvocationID.xy);

    ivec2 size = imageSize(outputImage);

    if (pixelCoord.x >= size.x || pixelCoord.y >= size.y) return;

    vec3 cp, rd;

    // Calculate UV coordinates (0.0 to 1.0)
    vec2 uv = (vec2(pixelCoord) + 0.5) / vec2(size);


    if (!getRay(uv, cp, rd)) return;

    m = inputData.mass;
    a = inputData.rotationSpeed * m;
    horizonRadius = m + sqrt(m * m - a * a);

    float intersect = raySphereIntersect(cp, rd, vec3(0.0), horizonRadius * 2.8 * inputData.intersectHigh);
    float lowIntersect = raySphereIntersect(cp, rd, vec3(0.0), horizonRadius * 2.5* inputData.intersectLow);
    bool needsHighRes = (intersect > 0.0 && !(lowIntersect > 0.0));

    vec2 warp;

    if (needsHighRes)
    {
        {
            vec2 noiseUV = fract(pixelCoord.xy / 64.0 + inputData.time * 0.1);
            stepNoise = texture(blueNoiseTexture, noiseUV).r;
        }
        traceRay(cp, rd, warp);
    }else
    {
        vec4 warpData = textureBicubic(directionTexture, uv);
        warp = warpData.xy;
    }

    vec3 background = vec3(0.0);

    if (warp != vec2(-1.0, 0.0)) 
    {
        background = texture(sphereMapTexture, warp).xyz;
        background = background - vec3(0.05);
    }



    vec4 volumeData = textureBicubic(volumeTexture, uv);
    vec3 L = volumeData.rgb;
    float T = volumeData.a;

    // Composite: accumulated light over background

    vec3 fcol = L + T * background;

    imageStore(outputImage, pixelCoord, vec4(fcol, 1.0));

}