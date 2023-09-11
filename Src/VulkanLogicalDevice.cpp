/*!
\file		VulkanLogicalDevice.cpp
\date		09/09/2023

\author		Adrian Tan
\email		t.xingkhiangadrian\@digipen.edu

\brief		Contains implementation for VulkanLogicalDevice class functions.

\copyright	All content © 2023 DigiPen (SINGAPORE) Corporation, all rights reserved.
			Reproduction or disclosure of this file or its contents without the
			prior written consent of DigiPen Institute of Technology is prohibited.
__________________________________________________________________________________*/

#include "meowpch.h"

#include "VulkanLogicalDevice.h"

#include "VulkanPhysicalDevice.h"
#include "VulkanContext.h"
#include "VulkanQuery.h"

namespace Nya
{
	//-- Singleton.
	//std::unique_ptr<VulkanLogicalDevice> VulkanLogicalDevice::s_Instance = nullptr;
	VulkanLogicalDevice* VulkanLogicalDevice::s_Instance = nullptr;

	VulkanLogicalDevice& VulkanLogicalDevice::Get()
	{
		if (!s_Instance)
		{
			//s_Instance = std::make_unique<VulkanLogicalDevice>();
			s_Instance = new VulkanLogicalDevice();
		}

		return *s_Instance;
	}


	//-- VulkanLogicalDevice Functions.
	void VulkanLogicalDevice::Init()
	{
		QueueFamilyIndices indices = VulkanQuery::FindQueueFamilies(VulkanPhysicalDevice::Get().GetPhysicalDevice(), VulkanContext::Get().GetSurface());

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies =
		{
			indices.m_GraphicsFamily.value(),
			indices.m_PresentFamily.value()
		};

		float queuePriority = 1.f;
		for (uint32_t queueFamily : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;

			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures{};

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(g_DeviceExtensions.size());
		createInfo.ppEnabledExtensionNames = g_DeviceExtensions.data();
		createInfo.pNext = VK_NULL_HANDLE;

		if (g_EnableValidationLayers)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(g_ValidationLayers.size());
			createInfo.ppEnabledLayerNames = g_ValidationLayers.data();
		}
		else
		{
			createInfo.enabledLayerCount = 0;
		}

		// Create logical device.
		if (vkCreateDevice(VulkanPhysicalDevice::Get().GetPhysicalDevice(), &createInfo, nullptr, &m_LogicalDevice) != VK_SUCCESS)
			throw std::runtime_error("Failed to create logical device!");

		// Retrieve handles to device queues:
		vkGetDeviceQueue(m_LogicalDevice, indices.m_GraphicsFamily.value(), 0, &m_GraphicsQueue);
		vkGetDeviceQueue(m_LogicalDevice, indices.m_PresentFamily.value(), 0, &m_PresentQueue);
	}

	void VulkanLogicalDevice::Cleanup() const
	{
		vkDestroyDevice(m_LogicalDevice, nullptr);

		//MEOW_LOG_INFO("VulkanLogicalDevice cleaned up!");
	}

	VkDevice VulkanLogicalDevice::GetLogicalDevice() const
	{
		return m_LogicalDevice;
	}

	VkQueue VulkanLogicalDevice::GetGraphicsQueue() const
	{
		return m_GraphicsQueue;
	}

	VkQueue VulkanLogicalDevice::GetPresentQueue() const
	{
		return m_PresentQueue;
	}
}
