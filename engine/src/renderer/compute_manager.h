#pragma once
#include "compute_shader.h"
#include <string_view>
class ComputeManager {
	virtual void dispatchComputeShader(ComputeShader& computeShader) = 0;
	virtual void setBuffer(DescriptorSet* set, int binding, void* bufferData, size_t size) = 0;
	virtual void setBuffer(DescriptorSet* set, std::string_view name, void* bufferData, size_t size) = 0;
	virtual void setImage(DescriptorSet* set, int binding, void* imageData, size_t width, size_t height) = 0;
	virtual void setImage(DescriptorSet* set, std::string_view name, void* imageData, size_t width, size_t height) = 0;
};