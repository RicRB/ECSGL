#pragma once
#include "compute_shader.h"
#include <string_view>
#include "renderer/buffer.h"
#include "renderer/fence.h"
#include "renderer/image.h"

class ComputeManager {
public:
	virtual void dispatchComputeShader(ComputeShaderData& computeShaderData, DescriptorSet* set, Fence* fence, uint32_t threadsX, uint32_t threadsY, uint32_t threadsZ) = 0;
	virtual void dispatchComputeShaderAsync(ComputeShaderData& computeShaderData, DescriptorSet* set, Fence* fence, uint32_t threadsX, uint32_t threadsY, uint32_t threadsZ) = 0;
	virtual void setBuffer(DescriptorSet* set, BufferData* buffer, int binding) = 0;
	virtual void setBuffer(ComputeShaderData& shaderData, DescriptorSet* set, BufferData* buffer, std::string_view name) = 0;
	virtual void setImage(DescriptorSet* set, Image* image, DescriptorBinding& binding) = 0;
	virtual void setImage(ComputeShaderData& shaderData, DescriptorSet* set, Image* image, std::string_view name) = 0;
};