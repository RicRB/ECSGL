#pragma once
#include "vk_def.h"
#include <array>
using MeshID = unsigned int;
using GLVertexID = GLuint;
using GLIndexID = GLuint;


struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 textCoords;

	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}
	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, position);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, normal);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, textCoords);

		return attributeDescriptions;
	}
};
struct VkVertexBuffer {
	VkBuffer buffer = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
};
struct VkIndexBuffer {
	VkBuffer buffer = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
};
struct Mesh
{
	MeshID id;
};
struct MeshData
{
	MeshID id;
	GLVertexID vertexID;
	GLIndexID indexID;
	VkVertexBuffer vertexBuffer;
	VkIndexBuffer indexBuffer;
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	unsigned int vertexCount;
};