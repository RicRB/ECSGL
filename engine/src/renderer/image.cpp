#include "image.h"
#include "core/engine.h"
#include "platform/vulkan/vk_image.h"
#include "platform/opengl/gl_image.h"
std::shared_ptr<TextureImage> TextureImage::Create()
{
	switch (engine::currentEngine->renderer.rendererType) {
	case GRAPHICS_API_VULKAN: return std::make_shared<VkTextureImage>();
	case GRAPHICS_API_OPENGL: return std::make_shared<GlTextureImage>();
	}

	return nullptr;
}