#include "shader.h"
#include "core/engine.h"
#include "platform/opengl/gl_def.h"
#include "platform/opengl/gl_program.h"
#include "platform/vulkan/vk_program.h"

ShaderData::ShaderData()
{
	program = Program::Create();
}
