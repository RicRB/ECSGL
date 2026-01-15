#include "descriptor_set.h"
#include "platform/vulkan/vk_descriptor_set.h"
#include "core/engine.h"
std::shared_ptr<DescriptorSet> DescriptorSet::Create()
{
	switch (engine::currentEngine->renderer.rendererType) {
		case GRAPHICS_API_VULKAN: return std::make_shared<VkDescriptorSetWrapper>();
		case GRAPHICS_API_OPENGL: return nullptr;
	}

	return nullptr;
}
