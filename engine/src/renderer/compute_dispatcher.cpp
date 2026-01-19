#include "compute_dispatcher.h"
#include "core/engine.h"
ComputeDispatcher::ComputeDispatcher(const ComputeShader& shader) : m_shaderData(engine::GetEngine().resourceManager.m_computeShaders->at(shader.handle))
{
	m_descriptorSet = DescriptorSet::CreateUnique();
	graphics::GetGraphicsAPI()->createDescriptorSet(m_shaderData.program.get(), m_descriptorSet.get());
}

void ComputeDispatcher::dispatch(uint32_t threadGroupX, uint32_t threadGroupY, uint32_t threadGroupZ)
{
	engine::GetEngine().computeManager->get()->dispatchComputeShader(m_shaderData, m_descriptorSet.get(), threadGroupX, threadGroupY, threadGroupZ);
}

void ComputeDispatcher::setBuffer(int binding, void* bufferData, size_t size)
{
	engine::GetEngine().computeManager->get()->setBuffer(m_descriptorSet.get(), static_cast<Buffer*>(bufferData), binding);
}

void ComputeDispatcher::setBuffer(std::string_view name, void* bufferData, size_t size)
{
	engine::GetEngine().computeManager->get()->setBuffer(m_shaderData, m_descriptorSet.get(), static_cast<Buffer*>(bufferData), name);
}
