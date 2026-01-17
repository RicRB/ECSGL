#include "vk_compute_manager.h"
#include <vulkan/vulkan.h>
void VulkanComputeManager::dispatchComputeShader(ComputeShader& computeShader)
{
}

void VulkanComputeManager::setBuffer(ComputeShader& computeShader, int binding, void* bufferData, size_t size)
{
}

void VulkanComputeManager::setBuffer(ComputeShader& computeShader, std::string_view name, void* bufferData, size_t size)
{
}

void VulkanComputeManager::setImage(ComputeShader& computeShader, int binding, void* imageData, size_t width, size_t height)
{

}

void VulkanComputeManager::setImage(ComputeShader& computeShader, std::string_view name, void* imageData, size_t width, size_t height)
{
}
