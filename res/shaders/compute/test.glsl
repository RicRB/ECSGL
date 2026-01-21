#version 450

layout (local_size_x = 16, local_size_y = 16) in;

//layout(std430, binding = 0) readonly buffer inputBuffer {
//    int numbers[];
//} inputData;

layout(set = 0, binding = 0) uniform sampler2D inputImage;

layout(set = 0, binding = 1, rgba8) uniform writeonly image2D outputImage;



void main() {
    ivec2 pixelCoord = ivec2(gl_GlobalInvocationID.xy);
    
    ivec2 size = textureSize(inputImage, 0);
    
    if (pixelCoord.x >= size.x || pixelCoord.y >= size.y) {
        return;
    }
    
    // Read from sampler with texelFetch
    vec4 color = texelFetch(inputImage, pixelCoord, 0);
    
    color = vec4(1.0, 0.0, 0.0, 1.0);
    color = clamp(color, 0.0, 1.0);
    
    // Write to storage image
    imageStore(outputImage, pixelCoord, color);
}