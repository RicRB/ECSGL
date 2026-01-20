#pragma once
#include "renderer/compute_manager.h"
#include <vulkan/vulkan.h>

class GlComputeManager : public ComputeManager {
public:
	virtual ~GlComputeManager() = default;
	virtual void dispatchComputeShader(ComputeShaderData& computeShaderData, DescriptorSet* set, Fence* fence, uint32_t threadsX, uint32_t threadsY, uint32_t threadsZ) override;
	virtual void setBuffer(DescriptorSet* set, Buffer* buffer, int binding) override;
	virtual void setBuffer(ComputeShaderData& shaderData, DescriptorSet* set, Buffer* buffer, std::string_view name) override;

};