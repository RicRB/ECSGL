#pragma once
#include "glm/glm.hpp"
#include "renderer/shader.h"
#include "renderer/texture.h"
#include "renderer/buffer.h"
#include "renderer/descriptor_set.h"
#include "renderer/uniform_objects.h"
#include "memory/memory_pool.hpp"

constexpr uint32_t ALBEDO_MAP_BINDING = 0;
constexpr uint32_t SPECULAR_MAP_BINDING = 1;
constexpr uint32_t NORMAL_MAP_BINDING = 2;

struct MaterialData {
	MaterialData();
	ShaderDataHandle shaderHandle;
	MaterialUBO ubo;
	// vulkan specific
	Texture albedoMap;
	Texture specularMap;
	Texture normalMap;
	std::shared_ptr<DescriptorSet> descriptorSet;
	std::shared_ptr<UniformBuffer> uniformBuffer;
};
struct Material {
	MaterialDataHandle handle;
	Material(const MaterialDataHandle h = MaterialDataHandle()) : handle(h) {};
	
};

using MaterialDataHandle = MemoryHandle<MaterialData>;