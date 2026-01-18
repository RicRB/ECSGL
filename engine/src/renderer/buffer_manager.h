#pragma once
#include "renderer/buffer.h"
#include "renderer/graphics_api.h"

enum class BufferType {
	Uniform,
	SSBO,
};

struct BufferDesc {
	size_t size;
	BufferType type;
};


class BufferManager {
public:
	virtual ~BufferManager() = default;
public:
	virtual void createBuffer(Buffer* buffer, const BufferDesc& desc) = 0;
	virtual void destroyBuffer(Buffer* buffer) = 0;
	virtual void updateBuffer(Buffer* buffer, const void* data, size_t size) = 0;
};