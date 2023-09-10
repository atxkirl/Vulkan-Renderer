/*!
\file		VulkanFramebuffer.cpp
\date		07/09/2023

\author		Adrian Tan
\email		t.xingkhiangadrian\@digipen.edu

\brief		Contains implementation for VulkanFramebuffer class functions.

\copyright	All content © 2023 DigiPen (SINGAPORE) Corporation, all rights reserved.
			Reproduction or disclosure of this file or its contents without the
			prior written consent of DigiPen Institute of Technology is prohibited.
__________________________________________________________________________________*/

#include "meowpch.h"

#include "VulkanFramebuffer.h"
#include "VulkanLogicalDevice.h"
#include "VulkanSwapchain.h"

namespace Nya
{
	void VulkanFramebuffer::Init(size_t index, VkImageView attachments[], VkRenderPass renderPass)
	{
		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = VulkanSwapchain::Get().GetSwapChainImageExtents().width;
		framebufferInfo.height = VulkanSwapchain::Get().GetSwapChainImageExtents().height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(VulkanLogicalDevice::Get().GetLogicalDevice(), &framebufferInfo, nullptr, &m_Framebuffer) != VK_SUCCESS)
			throw std::runtime_error("Failed to create framebuffer at index [" + std::to_string(index) + "]!");
	}

	void VulkanFramebuffer::Cleanup() const
	{
		vkDestroyFramebuffer(VulkanLogicalDevice::Get().GetLogicalDevice(), m_Framebuffer, nullptr);
	}

	VkFramebuffer VulkanFramebuffer::GetFramebuffer() const
	{
		return m_Framebuffer;
	}
}
