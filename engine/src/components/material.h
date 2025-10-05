#pragma once
#include "glm/glm.hpp"
#include "renderer/shader.h"
#include "renderer/texture.h"
#include "renderer/buffer.h"
#include "renderer/descriptor_set.h"
#include "renderer/uniform_objects.h"

using MaterialID = unsigned int;
constexpr uint32_t ALBEDO_MAP_BINDING = 0;
constexpr uint32_t SPECULAR_MAP_BINDING = 1;
constexpr uint32_t NORMAL_MAP_BINDING = 2;

struct MaterialData {
	MaterialData();
	MaterialID id;
	ShaderID shaderID;
	MaterialUBO ubo;
	// vulkan specific
	Texture albedoMap;
	Texture specularMap;
	Texture normalMap;
	std::shared_ptr<DescriptorSet> descriptorSet;
	std::shared_ptr<UniformBuffer> uniformBuffer;
};
struct Material {
	MaterialID id;
};