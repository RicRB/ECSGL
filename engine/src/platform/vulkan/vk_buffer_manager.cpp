#include "vk_buffer_manager.h"
#include "vk_utils.h"
#include <stdexcept>

VkBufferManager::VkBufferManager(VkDevice device, VkPhysicalDevice physicalDevice) : device(device), physicalDevice(physicalDevice)
{
	bufferPool = std::make_unique<MemoryPool<VulkanBuffer, 256>>();
}

Buffer VkBufferManager::createBuffer(const BufferDescription& desc)
{
	Buffer buffer;
	buffer.handle = static_cast<MemoryHandle>(bufferPool->allocate());
	VulkanBuffer& vkBuffer = bufferPool->at(buffer.handle);
	VkDeviceSize size = desc.size;

	vkBuffer.buffer = VK_NULL_HANDLE;
	vkBuffer.memory = VK_NULL_HANDLE;
	vkBuffer.mapped = nullptr;

	vkBuffer.bufferSize = desc.size;
	vkBuffer.desc = desc;

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

	createBuffer(size, usage, properties, vkBuffer.buffer, vkBuffer.memory);
	vkMapMemory(device, vkBuffer.memory, 0, desc.size, 0, &vkBuffer.mapped);

	return buffer;
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

void VkBufferManager::destroyBuffer(Buffer buffer)
{
	VulkanBuffer& vkBuffer = bufferPool->at(buffer.handle);
	vkDestroyBuffer(device, vkBuffer.buffer, nullptr);
	vkFreeMemory(device, vkBuffer.memory, nullptr);
	vkBuffer.buffer = VK_NULL_HANDLE;
	bufferPool->deallocate(buffer.handle);
}

void VkBufferManager::updateBuffer(Buffer buffer, const void* data, size_t size)
{
	VulkanBuffer& vkBuffer = bufferPool->at(buffer.handle);

	memcpy(static_cast<char*>(vkBuffer.mapped), data, vkBuffer.bufferSize);
}

void VkBufferManager::getBufferData(Buffer buffer, void* data)
{
	VulkanBuffer& vkBuffer = bufferPool->at(buffer.handle);
	memcpy(data, static_cast<char*>(vkBuffer.mapped), vkBuffer.bufferSize);
}

BufferData& VkBufferManager::getBufferData(Buffer buffer)
{
	return bufferPool->at(buffer.handle);
}

BufferDescription VkBufferManager::getBufferDesc(Buffer buffer)
{
	VulkanBuffer& vkBuffer = bufferPool->at(buffer.handle);
	return vkBuffer.desc;
}
