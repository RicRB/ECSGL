#pragma once
#include "renderer/buffer_manager.h"
#include <vulkan/vulkan.h>
#include "vk_buffer.h"
class VkBufferManager : public BufferManager {
	VkDevice device;
	VkPhysicalDevice physicalDevice;
public:
	VkBufferManager(VkDevice device, VkPhysicalDevice physicalDevice) : device(device), physicalDevice(physicalDevice) {}
	~VkBufferManager() = default;
public:
	void createBuffer(Buffer* buffer, const BufferDesc& desc) override;
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void destroyBuffer(Buffer* buffer) override;
	void updateBuffer(Buffer* buffer, const void* data, size_t size) override;
	void getBufferData(Buffer* buffer, void* data);

	//void createVertexBuffer(const std::vector<Vertex>& vertices, VkVertexBuffer* vertexBuffer);
	//void createIndexBuffer(const std::vector<unsigned int>& indices, VkIndexBuffer* indexBuffer);
private:
};