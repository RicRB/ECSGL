#pragma once
#include <memory>


enum class BufferType {
	Uniform,
	SSBO,
};

struct BufferDesc {
	size_t size;
	BufferType type;
};


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

struct Buffer {
	BufferDesc desc;
	static std::shared_ptr<Buffer> Create();
	static std::unique_ptr<Buffer> CreateUnique();
};