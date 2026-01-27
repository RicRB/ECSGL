#pragma once
#include <vulkan/vulkan.h>
#include <optional>
#include "renderer/descriptor_set.h"
#include "renderer/texture.h"
struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;
	std::optional<uint32_t> computeFamily;

	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value() && computeFamily.has_value();
	}
};

struct PendingCommand {
	VkCommandBuffer buffer;
	VkCommandPool pool;
	VkFence fence;

	PendingCommand(VkCommandBuffer buffer, VkCommandPool pool, VkFence fence) : buffer(buffer), pool(pool), fence(fence) {};
};


class VkUtils {
public:
	static std::vector<PendingCommand> pendingCommands;
	static uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
	static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
	static uint32_t findComputeQueueFamily(VkPhysicalDevice physicalDevice);
	static VkCommandBuffer beginSingleTimeCommands(VkDevice device, VkCommandPool commandPool);
	static void endSingleTimeCommands(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkCommandBuffer commandBuffer, VkFence fence = VK_NULL_HANDLE);
	static void endSingleTimeCommandsAsync(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkCommandBuffer commandBuffer, VkFence fence);
	static VkDescriptorType getVkDescriptorType(DescriptorType type);
	static VkImageLayout getVkImageLayout(TextureUsage usage);
	static void cleanupPendingCommands(VkDevice device);
};