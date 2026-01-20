#pragma once
#include "compute_shader.h"
#include <string_view>
#include "renderer/buffer.h"
#include "renderer/fence.h"
class ComputeManager {
public:
	virtual void dispatchComputeShader(ComputeShaderData& computeShaderData, DescriptorSet* set, Fence* fence, uint32_t threadsX, uint32_t threadsY, uint32_t threadsZ) = 0;
	virtual void setBuffer(DescriptorSet* set, Buffer* buffer, int binding) = 0;
	virtual void setBuffer(ComputeShaderData& shaderData, DescriptorSet* set, Buffer* buffer, std::string_view name) = 0;
};