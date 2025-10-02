#pragma once
#include "renderer/image.h"
#include <vulkan/vulkan.h>

struct VkTextureImage : public TextureImage {
	VkImage image;
	VkDeviceMemory memory;
	VkImageView imageView;
	VkSampler sampler;
};