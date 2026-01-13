#pragma once
#include <memory>
struct VertexBuffer {
	static std::shared_ptr<VertexBuffer> Create();
}; 

struct IndexBuffer {
	static std::shared_ptr<IndexBuffer> Create();
};

struct UniformBuffer {
	static std::shared_ptr<UniformBuffer> Create();
};

struct ComputeBuffer {
	static std::shared_ptr<ComputeBuffer> Create();
};