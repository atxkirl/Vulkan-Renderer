/*!
\file		VulkanCommandPool.cpp
\date		10/09/2023

\author		Adrian Tan
\email		t.xingkhiangadrian\@digipen.edu

\brief		Contains implementation for VulkanCommandPool class functions.

\copyright	All content © 2023 DigiPen (SINGAPORE) Corporation, all rights reserved.
			Reproduction or disclosure of this file or its contents without the
			prior written consent of DigiPen Institute of Technology is prohibited.
__________________________________________________________________________________*/

#include "meowpch.h"

#include "VulkanCommandPool.h"

#include "VulkanContext.h"
#include "VulkanLogicalDevice.h"
#include "VulkanPhysicalDevice.h"
#include "VulkanQuery.h"

namespace Nya
{
	void VulkanCommandPool::Init()
	{
		const QueueFamilyIndices queueFamilyIndices = VulkanQuery::FindQueueFamilies(VulkanPhysicalDevice::Get().GetPhysicalDevice(), VulkanContext::Get().GetSurface());

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = queueFamilyIndices.m_GraphicsFamily.value();

		if (vkCreateCommandPool(VulkanLogicalDevice::Get().GetLogicalDevice(), &poolInfo, nullptr, &m_CommandPool) != VK_SUCCESS)
			throw std::runtime_error("Failed to create command pool!");
	}

	void VulkanCommandPool::Cleanup() const
	{
		vkDestroyCommandPool(VulkanLogicalDevice::Get().GetLogicalDevice(), m_CommandPool, nullptr);
	}

	VkCommandPool VulkanCommandPool::GetCommandPool() const
	{
		return m_CommandPool;
	}
}
