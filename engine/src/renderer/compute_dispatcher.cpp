#include "compute_dispatcher.h"
#include "core/engine.h"
ComputeDispatcher::ComputeDispatcher(const ComputeShader& shader)
{
	descriptorSet = DescriptorSet::CreateUnique();
	graphics::GetGraphicsAPI()->createDescriptorSet(engine::GetEngine().resourceManager.m_computeShaders->at(shader.handle).program.get(), descriptorSet.get());
}

void ComputeDispatcher::dispatch()
{
	//graphics::GetEngine().computeManager->dispatchComputeShader(const_cast<ComputeShader&>(ComputeShader(descriptorSet->getProgram()->computeShaderHandle)));
}

void ComputeDispatcher::setBuffer(int binding, void* bufferData, size_t size)
{
}

void ComputeDispatcher::setBuffer(std::string_view name, void* bufferData, size_t size)
{
}

void ComputeDispatcher::setImage(int binding, void* imageData, size_t width, size_t height)
{
}

void ComputeDispatcher::setImage(std::string_view name, void* imageData, size_t width, size_t height)
{
}
