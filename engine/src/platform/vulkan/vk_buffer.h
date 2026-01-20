#pragma once
#include "renderer/buffer.h"
#include <vulkan/vulkan.h>


struct VulkanBuffer : public BufferData {
	VkBuffer buffer = nullptr;
	VkDeviceMemory memory = nullptr;
	void* mapped = nullptr;
	VkDeviceSize bufferSize;
	int count;
	int stride;
	VulkanBuffer() = default;
};

struct VkVertexBuffer : public VertexBuffer, public VulkanBuffer {
};
struct VkIndexBuffer : public IndexBuffer, public VulkanBuffer {
};

struct VkUniformBuffer : public UniformBuffer, public VulkanBuffer {
};

struct VkComputeBuffer : public ComputeBuffer, public VulkanBuffer {
};
