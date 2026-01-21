#pragma once
#include "renderer/image.h"
#include <vulkan/vulkan.h>

struct VulkanImage : public Image {
	VkImage image;
	VkDeviceMemory memory;
	VkImageView imageView;
	VkSampler sampler;
	VkImageLayout layout;
	VkFormat format;
};