/*!
\file		VulkanSyncObjects.cpp
\date		07/09/2023

\author		Adrian Tan
\email		t.xingkhiangadrian\@digipen.edu

\brief		Contains implementation for VulkanSyncObjects class functions.

\copyright	All content © 2023 DigiPen (SINGAPORE) Corporation, all rights reserved.
			Reproduction or disclosure of this file or its contents without the
			prior written consent of DigiPen Institute of Technology is prohibited.
__________________________________________________________________________________*/

#include "meowpch.h"

#include "VulkanSyncObjects.h"
#include "VulkanDefines.h"
#include "VulkanLogicalDevice.h"

namespace Nya
{
	void VulkanSyncObjects::Init()
	{
		m_ImageAvailableSemaphores.resize(g_MaxFramesInFlight);
		m_RenderFinishedSemaphores.resize(g_MaxFramesInFlight);
		m_InFlightFences.resize(g_MaxFramesInFlight);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < g_MaxFramesInFlight; ++i)
		{
			if (vkCreateSemaphore(VulkanLogicalDevice::Get().GetLogicalDevice(), &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]) != VK_SUCCESS)
				throw std::runtime_error("Failed to create semaphore for 'image available' for a frame!");
			if (vkCreateSemaphore(VulkanLogicalDevice::Get().GetLogicalDevice(), &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]) != VK_SUCCESS)
				throw std::runtime_error("Failed to create semaphore for 'render finished' for a frame!");
			if (vkCreateFence(VulkanLogicalDevice::Get().GetLogicalDevice(), &fenceInfo, nullptr, &m_InFlightFences[i]) != VK_SUCCESS)
				throw std::runtime_error("Failed to create fence for 'image in flight' for a frame!");
		}
	}

	void VulkanSyncObjects::Cleanup() const
	{
		for (size_t i = 0; i < g_MaxFramesInFlight; ++i)
		{
			vkDestroySemaphore(VulkanLogicalDevice::Get().GetLogicalDevice(), m_ImageAvailableSemaphores[i], nullptr);
			vkDestroySemaphore(VulkanLogicalDevice::Get().GetLogicalDevice(), m_RenderFinishedSemaphores[i], nullptr);
			vkDestroyFence(VulkanLogicalDevice::Get().GetLogicalDevice(), m_InFlightFences[i], nullptr);
		}
	}

	VkSemaphore VulkanSyncObjects::GetImageAvailableSemaphoreAt(size_t index) const
	{
		return m_ImageAvailableSemaphores.at(index);
	}

	VkSemaphore VulkanSyncObjects::GetRenderFinishedSemaphoreAt(size_t index) const
	{
		return m_RenderFinishedSemaphores.at(index);
	}

	VkFence VulkanSyncObjects::GetInFlightFenceAt(size_t index) const
	{
		return m_InFlightFences.at(index);
	}
}
