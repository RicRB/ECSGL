#pragma once
#include "renderer/buffer_manager.h"
#include "gl_buffer.h"
#include "memory/memory_pool.hpp"
#include <memory>

class GlBufferManager : public BufferManager {
	std::unique_ptr<MemoryPool<GlBuffer, 256>> bufferPool;
public:
	GlBufferManager();
	~GlBufferManager() = default;
public:
	Buffer createBuffer(const BufferDescription& desc) override;
	void destroyBuffer(Buffer buffer) override;
	void updateBuffer(Buffer buffer, const void* data, size_t size) override;
	void getBufferData(Buffer buffer, void* data) override;
	virtual BufferData& getBufferData(Buffer buffer) override;
	virtual BufferDescription getBufferDesc(Buffer buffer) override;
};