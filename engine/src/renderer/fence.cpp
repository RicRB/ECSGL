#include "fence.h"
#include "core/engine.h"
#include "platform/vulkan/vk_fence.h"

std::unique_ptr<Fence> Fence::createUnique()
{
	switch (graphics::getGraphicsAPIType()) {
	case GRAPHICS_API_VULKAN: return std::make_unique<VulkanFence>();
	case GRAPHICS_API_OPENGL: return nullptr;
	}
}
