#pragma once
#include "renderer/buffer.h"
#include <vulkan/vulkan.h>


struct VulkanBuffer : public Buffer {
	VkBuffer buffer = nullptr;
	VkDeviceMemory memory = nullptr;
	void* mapped = nullptr;
	VkDeviceSize bufferSize;
	int count;
	int stride;
};

struct VkVertexBuffer : public VertexBuffer, public VulkanBuffer {
};
struct VkIndexBuffer : public IndexBuffer, public VulkanBuffer {
};

struct VkUniformBuffer : public UniformBuffer, public VulkanBuffer {
};

struct VkComputeBuffer : public ComputeBuffer, public VulkanBuffer {
};
