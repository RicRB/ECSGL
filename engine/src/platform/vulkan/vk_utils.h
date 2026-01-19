#pragma once
#include <vulkan/vulkan.h>
#include <optional>
struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;
	std::optional<uint32_t> computeFamily;

	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value() && computeFamily.has_value();
	}
};
class VkUtils {
public:
	static uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
	static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
	static uint32_t findComputeQueueFamily(VkPhysicalDevice physicalDevice);
	static VkCommandBuffer beginSingleTimeCommands(VkDevice device, VkCommandPool commandPool);
	static void endSingleTimeCommands(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkCommandBuffer commandBuffer);
};