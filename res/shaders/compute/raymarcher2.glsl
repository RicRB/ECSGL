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
    float bhRotation;
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

float disk_num = 2;

layout(set=0, binding = 3) uniform sampler3D noiseMap;

layout(set=0, binding = 4, rgba8) uniform writeonly image2D directionTexture;
layout(set=0, binding = 5, rgba8) uniform writeonly image2D volumeTexture;

const float PI = 3.1415926538;
const float eps = 0.01;  // Hamiltonian gradient step
const vec2 dq = vec2(0, eps);
const float cdist = 120;
float camR;
float a;
float m;
float dt = 0.1;
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

	float det = n11 * t11 + n12 * t12 + n13 * t13 + n14 * t14;
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
    float u = uv.x * inputData.aspect * inputData.tanHalfFov;
    float v = uv.y * inputData.tanHalfFov;

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
void main() {

    ivec2 pixelCoord = ivec2(gl_GlobalInvocationID.xy);

    ivec2 size = imageSize(directionTexture);

    if (pixelCoord.x >= size.x || pixelCoord.y >= size.y) return;

    vec2 uv = (vec2(pixelCoord) + 0.5) / vec2(size);
    // intialize global variables
    m = inputData.mass;
    a = inputData.rotationSpeed * m;
    horizonRadius = m + sqrt(m * m - a * a);

    float actualAspect = float(size.x) / float(size.y);
    // Init Ray
    float u = (2.0 * (pixelCoord.x + 0.5) / float(size.x) - 1.0) * actualAspect * inputData.tanHalfFov;
    float v = (1.0 - 2.0 * (pixelCoord.y + 0.5) / float(size.y)) * inputData.tanHalfFov;

    vec3 cp, rd; // ray pos and ray dir
    if (!getRay(vec2(u,v), cp, rd)) return;

    vec3 fcol = vec3(0.0);
    float t0 = sphIntersect(cp, rd, vec4(0,0,0,cdist));
    if(t0 > 0.0 && t0 < 1e10) cp += rd*t0;

    vec4 Q = vec4(0., cp);
    vec4 P = normalize(Vec2P(rd, Q.yzw));

    float p0 = P.x;

    float totalDist = 0.0;

    
    vec3 L = vec3(0.0);  // Accumulated radiance
    float T = 1.0;       // Transmittance
    bool hitBlackHole = false;
    bool hitObject = false;
    bool shouldTerminate = false;
    bool tempBlackHoleHit = false;
    vec4 prevQ;

    for (int i = 0; i < inputData.maxSteps; ++i) {

        prevQ = Q;
        hstepRK4(P, Q);

        vec3 seg = Q.yzw - prevQ.yzw;
        float segLen = length(seg);

        if (inputData.diskEnable != 0 && T > 0.01) {

            if (segLen > 0.0) {
                vec3 segDir = seg / segLen;
                int sub = max(1, inputData.volSubsteps);
                float ds = (segLen * inputData.dsVolScale) / float(sub);

                // March through the segment
                for (int s = 0; s < sub; ++s) {
                    float tMid = (float(s) + 0.5) / float(sub);
                    vec3 pMid = Q.yzw + seg * tMid;
                    
                    vec2 xz = vec2(pMid.x, pMid.z);
                    float rperp = length(xz);
                    float h = abs(pMid.y);

                    // Check if we're in the disk region
                    float innerR = inputData.diskInnerRadius * 0.8;
                    float outerR = inputData.diskOuterRadius * 1.2;
                    float maxH = inputData.diskH * 5.0;

                    if (rperp >= innerR && rperp <= outerR && h <= maxH) {
                        // Sample the disk density and emission
                        sampleDisk(pMid, segDir, m, a, inputData.time);
                        
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
    vec2 warp = warpedUV - uv;
    imageStore(directionTexture, pixelCoord, vec4(warpedUV, 0.0, 1.0)); // a should be something important
    imageStore(volumeTexture, pixelCoord, vec4(L.rgb, T));
    /*
    float p1 = P.x;
    float redshift = p0/p1;

    if (shouldTerminate && tempBlackHoleHit && !hitObject)
    {
        hitBlackHole = true;
    }
    if (hitObject)
    {
            // Find actual intersection point along the ray
        vec3 toCenter = hitCenter - prevQ.yzw;
        vec3 rayDir = normalize(Q.yzw - prevQ.yzw);
        
        // Project center onto ray to find closest point
        float t = dot(toCenter, rayDir);
        vec3 closestPoint = prevQ.yzw + rayDir * t;
        
        // Calculate actual intersection distance
        float distToCenter = length(hitCenter - closestPoint);
        float offset = sqrt(max(0.0, hitRadius * hitRadius - distToCenter * distToCenter));
        
        // Actual hit position
        vec3 hitPos = closestPoint - rayDir * offset;
        
        // Now compute shading with correct position and normal
        vec3 N = normalize(hitPos - hitCenter);
        vec3 V = normalize(inputData.camPos - hitPos);
        
        float ambient = 0.1;
        float diff = max(dot(N, V), 0.0);
        float intensity = ambient + (1.0 - ambient) * diff;
        vec3 shaded = objectColor.rgb * intensity;
        
        fcol = L + T * shaded;
    } else if (hitBlackHole) {
        if (inputData.diskEnable == 0)
        {

        }else{
            fcol = L;
        }

    } else {
        fcol = L + T * sampleBackground(rd).xyz;
    }

    fcol = fcol * pow(colorTemperatureToRGB(6700.0 * redshift), vec3(10.0)) * redshift;
    imageStore(outputImage, pixelCoord, vec4(fcol, 1.0));
    */
}