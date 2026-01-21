#pragma once
#include "renderer/compute_manager.h"
#include <vulkan/vulkan.h>
#include "vk_buffer.h"
#include "vk_fence.h"
#include "vk_image.h"
class VulkanComputeManager : public ComputeManager {
	VkDevice m_device;
	VkPhysicalDevice m_physicalDevice;
	VkCommandPool m_commandPool;
	VkQueue m_computeQueue;
public:
	VulkanComputeManager(VkDevice device, VkPhysicalDevice physicalDevice);
	~VulkanComputeManager() = default;
	void dispatchComputeShader(ComputeShaderData& computeShaderData, DescriptorSet* set, Fence* fence, uint32_t threadsX, uint32_t threadsY, uint32_t threadsZ) override;
	void setBuffer(DescriptorSet* set, BufferData* buffer, int binding) override;
	void setBuffer(ComputeShaderData& shaderData, DescriptorSet* set, BufferData* buffer, std::string_view name) override;
	void setImage(DescriptorSet* set, Image* image, DescriptorBinding& binding) override;
	void setImage(ComputeShaderData& shaderData, DescriptorSet* set, Image* image, std::string_view name) override;
};