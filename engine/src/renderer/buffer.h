#pragma once
#include <memory>
#include "memory/memory_pool.hpp"

enum class BufferType {
	Uniform,
	SSBO,
};

struct BufferDescription {
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

struct BufferData {
	BufferDescription desc;
	static std::shared_ptr<BufferData> Create();
	static std::unique_ptr<BufferData> CreateUnique();
	BufferData() = default;
};

struct Buffer {
	MemoryHandle handle;
	Buffer(const MemoryHandle h = MemoryHandle()) : handle(h) {};
};