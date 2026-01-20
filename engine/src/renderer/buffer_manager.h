#pragma once
#include "renderer/buffer.h"

class BufferManager {
public:
	virtual ~BufferManager() = default;
public:
	virtual void createBuffer(Buffer* buffer, const BufferDesc& desc) = 0;
	virtual void destroyBuffer(Buffer* buffer) = 0;
	virtual void updateBuffer(Buffer* buffer, const void* data, size_t size) = 0;
	virtual void getBufferData(Buffer* buffer, void* data) = 0;
};