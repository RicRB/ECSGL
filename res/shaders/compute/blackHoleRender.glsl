#version 450

layout (local_size_x = 8, local_size_y = 8) in;

//layout(std430, binding = 0) readonly buffer inputBuffer {
//    int numbers[];
//} inputData;


layout(set=0, binding = 0) uniform sampler2D directionTexture;
layout(set=0, binding = 1) uniform sampler2D volumeTexture;
layout(set = 0, binding = 2) uniform sampler2D sphereMapTexture;

layout(set=0, binding = 3, rgba8) uniform writeonly image2D outputImage;


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

// from http://www.java-gaming.org/index.php?topic=35123.0
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

void main() {

    ivec2 pixelCoord = ivec2(gl_GlobalInvocationID.xy);

    ivec2 size = imageSize(outputImage);

    if (pixelCoord.x >= size.x || pixelCoord.y >= size.y) return;

    // Calculate UV coordinates (0.0 to 1.0)
    vec2 uv = (vec2(pixelCoord) + 0.5) / vec2(size);

    vec4 volumeData = textureBicubic(volumeTexture, uv);
    vec3 accumulatedLight = volumeData.rgb;
    float T = volumeData.a;


    vec4 warpData = textureBicubic(directionTexture, uv);
    vec2 finalWarp = warpData.xy;
    vec3 background = vec3(0.0);
    if (finalWarp != vec2(-1.0, 0.0)) 
    {
        background = texture(sphereMapTexture, finalWarp).xyz;
    }
    

    // Composite: accumulated light over background
    vec3 fcol = accumulatedLight + T * background;
    imageStore(outputImage, pixelCoord, vec4(fcol, 1.0));
}