#include "buffer.h"
#include "core/engine.h"
#include "platform/vulkan/vk_buffer.h"
#include "platform/opengl/gl_buffer.h"
std::shared_ptr<VertexBuffer> VertexBuffer::Create()
{
	switch (graphics::getGraphicsAPIType()) {
	case GRAPHICS_API_VULKAN: return std::make_shared<VkVertexBuffer>();
	case GRAPHICS_API_OPENGL: return std::make_shared<VertexBuffer>();
	}

	return nullptr;
}

std::shared_ptr<IndexBuffer> IndexBuffer::Create()
{
	switch (graphics::getGraphicsAPIType()) {
	case GRAPHICS_API_VULKAN: return std::make_shared<VkIndexBuffer>();
	case GRAPHICS_API_OPENGL: return std::make_shared<GlIndexBuffer>();
	}
	return nullptr;
}

std::shared_ptr<UniformBuffer> UniformBuffer::Create()
{
	switch (graphics::getGraphicsAPIType()) {
	case GRAPHICS_API_VULKAN: return std::make_shared<VkUniformBuffer>();
	case GRAPHICS_API_OPENGL: return std::make_shared<GlUniformBuffer>();
	}
	return nullptr;
}

std::shared_ptr<ComputeBuffer> ComputeBuffer::Create()
{
	switch (graphics::getGraphicsAPIType()) {
	case GRAPHICS_API_VULKAN: return std::make_shared<VkComputeBuffer>();
	case GRAPHICS_API_OPENGL: return std::make_shared<GlComputeBuffer>();
	}
	return nullptr;
}

std::shared_ptr<BufferData> BufferData::Create()
{
	switch (graphics::getGraphicsAPIType()) {
	case GRAPHICS_API_VULKAN: return std::make_shared<VulkanBuffer>();
	case GRAPHICS_API_OPENGL: return std::make_shared<GlBuffer>();
	}
	return std::shared_ptr<BufferData>();
}

std::unique_ptr<BufferData> BufferData::CreateUnique()
{
	switch (graphics::getGraphicsAPIType()) {
	case GRAPHICS_API_VULKAN: return std::make_unique<VulkanBuffer>();
	case GRAPHICS_API_OPENGL: return std::make_unique<GlBuffer>();
	}
	return nullptr;
}
