#version 450

layout(std430, binding = 0) readonly buffer blurDataBlock {
    int mode;
    float pad_;
    vec2 texelSize;
} blurData;

layout(set = 0, binding = 1) uniform sampler2D sourceTexture;
layout(set = 0, binding = 2, rgba16) uniform writeonly image2D outputTexture;
// Downsample pass
vec4 downsample(sampler2D tex, vec2 uv, vec2 texelSize) {
    vec4 sum = texture(tex, uv) * 4.0;
    sum += texture(tex, uv + vec2(-1, -1) * texelSize);
    sum += texture(tex, uv + vec2( 1, -1) * texelSize);
    sum += texture(tex, uv + vec2(-1,  1) * texelSize);
    sum += texture(tex, uv + vec2( 1,  1) * texelSize);
    return sum / 8.0;
}

// Upsample pass
vec4 upsample(sampler2D tex, vec2 uv, vec2 halfpixel) {
    vec4 sum = vec4(0.0);
    
    // Inner box (weighted 2x)
    sum += texture(tex, uv + vec2(-1.0,  1.0) * halfpixel);
    sum += texture(tex, uv + vec2( 1.0,  1.0) * halfpixel) * 2.0;
    sum += texture(tex, uv + vec2( 1.0, -1.0) * halfpixel) * 2.0;
    sum += texture(tex, uv + vec2(-1.0, -1.0) * halfpixel);
    
    // Outer cross
    sum += texture(tex, uv + vec2( 0.0,  2.0) * halfpixel);
    sum += texture(tex, uv + vec2( 2.0,  0.0) * halfpixel);
    sum += texture(tex, uv + vec2( 0.0, -2.0) * halfpixel);
    sum += texture(tex, uv + vec2(-2.0,  0.0) * halfpixel);
    
    return sum / 12.0;
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

layout (local_size_x = 16, local_size_y = 16) in;

void main() {
    ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
    ivec2 outputSize = imageSize(outputTexture);
    
    // Bounds check
    if (pixelCoords.x >= outputSize.x || pixelCoords.y >= outputSize.y) {
        return;
    }
    
    vec2 uv = (vec2(pixelCoords) + 0.5) / vec2(outputSize);

    vec4 result;

    if (blurData.mode == 0) {
        result = downsample(sourceTexture, uv, blurData.texelSize);
    }
    else {
        result = textureBicubic(sourceTexture, uv);
    }

    imageStore(outputTexture, pixelCoords, result);
}