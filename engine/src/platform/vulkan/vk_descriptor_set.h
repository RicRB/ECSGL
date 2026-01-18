#pragma once
#include "renderer/descriptor_set.h"
#include <vulkan/vulkan.h>
struct VulkanDescriptorSet : public DescriptorSet {
	VkDescriptorSet set;
};