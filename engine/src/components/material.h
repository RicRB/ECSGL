#pragma once
#include "glm/glm.hpp"
#include "renderer/shader.h"
#include "renderer/texture.h"
#include "platform/opengl/gl_def.h"
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
	MaterialUBO ubo;

	// opengl specific
	GLuint gl_uniformBuffer;

	// vulkan specific
	VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
	VkUniformBuffer uniformBuffer;
};
struct Material {
	MaterialID id;
};