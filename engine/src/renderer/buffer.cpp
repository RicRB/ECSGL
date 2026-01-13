#include "buffer.h"
#include "core/engine.h"
#include "platform/vulkan/vk_buffer.h"
#include "platform/opengl/gl_buffer.h"
std::shared_ptr<VertexBuffer> VertexBuffer::Create()
{
	switch (engine::currentEngine->renderer.rendererType) {
	case GRAPHICS_API_VULKAN: return std::make_shared<VkVertexBuffer>();
	case GRAPHICS_API_OPENGL: return std::make_shared<VertexBuffer>();
	}

	return nullptr;
}

std::shared_ptr<IndexBuffer> IndexBuffer::Create()
{
	switch (engine::currentEngine->renderer.rendererType) {
	case GRAPHICS_API_VULKAN: return std::make_shared<VkIndexBuffer>();
	case GRAPHICS_API_OPENGL: return std::make_shared<GlIndexBuffer>();
	}
	return nullptr;
}

std::shared_ptr<UniformBuffer> UniformBuffer::Create()
{
	switch (engine::currentEngine->renderer.rendererType) {
	case GRAPHICS_API_VULKAN: return std::make_shared<VkUniformBuffer>();
	case GRAPHICS_API_OPENGL: return std::make_shared<GlUniformBuffer>();
	}
	return nullptr;
}

std::shared_ptr<ComputeBuffer> ComputeBuffer::Create()
{
	switch (engine::currentEngine->renderer.rendererType) {
	case GRAPHICS_API_VULKAN: return std::make_shared<VkComputeBuffer>();
	case GRAPHICS_API_OPENGL: return std::make_shared<GlComputeBuffer>();
	}
	return nullptr;
}