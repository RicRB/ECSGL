#include "image.h"
#include "core/engine.h"
#include "platform/vulkan/vk_image.h"
#include "platform/opengl/gl_image.h"
std::shared_ptr<Image> Image::Create()
{
	switch (graphics::getGraphicsAPIType()) {
	case GRAPHICS_API_VULKAN: return std::make_shared<VulkanImage>();
	case GRAPHICS_API_OPENGL: return std::make_shared<GlImage>();
	}

	return nullptr;
}