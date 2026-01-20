#version 450
#extension GL_EXT_debug_printf : enable

layout(std430, binding = 0) readonly buffer inputBuffer {
    int numbers[];
} inputData;

layout(std430, binding = 1) writeonly buffer outputBuffer {
    int numbers[];
} outputData;

layout (local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

void main() {
    uint index = gl_GlobalInvocationID.x;
    debugPrintfEXT("Thread %d: input = %d", index, inputData.numbers[index]);

    outputData.numbers[index] = inputData.numbers[index] + 1;
}