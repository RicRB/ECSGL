#pragma once
#include "renderer/fence.h"
#include <vulkan/vulkan.h>
struct VulkanFence : public Fence {
private: 

public:
	VkFence m_fence = VK_NULL_HANDLE;
	VkDevice m_device = VK_NULL_HANDLE;
	~VulkanFence() override;
	void wait() override;
	bool isSignaled() override;
	void reset() override;
	bool isValid() override;
};