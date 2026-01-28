#version 450

layout (local_size_x = 8, local_size_y = 8) in;

//layout(std430, binding = 0) readonly buffer inputBuffer {
//    int numbers[];
//} inputData;
layout(std430, binding = 0) readonly buffer inputDataBlock {
    int size;
    float scale;
    float boost;
    int octaves;
    int tileSize;
    float lacunarity;
    float gain;
    float time;

} inputData;


layout(set = 0, binding = 1, rgba8) uniform writeonly image3D outputImage;

uint hash3D(ivec3 p)
{
    uint h = uint(p.x * 374761393u + p.y * 668265263u + p.z * 3624371u);
    h = (h ^ (h >> 13u)) * 1274126177u;
    return h ^ (h >> 16u);
}

vec3 perlinGradient(ivec3 p, int period)
{
    ivec3 ip = (p % period + period) % period;
    float rand = fract(float(hash3D(ip)) * 0.00000011920928955078125); // 1/2^23
    float theta = rand * 6.2831853;
    float phi = acos(2.0 * fract(rand * 1.37) - 1.0);
    return vec3(sin(phi) * cos(theta), sin(phi) * sin(theta), cos(phi));
}

vec3 fade(vec3 t)
{
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

float perlinNoise3D(vec3 p, int period)
{
    vec3 pi = floor(p);
    vec3 pf = fract(p);
    
    float dots[8];
    for (int i = 0; i < 8; i++)
    {
        ivec3 corner = ivec3(i & 1, (i >> 1) & 1, (i >> 2) & 1);
        vec3 grad = perlinGradient(ivec3(pi) + corner, period);
        vec3 dist = pf - vec3(corner);
        dots[i] = dot(grad, dist);
    }
    
    vec3 f = fade(pf);
    
    // trilinear interpolation
    float x1 = mix(dots[0], dots[1], f.x);
    float x2 = mix(dots[2], dots[3], f.x);
    float x3 = mix(dots[4], dots[5], f.x);
    float x4 = mix(dots[6], dots[7], f.x);
    
    float y1 = mix(x1, x2, f.y);
    float y2 = mix(x3, x4, f.y);
    
    return mix(y1, y2, f.z);
}

float fbm3(vec3 p, int octaves, float lacunarity, float gain, int basePeriod)
{
    float value = 0.0;
    float amp = 1.0;
    float freq = 1.0;
    float totalAmp = 0.0;
    
    for (int i = 0; i < octaves; i++)
    {
        int period = int(float(basePeriod) * freq);
        value += perlinNoise3D(p * freq, period) * amp;
        totalAmp += amp;
        freq *= lacunarity;
        amp *= gain;
    }
    
    return value / totalAmp;  // normalize
}

void main()
{
    uvec3 dispatchThreadID = gl_GlobalInvocationID;
    int size = inputData.size;
    
    if (dispatchThreadID.x >= size || dispatchThreadID.y >= size || dispatchThreadID.z >= size) 
        return;
    
    vec3 uv = (vec3(dispatchThreadID) + 0.5) / float(size);
    uv *= inputData.scale;
    
    float value = fbm3(uv, inputData.octaves, inputData.lacunarity, inputData.gain, inputData.tileSize);
    value = clamp(value + 0.5, 0.0, 1.0);
    value *= inputData.boost;
    value = max(value, 0.0);
    
    imageStore(outputImage, ivec3(dispatchThreadID), vec4(value, value, value, 1.0));
}