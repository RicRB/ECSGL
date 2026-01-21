#include "gl_compute_manager.h"

void GlComputeManager::dispatchComputeShader(ComputeShaderData& computeShaderData, DescriptorSet* set, Fence* fence, uint32_t threadsX, uint32_t threadsY, uint32_t threadsZ)
{
}

void GlComputeManager::setBuffer(DescriptorSet* set, BufferData* buffer, int binding)
{
}

void GlComputeManager::setBuffer(ComputeShaderData& shaderData, DescriptorSet* set, BufferData* buffer, std::string_view name)
{
}

void GlComputeManager::setImage(DescriptorSet* set, Image* image, DescriptorBinding& binding)
{
}

void GlComputeManager::setImage(ComputeShaderData& shaderData, DescriptorSet* set, Image* image, std::string_view name)
{
}
