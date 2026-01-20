#include "gl_buffer_manager.h"


Buffer GlBufferManager::createBuffer(const BufferDescription& desc)
{
	return Buffer();
}

void GlBufferManager::destroyBuffer(Buffer buffer)
{
}

void GlBufferManager::updateBuffer(Buffer buffer, const void* data, size_t size)
{
}

void GlBufferManager::getBufferData(Buffer buffer, void* data)
{
}

BufferData& GlBufferManager::getBufferData(Buffer buffer)
{
	return *(new BufferData());
}

BufferDescription GlBufferManager::getBufferDesc(Buffer buffer)
{
	return BufferDescription();
}
