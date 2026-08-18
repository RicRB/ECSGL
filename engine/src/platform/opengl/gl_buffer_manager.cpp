#include "gl_buffer_manager.h"
#include "gl_def.h"

GlBufferManager::GlBufferManager()
{
	bufferPool = std::make_unique<MemoryPool<GlBuffer, 256>>();
}

static GLenum getBufferTarget(BufferType type)
{
	switch (type)
	{
	case BufferType::Uniform:
		return GL_UNIFORM_BUFFER;
	case BufferType::SSBO:
		return GL_SHADER_STORAGE_BUFFER;
	}
	return GL_SHADER_STORAGE_BUFFER;
}

Buffer GlBufferManager::createBuffer(const BufferDescription& desc)
{
	Buffer buffer;
	buffer.handle = bufferPool->allocate();
	GlBuffer& glBuffer = bufferPool->at(buffer.handle);
	glBuffer.desc = desc;
	glBuffer.bufferID = 0;

	GLenum target = getBufferTarget(desc.type);
	GLCall(glGenBuffers(1, &glBuffer.bufferID));
	GLCall(glBindBuffer(target, glBuffer.bufferID));
	GLCall(glBufferData(target, desc.size, nullptr, GL_DYNAMIC_DRAW));
	GLCall(glBindBuffer(target, 0));

	return buffer;
}

void GlBufferManager::destroyBuffer(Buffer buffer)
{
	GlBuffer& glBuffer = bufferPool->at(buffer.handle);
	GLCall(glDeleteBuffers(1, &glBuffer.bufferID));
	glBuffer.bufferID = 0;
	bufferPool->deallocate(buffer.handle);
}

void GlBufferManager::updateBuffer(Buffer buffer, const void* data, size_t size)
{
	GlBuffer& glBuffer = bufferPool->at(buffer.handle);
	GLenum target = getBufferTarget(glBuffer.desc.type);
	GLCall(glBindBuffer(target, glBuffer.bufferID));
	GLCall(glBufferSubData(target, 0, size, data));
	GLCall(glBindBuffer(target, 0));
}

void GlBufferManager::getBufferData(Buffer buffer, void* data)
{
	GlBuffer& glBuffer = bufferPool->at(buffer.handle);
	GLenum target = getBufferTarget(glBuffer.desc.type);
	GLCall(glBindBuffer(target, glBuffer.bufferID));
	void* mapped = glMapBuffer(target, GL_READ_ONLY);
	if (mapped != nullptr)
	{
		memcpy(data, mapped, glBuffer.desc.size);
		glUnmapBuffer(target);
	}
	GLCall(glBindBuffer(target, 0));
}

BufferData& GlBufferManager::getBufferData(Buffer buffer)
{
	return bufferPool->at(buffer.handle);
}

BufferDescription GlBufferManager::getBufferDesc(Buffer buffer)
{
	return bufferPool->at(buffer.handle).desc;
}