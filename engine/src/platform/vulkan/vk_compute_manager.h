#pragma once
#include "renderer/compute_manager.h"
#include <vulkan/vulkan.h>
#include "vk_buffer.h"
class VulkanComputeManager : public ComputeManager {
	VkDevice m_device;
	VkPhysicalDevice m_physicalDevice;
	VkCommandPool m_commandPool;
	VkQueue m_computeQueue;
public:
	VulkanComputeManager(VkDevice device, VkPhysicalDevice physicalDevice);
	~VulkanComputeManager() = default;
	void dispatchComputeShader(ComputeShaderData& computeShaderData, DescriptorSet* set, uint32_t threadsX, uint32_t threadsY, uint32_t threadsZ) override;
	void setBuffer(DescriptorSet* set, Buffer* buffer, int binding) override;
	void setBuffer(ComputeShaderData& shaderData, DescriptorSet* set, Buffer* buffer, std::string_view name) override;
};