#include "gl_compute_manager.h"

void GlComputeManager::dispatchComputeShader(ComputeShaderData& computeShaderData, DescriptorSet* set, uint32_t threadsX, uint32_t threadsY, uint32_t threadsZ)
{
}

void GlComputeManager::setBuffer(DescriptorSet* set, Buffer* buffer, int binding)
{
}

void GlComputeManager::setBuffer(ComputeShaderData& shaderData, DescriptorSet* set, Buffer* buffer, std::string_view name)
{
}
