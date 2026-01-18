#include "vk_buffer_manager.h"
#include "vk_utils.h"

void VkBufferManager::createBuffer(Buffer* buffer, const BufferDesc& desc)
{
	VulkanBuffer* vkBuffer = (VulkanBuffer*)buffer;
	VkDeviceSize size = desc.size;
	vkBuffer->bufferSize = desc.size;

	VkBufferUsageFlags usage;
	switch (desc.type)
	{
	case(BufferType::Uniform): 
		usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			break;
	case(BufferType::SSBO): 
		usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		break;
	}

	VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	createBuffer(size, usage, properties, vkBuffer->buffer, vkBuffer->memory);
	vkMapMemory(device, vkBuffer->memory, 0, desc.size, 0, &vkBuffer->mapped);
}

void VkBufferManager::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex =  VkUtils::findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate buffer memory!");
	}

	vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

void VkBufferManager::destroyBuffer(Buffer* buffer)
{
	VulkanBuffer* vkBuffer = (VulkanBuffer*)buffer;
	vkDestroyBuffer(device, vkBuffer->buffer, nullptr);
	vkFreeMemory(device, vkBuffer->memory, nullptr);
	vkBuffer->buffer = VK_NULL_HANDLE;
}

void VkBufferManager::updateBuffer(Buffer* buffer, const void* data, size_t size)
{
	VulkanBuffer* vkBuffer = (VulkanBuffer*)buffer;

	memcpy(static_cast<char*>(vkBuffer->mapped), data, vkBuffer->bufferSize);
}