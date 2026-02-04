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

layout (local_size_x = 16, local_size_y = 16) in;

void main() {
    ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
    ivec2 outputSize = imageSize(outputTexture);
    
    // Bounds check
    if (pixelCoords.x >= outputSize.x || pixelCoords.y >= outputSize.y) {
        return;
    }
    
    // Calculate UV coordinates (normalized 0-1)
    vec2 uv = (vec2(pixelCoords) + 0.5) / vec2(outputSize);
    
    vec4 result;
    if (blurData.mode == 0) {
        // Downsample
        result = downsample(sourceTexture, uv, blurData.texelSize);
    } else {
        // Upsample
        result = upsample(sourceTexture, uv, blurData.texelSize);
    }
    
    imageStore(outputTexture, pixelCoords, result);
}