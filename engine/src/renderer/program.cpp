#include "program.h"
#include "core/engine.h"
#include "platform/vulkan/vk_program.h"
#include "platform/opengl/gl_program.h"
std::shared_ptr<Program> Program::Create()
{
	switch (engine::currentEngine->renderer.rendererType) {
	case GRAPHICS_API_VULKAN: return std::make_shared<VkProgram>();
	case GRAPHICS_API_OPENGL: return std::make_shared<GlProgram>();
	}

	return nullptr;
}
