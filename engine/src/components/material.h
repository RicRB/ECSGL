#pragma once
#include "glm/glm.hpp"
#include "renderer/shader.h"
#include "renderer/texture.h"
#include "renderer/buffer.h"
#include "renderer/descriptor_set.h"
#include "renderer/uniform_objects.h"
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
	MaterialData();
	MaterialID id;
	ShaderID shaderID;
	MaterialUBO ubo;
	// vulkan specific
	std::shared_ptr<DescriptorSet> descriptorSet;
	std::shared_ptr<UniformBuffer> uniformBuffer;
};
struct Material {
	MaterialID id;
};