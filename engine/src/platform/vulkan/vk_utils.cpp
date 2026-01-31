#include "vk_utils.h"
#include <stdexcept>
#include <vector>

std::vector<PendingCommand> VkUtils::pendingCommands;
uint32_t VkUtils::findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}
	throw std::runtime_error("failed to find suitable memory type!");
}

QueueFamilyIndices VkUtils::findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
		if (presentSupport) {
			indices.presentFamily = i;
		}

		// Find compute queue - prefer dedicated compute (no graphics)
		if ((queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) &&
			!(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
			indices.computeFamily = i;
		}

		if (indices.isComplete()) {
			break;
		}
		i++;
	}

	// Fallback: if no dedicated compute queue, use graphics queue (it supports compute)
	if (!indices.computeFamily.has_value() && indices.graphicsFamily.has_value()) {
		indices.computeFamily = indices.graphicsFamily;
	}

	return indices;
}
uint32_t VkUtils::findComputeQueueFamily(VkPhysicalDevice physicalDevice) {
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

	// First, try to find a dedicated compute queue (compute but not graphics)
	for (uint32_t i = 0; i < queueFamilyCount; i++) {
		if ((queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) &&
			!(queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
			return i;
		}
	}

	// Otherwise, use any queue that supports compute (graphics queues support compute)
	for (uint32_t i = 0; i < queueFamilyCount; i++) {
		if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
			return i;
		}
	}

	throw std::runtime_error("Failed to find compute queue family");
}

VkCommandBuffer VkUtils::beginSingleTimeCommands(VkDevice device, VkCommandPool commandPool)
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void VkUtils::endSingleTimeCommands(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkCommandBuffer commandBuffer, VkFence fence)
{
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(queue, 1, &submitInfo, fence);
	vkQueueWaitIdle(queue);

	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}
void VkUtils::endSingleTimeCommandsAsync(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkCommandBuffer commandBuffer, VkFence fence)
{
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(queue, 1, &submitInfo, fence);

	pendingCommands.emplace_back(commandBuffer, commandPool, fence);
	cleanupPendingCommands(device);
}

VkDescriptorType VkUtils::getVkDescriptorType(DescriptorType type)
{
	VkDescriptorType descriptorType;
	switch (type)
	{
	case DescriptorType::UniformBuffer:
		descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		break;
	case DescriptorType::StorageBuffer:
		descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		break;
	case DescriptorType::CombinedImageSampler:
		descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		break;
	case DescriptorType::StorageImage:
		descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		break;
	default:
		throw std::runtime_error("Unsupported descriptor type!");
	}
	return descriptorType;
}



VkImageLayout VkUtils::getVkImageLayout(TextureUsage usage)
{
	switch (usage)
	{
	case TextureUsage::Read:
		return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	case TextureUsage::ReadWrite:
		return VK_IMAGE_LAYOUT_GENERAL;
	default:
		return VK_IMAGE_LAYOUT_GENERAL;
	}
}

VkFormat VkUtils::getVkFormat(TextureFormat format)
{
	switch (format)
	{
	case TextureFormat::R16G16_SFLOAT: return VK_FORMAT_R16G16_SFLOAT;
	case TextureFormat::R16G16_SNORM: return VK_FORMAT_R16G16_SNORM;
	case TextureFormat::R16G16_UNORM: return VK_FORMAT_R16G16_UNORM;
	case TextureFormat::R8G8B8A8_UNORM: return VK_FORMAT_R8G8B8A8_UNORM;
	case TextureFormat::R16G16B16A16_UNORM: return VK_FORMAT_R16G16B16A16_UNORM;
	case TextureFormat::R32G32B32A32_SFLOAT: return VK_FORMAT_R32G32B32A32_SFLOAT;;
	}
}

void VkUtils::cleanupPendingCommands(VkDevice device)
{
	for (auto it = pendingCommands.begin(); it != pendingCommands.end();)
	{
		if (vkGetFenceStatus(device, it->fence) == VK_SUCCESS)
		{
			vkFreeCommandBuffers(device, it->pool, 1, &it->buffer);
			it = pendingCommands.erase(it);
		}
		else
		{
			++it;
		}
	}
}