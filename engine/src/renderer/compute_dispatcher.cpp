#include "compute_dispatcher.h"
#include "core/engine.h"
ComputeDispatcher::ComputeDispatcher(const ComputeShader shader) : m_shaderData(engine::GetEngine().resourceManager.m_computeShaders->at(shader.handle))
{
	m_descriptorSet = DescriptorSet::CreateUnique();
	m_fence = Fence::createUnique();
	graphics::GetGraphicsAPI()->createDescriptorSet(m_shaderData.program.get(), m_descriptorSet.get());
}

void ComputeDispatcher::dispatch(uint32_t threadGroupX, uint32_t threadGroupY, uint32_t threadGroupZ)
{
	if (m_fence->isValid()) m_fence->reset();
	engine::GetEngine().computeManager->get()->dispatchComputeShaderAsync(m_shaderData, m_descriptorSet.get(), m_fence.get(), threadGroupX, threadGroupY, threadGroupZ);
}

void ComputeDispatcher::dispatchAndWait(uint32_t threadGroupX, uint32_t threadGroupY, uint32_t threadGroupZ)
{
	if (m_fence->isValid()) m_fence->reset();
	engine::GetEngine().computeManager->get()->dispatchComputeShader(m_shaderData, m_descriptorSet.get(), m_fence.get(), threadGroupX, threadGroupY, threadGroupZ);
}

void ComputeDispatcher::setBuffer(int binding, Buffer buffer)
{
	BufferData& bufferData = engine::GetEngine().bufferManager->get()->getBufferData(buffer);
	engine::GetEngine().computeManager->get()->setBuffer(m_descriptorSet.get(), &bufferData, binding);
}

void ComputeDispatcher::setBuffer(std::string_view name, Buffer buffer)
{
	BufferData& bufferData = engine::GetBufferManager().getBufferData(buffer);
	engine::GetEngine().computeManager->get()->setBuffer(m_shaderData, m_descriptorSet.get(), &bufferData, name);
}

void ComputeDispatcher::setTexture(int binding, Texture texture)
{
	ResourceManager& resourceManager = engine::GetEngine().resourceManager;
	TextureData& textureData = resourceManager.m_textures->at(texture.handle);
	for (size_t i = 0; i < m_shaderData.descriptorReflection.bindings.size(); i++)
	{
		if (m_shaderData.descriptorReflection.bindings[i].binding == binding) {
			DescriptorBinding& binding = m_shaderData.descriptorReflection.bindings[i];
			engine::GetEngine().computeManager->get()->setImage(m_descriptorSet.get(), textureData.image.get(), binding);
			return;
		}
	}
}

void ComputeDispatcher::setTexture(std::string_view name, Texture texture)
{
	ResourceManager& resourceManager = engine::GetEngine().resourceManager;
	TextureData& textureData = resourceManager.m_textures->at(texture.handle);
	engine::GetEngine().computeManager->get()->setImage(m_shaderData, m_descriptorSet.get(), textureData.image.get(), name);
}

bool ComputeDispatcher::isDone() const
{
	return m_fence->isSignaled();
}

void ComputeDispatcher::wait()
{
	m_fence->wait();
}
