#pragma once
#include "renderer/buffer.h"
#include <vulkan/vulkan.h>
struct VkVertexBuffer : public VertexBuffer {
	VkBuffer buffer = nullptr;
	VkDeviceMemory memory = nullptr;
};
struct VkIndexBuffer : public IndexBuffer {
	VkBuffer buffer = nullptr;
	VkDeviceMemory memory = nullptr;
};

struct VkUniformBuffer : public UniformBuffer {
	VkBuffer buffer = nullptr;
	VkDeviceMemory memory = nullptr;
	void* mapped = nullptr;
	VkDeviceSize bufferSize;
};