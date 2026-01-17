#pragma once
#include "renderer/compute_manager.h"
#include <vulkan/vulkan.h>

class VulkanComputeManager : public ComputeManager {
	public:
	virtual ~VulkanComputeManager() = default;
	virtual void dispatchComputeShader(ComputeShader& computeShader) override;
	virtual void setBuffer(ComputeShader& computeShader, int binding, void* bufferData, size_t size) override;
	virtual void setBuffer(ComputeShader& computeShader, std::string_view name, void* bufferData, size_t size) override;
	virtual void setImage(ComputeShader& computeShader, int binding, void* imageData, size_t width, size_t height) override;
	virtual void setImage(ComputeShader& computeShader, std::string_view name, void* imageData, size_t width, size_t height) override;
};