#pragma once
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include "uniform_objects.h"
#include <memory>

struct VkUniformBuffer {
	VkBuffer buffer = nullptr;
	VkDeviceMemory memory = nullptr;
	void* mapped = nullptr;
	VkDeviceSize bufferSize;
};

struct VkVertexBuffer {
	VkBuffer buffer = nullptr;
	VkDeviceMemory memory = nullptr;
};
struct VkIndexBuffer {
	VkBuffer buffer = nullptr;
	VkDeviceMemory memory = nullptr;
};