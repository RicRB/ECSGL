#include "descriptor_set.h"
#include "platform/vulkan/vk_descriptor_set.h"
#include "core/engine.h"
std::shared_ptr<DescriptorSet> DescriptorSet::Create()
{
	switch (graphics::getGraphicsAPIType()) {
		case GRAPHICS_API_VULKAN: return std::make_shared<VulkanDescriptorSet>();
		case GRAPHICS_API_OPENGL: return nullptr;
	}

	return nullptr;
}

std::unique_ptr<DescriptorSet> DescriptorSet::CreateUnique()
{
	switch (graphics::getGraphicsAPIType()) {
	case GRAPHICS_API_VULKAN: return std::make_unique<VulkanDescriptorSet>();
	case GRAPHICS_API_OPENGL: return nullptr;
	}

	return nullptr;
}
