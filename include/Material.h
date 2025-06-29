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
	ShaderID shaderID;
	VkDescriptorSet descriptorSet;
	VkUniformBuffer uniformBuffer;
	VkMaterialUBO ubo;

	MaterialVariable diffuse, specular;
	float shininess;
};
struct Material {
	MaterialID id;
	ShaderID shaderID;
	MaterialData* data;
};