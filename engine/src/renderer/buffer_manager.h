#pragma once
#include "renderer/buffer.h"
#include "memory/memory_pool.hpp"
class BufferManager {

public:
	virtual ~BufferManager() = default;
	virtual Buffer createBuffer(const BufferDescription& desc) = 0;
	virtual void destroyBuffer(Buffer buffer) = 0;
	virtual void updateBuffer(Buffer buffer, const void* data, size_t size) = 0;
	virtual void getBufferData(Buffer buffer, void* data) = 0;
	virtual BufferData& getBufferData(Buffer buffer) = 0;
	virtual BufferDescription getBufferDesc(Buffer buffer) = 0;
};