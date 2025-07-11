#pragma once
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include "uniform_objects.h"

struct VkUniformBuffer {
	VkBuffer buffer = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
	void* mapped = nullptr;
	VkDeviceSize bufferSize;
};