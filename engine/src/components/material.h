#pragma once
#include "glm/glm.hpp"
#include "renderer/shader.h"
#include "renderer/texture.h"
#include "renderer/buffer.h"
#include "renderer/descriptor_set.h"
#include "renderer/uniform_objects.h"

using MaterialID = unsigned int;

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