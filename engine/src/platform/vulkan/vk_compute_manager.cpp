#include "vk_compute_manager.h"
#include <vulkan/vulkan.h>
#include "vk_descriptors.h"
#include "vk_utils.h"
#include "vk_program.h"
#include "vk_descriptor_set.h"
VulkanComputeManager::VulkanComputeManager(VkDevice device, VkPhysicalDevice physicalDevice)
{
	m_device = device;
	m_physicalDevice = physicalDevice;

	// Find a suitable queue family for compute
	uint32_t queueFamilyIndex = VkUtils::findComputeQueueFamily(physicalDevice);

	// Create command pool
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueFamilyIndex;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Allow individual buffer resets

	vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool);

	// Get the queue
	vkGetDeviceQueue(m_device, queueFamilyIndex, 0, &m_computeQueue);
}
void VulkanComputeManager::dispatchComputeShader(ComputeShaderData& computeShaderData, DescriptorSet* set, uint32_t threadsX, uint32_t threadsY, uint32_t threadsZ)
{
	VkCommandBuffer commandBuffer = VkUtils::beginSingleTimeCommands(m_device, m_commandPool);
	VkProgram* vkProgram = static_cast<VkProgram*>(computeShaderData.program.get());
	VulkanDescriptorSet* vkDescriptorSet = static_cast<VulkanDescriptorSet*>(set);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, vkProgram->pipeline);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, vkProgram->pipelineLayout, 0, 1, &vkDescriptorSet->set, 0, 0);
	vkCmdDispatch(commandBuffer, threadsX, threadsY, threadsZ);

	VkUtils::endSingleTimeCommands(m_device, m_commandPool, m_computeQueue, commandBuffer);
}

void VulkanComputeManager::setBuffer(DescriptorSet* set, Buffer* buffer, int binding)
{
	DescriptorWriter writer;
	VulkanBuffer* vulkanBuffer = static_cast<VulkanBuffer*>(buffer);
	writer.writeBuffer(binding, vulkanBuffer->buffer, vulkanBuffer->bufferSize, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	writer.updateSet(m_device, static_cast<VulkanDescriptorSet*>(set)->set);
}

void VulkanComputeManager::setBuffer(ComputeShaderData& shaderData, DescriptorSet* set, Buffer* buffer, std::string_view name)
{
	DescriptorSetReflection::Binding binding = shaderData.descriptorReflection.bindings[shaderData.descriptorReflection.nameToIndex[name.data()]];
	this->setBuffer(set, buffer, binding.binding);
}
