/*!
\file		VulkanSwapchain.cpp
\date		07/09/2023

\author		Adrian Tan
\email		t.xingkhiangadrian\@digipen.edu

\brief		Contains implementation for VulkanSwapchain class functions.

\copyright	All content © 2023 DigiPen (SINGAPORE) Corporation, all rights reserved.
			Reproduction or disclosure of this file or its contents without the
			prior written consent of DigiPen Institute of Technology is prohibited.
__________________________________________________________________________________*/

#include "meowpch.h"

#include "VulkanSwapchain.h"

#include "VulkanContext.h"
#include "VulkanPhysicalDevice.h"
#include "VulkanLogicalDevice.h"
#include "VulkanQuery.h"

namespace Nya
{
	//-- Singleton.
	std::unique_ptr<VulkanSwapchain> VulkanSwapchain::s_Instance = nullptr;

	VulkanSwapchain& VulkanSwapchain::Get()
	{
		if (!s_Instance)
			s_Instance = std::unique_ptr<VulkanSwapchain>();

		return *s_Instance;
	}


	//-- Helpers
	VkSurfaceFormatKHR ChooseSwapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		for (const auto& availableFormat : availableFormats)
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
				availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				return availableFormat;
		}

		return availableFormats.front();
	}

	VkPresentModeKHR ChooseSwapChainPresentMode(const std::vector<VkPresentModeKHR>& availableModes)
	{
		for (const auto& availablePresentMode : availableModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
				return availablePresentMode;
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D ChooseSwapChainExtents(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
			return capabilities.currentExtent;

		int width;
		int height;
		glfwGetFramebufferSize(VulkanContext::Get().GetWindow(), &width, &height);

		VkExtent2D actualExtents
		{
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};
		actualExtents.width = std::clamp(actualExtents.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtents.height = std::clamp(actualExtents.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtents;
	}


	//-- VulkanSwapChain Functions.
	void VulkanSwapchain::CreateSwapChain()
	{
		const SwapChainSupport details = VulkanQuery::QuerySwapChainSupport(VulkanPhysicalDevice::Get().GetPhysicalDevice(), VulkanContext::Get().GetSurface());

		const VkSurfaceFormatKHR surfaceFormat = ChooseSwapChainSurfaceFormat(details.m_Formats);
		const VkPresentModeKHR presentMode = ChooseSwapChainPresentMode(details.m_PresentModes);
		const VkExtent2D extents = ChooseSwapChainExtents(details.m_Capabilities);

		uint32_t imageCount = details.m_Capabilities.minImageCount + 1;
		if (details.m_Capabilities.maxImageCount > 0 && imageCount > details.m_Capabilities.maxImageCount)
			imageCount = details.m_Capabilities.maxImageCount;

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = VulkanContext::Get().GetSurface();
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extents;
		createInfo.imageArrayLayers = 1;								// Leave as 1 layer per image, unless doing stereoscopic (red & blue) 3D applications.
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;	// VK_IMAGE_USAGE_TRANSFER_DST_BIT if doing post-processing. For now rendering directly.

		const QueueFamilyIndices indices = VulkanQuery::FindQueueFamilies(VulkanPhysicalDevice::Get().GetPhysicalDevice(), VulkanContext::Get().GetSurface());
		const uint32_t queueFamilyIndices[] = { indices.m_GraphicsFamily.value(), indices.m_PresentFamily.value() };

		// Best performance:
		// - VK_SHARING_MODE_EXCLUSIVE <- Image is owned by one queue family at a given time, must be explicitly transferred to another queue family before using.
		// Worse performance:
		// - VK_SHARING_MODE_CONCURRENT  <- Image can be accessed across multiple queues without explicit transfers.

		if (indices.m_GraphicsFamily != indices.m_PresentFamily)
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0;		// Not really required to do.
			createInfo.pQueueFamilyIndices = nullptr;	// Not really required to do.
		}

		createInfo.preTransform = details.m_Capabilities.currentTransform; // Can use to specify a global pre-transform for all images in the swap chain, for like a global 90deg rotation or smth.
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;				// Ignore pixels that are obscured by another window.
		createInfo.oldSwapchain = VK_NULL_HANDLE;	// Should store the old swapchain handle here, if swapchain is being recreated.

		if (vkCreateSwapchainKHR(VulkanLogicalDevice::Get().GetLogicalDevice(), &createInfo, nullptr, &m_SwapChain) != VK_SUCCESS)
			throw std::runtime_error("Failed to create swap chain!");

		vkGetSwapchainImagesKHR(VulkanLogicalDevice::Get().GetLogicalDevice(), m_SwapChain, &imageCount, nullptr);
		m_SwapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(VulkanLogicalDevice::Get().GetLogicalDevice(), m_SwapChain, &imageCount, m_SwapChainImages.data());

		m_SwapChainImageFormat = surfaceFormat.format;
		m_SwapChainImageExtents = extents;
	}

	void VulkanSwapchain::CreateSwapChainImageViews()
	{
		m_SwapChainImageViews.resize(m_SwapChainImages.size());

		for (size_t i = 0; i < m_SwapChainImages.size(); ++i)
		{
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = m_SwapChainImages[i];
			// Define texture type. (1D, 2D or 3D/Cubemap) and image format.
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = m_SwapChainImageFormat;
			// Set default color channel mapping:
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			// Define image's ppurpose and which parts of image to be accessed.
			// - For now will be color target without any mipmapping or multiple layers:
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(VulkanLogicalDevice::Get().GetLogicalDevice(), &createInfo, nullptr, &m_SwapChainImageViews[i]) != VK_SUCCESS)
				throw std::runtime_error("Failed to create image views at index " + i);
		}
	}

	void VulkanSwapchain::Init()
	{
		CreateSwapChain();
		CreateSwapChainImageViews();
	}

	void VulkanSwapchain::Cleanup() const
	{
		// Cleanup swap-chain image views.
		for (const auto imageView : m_SwapChainImageViews)
			vkDestroyImageView(VulkanLogicalDevice::Get().GetLogicalDevice(), imageView, nullptr);

		// Cleanup swap-chain.
		vkDestroySwapchainKHR(VulkanLogicalDevice::Get().GetLogicalDevice(), m_SwapChain, nullptr);
	}

	VkSwapchainKHR VulkanSwapchain::GetSwapChain() const
	{
		return m_SwapChain;
	}

	VkFormat VulkanSwapchain::GetSwapChainImageFormat() const
	{
		return m_SwapChainImageFormat;
	}

	VkExtent2D VulkanSwapchain::GetSwapChainImageExtents() const
	{
		return m_SwapChainImageExtents;
	}

	std::vector<VkImage> VulkanSwapchain::GetSwapChainImages() const
	{
		return m_SwapChainImages;
	}

	std::vector<VkImageView> VulkanSwapchain::GetSwapChainImageViews() const
	{
		return m_SwapChainImageViews;
	}
}
