#include "vk_fence.h"
#
void VulkanFence::wait()
{
	vkWaitForFences(m_device, 1, &m_fence, VK_TRUE, UINT64_MAX);
}

bool VulkanFence::isSignaled()
{
	VkResult result = vkGetFenceStatus(m_device, m_fence);
	return result == VK_SUCCESS;
}
