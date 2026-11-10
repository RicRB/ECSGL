#include "vk_fence.h"

VulkanFence::~VulkanFence()
{
	if (m_fence == VK_NULL_HANDLE) return;
	vkWaitForFences(m_device, 1, &m_fence, VK_TRUE, UINT64_MAX);
	vkDestroyFence(m_device, m_fence, nullptr);
}

void VulkanFence::wait()
{
	if (m_fence == VK_NULL_HANDLE) return;
	vkWaitForFences(m_device, 1, &m_fence, VK_TRUE, UINT64_MAX);
}

bool VulkanFence::isSignaled()
{
	if (m_fence == VK_NULL_HANDLE) return false;
	VkResult result = vkGetFenceStatus(m_device, m_fence);
	return result == VK_SUCCESS;
}

void VulkanFence::reset()
{
	vkResetFences(m_device, 1, &m_fence);
}

bool VulkanFence::isValid()
{
	return !(m_fence == VK_NULL_HANDLE);
}
