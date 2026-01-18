#pragma once
#include "compute_shader.h"
#include <string_view>
class ComputeDispatcher {
public:
	std::unique_ptr<DescriptorSet> descriptorSet;
	ComputeDispatcher(const ComputeShader& shader);
	void dispatch();
	void setBuffer(int binding, void* bufferData, size_t size);
	void setBuffer(std::string_view name, void* bufferData, size_t size);
	void setImage(int binding, void* imageData, size_t width, size_t height);
	void setImage(std::string_view name, void* imageData, size_t width, size_t height);
};