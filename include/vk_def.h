#pragma once
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include "uniform_objects.h"


struct alignas(256) VkMaterialUBO {
	glm::vec4 diffuseColor;
	glm::vec4 specularColor;
	glm::vec4 ambientColor;
};

struct VkUniformBuffer {
	VkBuffer buffer;
	VkDeviceMemory memory;
	void* mapped = nullptr;
	VkDeviceSize bufferSize;
};
struct PushConstants {
	glm::mat4 mvp;
	float time;
};