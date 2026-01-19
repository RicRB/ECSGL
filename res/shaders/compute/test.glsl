#version 450

layout(std140, binding = 0) readonly buffer InputBuffer {
    int numbers[];
} inputData;

layout(std140, binding = 1) writeonly buffer OutputBuffer {
    int numbers[];
} outputData;

layout (local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

void main() {
    uint index = gl_GlobalInvocationID.x;
    outputData.numbers[index] = inputData.numbers[index] + 1;
}