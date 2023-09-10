/*!
\file		VulkanRenderpass.cpp
\date		07/09/2023

\author		Adrian Tan
\email		t.xingkhiangadrian\@digipen.edu

\brief		Contains implementation for VulkanRenderpass class functions.

\copyright	All content © 2023 DigiPen (SINGAPORE) Corporation, all rights reserved.
			Reproduction or disclosure of this file or its contents without the
			prior written consent of DigiPen Institute of Technology is prohibited.
__________________________________________________________________________________*/

#include "meowpch.h"

#include "VulkanContext.h"
#include "VulkanLogicalDevice.h"
#include "VulkanRenderpass.h"
#include "VulkanSwapchain.h"

namespace Nya
{
	void VulkanRenderpass::Init()
	{
		//-- Attachment Description.
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = VulkanSwapchain::Get().GetSwapChainImageFormat();
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

		/// VK_ATTACHMENT_LOAD_OP_LOAD		: Preserve existing contents of the attachment.
		/// VK_ATTACHMENT_LOAD_OP_CLEAR		: Clear the values to a constant (black) at the start.
		/// VK_ATTACHMENT_LOAD_OP_DONT_CARE	: Existing contents are undefined; don't care about them.
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;

		/// VK_ATTACHMENT_STORE_OP_STORE		: Rendered contents will be sstored in memory and can be read later.
		/// VK_ATTACHMENT_STORE_OP_DONT_CARE : Contents of framebuffer will be undefined after render operation.
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		//-- Attachment References.
		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		/// VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL	: Images used as color attachment.
		/// VK_IMAGE_LAYOUT_PRESENT_SRC_KHR			: Images to be presented in the swapchain.
		/// VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL		: Images to be used as destination for a memory copy operation.
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		//-- Subpasses.
		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pInputAttachments = nullptr;		// Optional.
		subpass.pResolveAttachments = nullptr;		// Optional.
		subpass.pDepthStencilAttachment = nullptr;	// Optional.
		subpass.pPreserveAttachments = nullptr;		// Optional.

		//-- Subpass Dependencies.
		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		//-- Render Pass.
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(VulkanLogicalDevice::Get().GetLogicalDevice(), &renderPassInfo, nullptr, &m_RenderPass) != VK_SUCCESS)
			throw std::runtime_error("Failed to create render pass!");
	}

	void VulkanRenderpass::Cleanup() const
	{
		vkDestroyRenderPass(VulkanLogicalDevice::Get().GetLogicalDevice(), m_RenderPass, nullptr);
	}

	VkRenderPass VulkanRenderpass::GetRenderPass() const
	{
		return m_RenderPass;
	}
}
