#pragma once
#include <vulkan/vulkan.h>

class VkUtils {
public:
	static uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

};