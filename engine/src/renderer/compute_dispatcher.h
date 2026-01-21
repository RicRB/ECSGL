#pragma once
#include "compute_shader.h"
#include "fence.h"
#include "buffer.h"
#include "texture.h"
#include <string_view>
class ComputeDispatcher {
	ComputeShaderData& m_shaderData;
	std::unique_ptr<DescriptorSet> m_descriptorSet;
	std::unique_ptr<Fence> m_fence;
public:
	ComputeDispatcher(const ComputeShader& shader);
	void dispatch(uint32_t threadGroupX, uint32_t threadGroupY, uint32_t threadGroupZ);
	void setBuffer(int binding, Buffer buffer, size_t size);
	void setBuffer(std::string_view name, Buffer buffer, size_t size);
	void setTexture(int binding, Texture* texture);
	void setTexture(std::string_view name, Texture* texture);
	bool isDone() const;
	void wait();
};