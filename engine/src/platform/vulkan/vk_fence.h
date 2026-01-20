#pragma once
#include "renderer/fence.h"
#include <vulkan/vulkan.h>
struct VulkanFence : public Fence {
private: 

public:
	VkFence m_fence;
	VkDevice m_device;
	void wait() override;
	bool isSignaled() override;
};