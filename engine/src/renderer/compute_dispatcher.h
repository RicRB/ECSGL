#pragma once
#include "compute_shader.h"
#include <string_view>
class ComputeDispatcher {
	ComputeShaderData& m_shaderData;
	std::unique_ptr<DescriptorSet> m_descriptorSet;
public:
	ComputeDispatcher(const ComputeShader& shader);
	void dispatch(uint32_t threadGroupX, uint32_t threadGroupY, uint32_t threadGroupZ);
	void setBuffer(int binding, void* bufferData, size_t size);
	void setBuffer(std::string_view name, void* bufferData, size_t size);
	void setImage(int binding, void* imageData, size_t width, size_t height);
	void setImage(std::string_view name, void* imageData, size_t width, size_t height);
};