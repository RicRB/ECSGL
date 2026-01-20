#pragma once
#include "compute_shader.h"
#include "fence.h"
#include "buffer.h"
#include <string_view>
class ComputeDispatcher {
	ComputeShaderData& m_shaderData;
	std::unique_ptr<DescriptorSet> m_descriptorSet;
	std::unique_ptr<Fence> m_fence;
public:
	ComputeDispatcher(const ComputeShader& shader);
	void dispatch(uint32_t threadGroupX, uint32_t threadGroupY, uint32_t threadGroupZ);
	void setBuffer(int binding, Buffer* bufferData, size_t size);
	void setBuffer(std::string_view name, Buffer* bufferData, size_t size);
	bool isDone() const;
	void wait();
	//void setImage(int binding, void* imageData, size_t width, size_t height);
	//void setImage(std::string_view name, void* imageData, size_t width, size_t height);

};