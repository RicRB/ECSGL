#pragma once
#include "renderer/descriptor_set.h"
#include <vulkan/vulkan.h>
struct VkDescriptorSetWrapper : public DescriptorSet {
	VkDescriptorSet set;
};