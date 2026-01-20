#pragma once
#include "renderer/buffer_manager.h"

class GlBufferManager : public BufferManager {
public:
	GlBufferManager() = default;
	~GlBufferManager() = default;
public:
	Buffer createBuffer(const BufferDescription& desc) override;
	void destroyBuffer(Buffer buffer) override;
	void updateBuffer(Buffer buffer, const void* data, size_t size) override;
	void getBufferData(Buffer buffer, void* data) override;
	virtual BufferData& getBufferData(Buffer buffer) override;
	virtual BufferDescription getBufferDesc(Buffer buffer) override;
};