#pragma once
#include "glm/glm.hpp"
#include "shader.h"
#include "texture.h"
enum class VariableType
{
	TEXTURE,
	VEC3
};
using MaterialID = unsigned int;
// FIXME: find a better thing to do here, waste of memory
struct MaterialVariable
{
	VariableType type;
	union
	{
		Texture texture;
		glm::vec3 vec3;
	};
	MaterialVariable(Texture texture)
		:type(VariableType::TEXTURE), texture(texture)
	{}
	MaterialVariable(const glm::vec3& vec3)
		:type(VariableType::VEC3), vec3(vec3)
	{}
};

struct MaterialData {
	MaterialID id;
	ShaderID shaderID;
	VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
	VkUniformBuffer uniformBuffer;
	VkMaterialUBO ubo;

	MaterialVariable diffuse, specular;
	float shininess;
	MaterialData() : id(0), shaderID(0), descriptorSet(VK_NULL_HANDLE), uniformBuffer(VK_NULL_HANDLE), ubo(), diffuse(glm::vec3(0.0, 0.0, 0.0)), specular(glm::vec3(0.0, 0.0, 0.0)) {}
};
struct Material {
	MaterialID id;
};