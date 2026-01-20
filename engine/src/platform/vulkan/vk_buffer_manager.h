#pragma once
#include "renderer/buffer_manager.h"
#include <vulkan/vulkan.h>
#include "vk_buffer.h"
#include "memory/memory_pool.hpp"
#include <memory>
class VkBufferManager : public BufferManager {
	VkDevice device;
	VkPhysicalDevice physicalDevice;
	std::unique_ptr<MemoryPool<VulkanBuffer, 256>> bufferPool;

public:
	VkBufferManager(VkDevice device, VkPhysicalDevice physicalDevice);
	~VkBufferManager() = default;
public:
	Buffer createBuffer(const BufferDescription& desc) override;
	void destroyBuffer(Buffer buffer) override;
	void updateBuffer(Buffer buffer, const void* data, size_t size) override;
	void getBufferData(Buffer buffer, void* data) override;
	BufferData& getBufferData(Buffer buffer) override;
	BufferDescription getBufferDesc(Buffer buffer) override;
	//void createVertexBuffer(const std::vector<Vertex>& vertices, VkVertexBuffer* vertexBuffer);
	//void createIndexBuffer(const std::vector<unsigned int>& indices, VkIndexBuffer* indexBuffer);
private:

	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
};