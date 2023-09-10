/*!
\file		VulkanPhysicalDevice.cpp
\date		09/09/2023

\author		Adrian Tan
\email		t.xingkhiangadrian\@digipen.edu

\brief		Contains implementation for VulkanPhysicalDevice class functions.

\copyright	All content © 2023 DigiPen (SINGAPORE) Corporation, all rights reserved.
			Reproduction or disclosure of this file or its contents without the
			prior written consent of DigiPen Institute of Technology is prohibited.
__________________________________________________________________________________*/

#include "meowpch.h"

#include "VulkanPhysicalDevice.h"
#include "VulkanContext.h"

namespace Nya
{
	//-- Singleton.
	std::unique_ptr<VulkanPhysicalDevice> VulkanPhysicalDevice::s_Instance = nullptr;

	VulkanPhysicalDevice& VulkanPhysicalDevice::Get()
	{
		if (!s_Instance)
			s_Instance = std::make_unique<VulkanPhysicalDevice>();

		return *s_Instance;
	}


	//-- Helpers.
	int RateDevice(VkPhysicalDevice device)
	{
		int score = 0;
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		// GPU *must* support geometry shaders!
		if (!deviceFeatures.geometryShader)
			return -1;

		// Discrete GPUs first and foremost.
		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			score += 1000;
		// Larger maximum texture size is better.
		score += static_cast<int>(deviceProperties.limits.maxImageDimension2D);

		return score;
	}


	//-- VulkanPhysicalDevice Functions.
	VkPhysicalDevice VulkanPhysicalDevice::GetPhysicalDevice() const
	{
		return m_PhysicalDevice;
	}

	void VulkanPhysicalDevice::Init()
	{
		// Retrieve all attached GPUs.
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(VulkanContext::Get().GetInstance(), &deviceCount, nullptr);
		if (deviceCount == 0)
			throw std::runtime_error("Failed to find any GPUs with Vulkan support!");

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(VulkanContext::Get().GetInstance(), &deviceCount, devices.data());

		// Rate and Sort GPUs by ability.
		std::multimap<int, VkPhysicalDevice> deviceCandidates;
		for (const auto& device : devices)
		{
			int score = RateDevice(device);
			deviceCandidates.insert({ score, device });
		}

		// Pick best GPU.
		if (deviceCandidates.rbegin()->first > 0)
		{
			m_PhysicalDevice = deviceCandidates.rbegin()->second;

#ifdef MEOW_DEBUG
			VkPhysicalDeviceProperties deviceProperties;
			vkGetPhysicalDeviceProperties(m_PhysicalDevice, &deviceProperties);

			MEOW_LOG_INFO("Selected GPU:");
			MEOW_LOG_INFO("GPU Name: ", deviceProperties.deviceName);
			MEOW_LOG_INFO("GPU Vendor: ", deviceProperties.vendorID);
#endif

		}
		else
		{
			throw std::runtime_error("Failed to find suitable GPU!");
		}
	}
}
