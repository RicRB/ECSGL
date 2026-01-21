#pragma once
#include "renderer/compute_manager.h"
#include <vulkan/vulkan.h>

class GlComputeManager : public ComputeManager {
public:
	virtual ~GlComputeManager() = default;
	virtual void dispatchComputeShader(ComputeShaderData& computeShaderData, DescriptorSet* set, Fence* fence, uint32_t threadsX, uint32_t threadsY, uint32_t threadsZ) override;
	virtual void setBuffer(DescriptorSet* set, BufferData* buffer, int binding) override;
	virtual void setBuffer(ComputeShaderData& shaderData, DescriptorSet* set, BufferData* buffer, std::string_view name) override;
	virtual void setImage(DescriptorSet* set, Image* image, DescriptorBinding& binding) override;
	virtual void setImage(ComputeShaderData& shaderData, DescriptorSet* set, Image* image, std::string_view name) override;
};