#include "compute_shader.h"
#include "platform/opengl/gl_def.h"
#include "platform/opengl/gl_program.h"
#include "platform/vulkan/vk_program.h"
ComputeShaderData::ComputeShaderData()
{
	program = Program::Create();
}
