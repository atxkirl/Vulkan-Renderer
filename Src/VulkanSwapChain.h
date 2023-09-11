/*!
\file		VulkanSwapchain.h
\date		07/09/2023

\author		Adrian Tan
\email		t.xingkhiangadrian\@digipen.edu

\brief		Contains definition of VulkanSwapchain class.

\copyright	All content © 2023 DigiPen (SINGAPORE) Corporation, all rights reserved.
			Reproduction or disclosure of this file or its contents without the
			prior written consent of DigiPen Institute of Technology is prohibited.
__________________________________________________________________________________*/
#pragma once

#include <vulkan/vulkan.h>
#include "VulkanContext.h"

namespace Nya
{
	class VulkanSwapchain
	{
		//static std::unique_ptr<VulkanSwapchain> s_Instance;
		static VulkanSwapchain* s_Instance;

		VkSwapchainKHR m_SwapChain{};
		std::vector<VkImage> m_SwapChainImages;
		std::vector<VkImageView> m_SwapChainImageViews;
		std::vector<VkFramebuffer> m_SwapChainFramebuffers;

		VkFormat m_SwapChainImageFormat{};
		VkExtent2D m_SwapChainImageExtents{};

		void CreateSwapChain();
		void CreateSwapChainImageViews();

	public:
		static VulkanSwapchain& Get();
		void Init();
		void CreateSwapChainFramebuffers(VkRenderPass renderPass);
		void Cleanup() const;
		void Recreate(VkRenderPass renderPass);

		VkSwapchainKHR GetSwapChain() const;
		VkFormat GetSwapChainImageFormat() const;
		VkExtent2D GetSwapChainImageExtents() const;

		std::vector<VkImage> GetSwapChainImages() const;
		std::vector<VkImageView> GetSwapChainImageViews() const;
		std::vector<VkFramebuffer> GetSwapChainFramebuffers() const;
	};
}
