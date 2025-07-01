#include <vulkan/vulkan.h>
#include <glm/glm.hpp>


struct GlobalUniformBufferObject {
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};
struct alignas(256) VkMaterialUBO {
	glm::vec4 diffuseColor;
	glm::vec4 specularColor;
	glm::vec4 ambientColor;
};

struct VkUniformBuffer {
	VkBuffer buffer;
	VkDeviceMemory memory;
	void* mapped = nullptr;
	uint32_t bufferSize;
};
struct PushConstants {
	glm::mat4 mvp;
	float time;
};