/*!
\file		VulkanContext.cpp
\date		08/09/2023

\author		Adrian Tan
\email		t.xingkhiangadrian\@digipen.edu

\brief		Contains implementation for VulkanContext class functions.

\copyright	All content © 2023 DigiPen (SINGAPORE) Corporation, all rights reserved.
			Reproduction or disclosure of this file or its contents without the
			prior written consent of DigiPen Institute of Technology is prohibited.
__________________________________________________________________________________*/

#include "meowpch.h"

#include "VulkanContext.h"
#include "VulkanDebugger.h"

namespace Nya
{
	//-- Singleton.
	std::unique_ptr<VulkanContext> VulkanContext::s_Instance = nullptr;

	VulkanContext& VulkanContext::Get()
	{
		if (!s_Instance)
			s_Instance = std::make_unique<VulkanContext>();

		return *s_Instance;
	}

	//-- VulkanContext Functions.
	void VulkanContext::CreateInstance()
	{
		// Debugging.
		VulkanDebugger::PrintExtensionSupport();
		if (g_EnableValidationLayers && !CheckValidationLayerSupport())
			throw std::runtime_error("Vulkan validation layers requested, but not available!");

		// Application info.
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Vulkan Renderer";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "MEOW";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		// Create info for vulkan instance.
		GetRequiredExtensions();
		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(m_Extensions.size());
		createInfo.ppEnabledExtensionNames = m_Extensions.data();

		// Link debug layers.
		VkDebugUtilsMessengerCreateInfoEXT debugInfo{};
		if (g_EnableValidationLayers)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(g_ValidationLayers.size());
			createInfo.ppEnabledLayerNames = g_ValidationLayers.data();
			VulkanDebugger::PopulateDebugMessengerCreateInfo(debugInfo);
			createInfo.pNext = &debugInfo;
		}
		else
		{
			createInfo.enabledLayerCount = 0;
			createInfo.pNext = VK_NULL_HANDLE;
		}

		// Create vulkan instance.
		if (vkCreateInstance(&createInfo, nullptr, &m_Instance) != VK_SUCCESS)
			throw std::runtime_error("Failed to create Vulkan instance!");
	}

	void VulkanContext::CreateSurface()
	{
		if (glfwCreateWindowSurface(m_Instance, m_Window, nullptr, &m_Surface) != VK_SUCCESS)
			throw std::runtime_error("Failed to create window surface with GLFW!");
	}

	bool VulkanContext::CheckValidationLayerSupport() const
	{
		uint32_t layerCount = 0;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		std::cout << "\t" << "Available Vulkan validation layers: \n";
		for (const char* layerName : g_ValidationLayers)
		{
			bool found = false;
			for (const auto& layerProperties : availableLayers)
			{
				if (strcmp(layerName, layerProperties.layerName) == 0)
				{
					found = true;

					std::cout << "\t\t" << layerProperties.layerName << std::endl;
					break;
				}
			}

			if (!found)
			{
				return false;
			}
		}

		return true;
	}

	void VulkanContext::GetRequiredExtensions()
	{
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		if (glfwExtensions == nullptr)
			throw std::runtime_error("Required Vulkan extensions not available on this machine!");

		m_Extensions.reserve(glfwExtensionCount);
		m_Extensions.assign(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (g_EnableValidationLayers)
			m_Extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	void VulkanContext::Init(GLFWwindow* window)
	{
		m_Window = window;

		CreateInstance();
		CreateSurface();
	}

	void VulkanContext::Cleanup() const
	{
		vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
		vkDestroyInstance(m_Instance, nullptr);

		//MEOW_LOG_INFO("VulkanContext cleaned up!");
	}

	GLFWwindow* VulkanContext::GetWindow() const
	{
		return m_Window;
	}

	VkInstance VulkanContext::GetInstance() const
	{
		return m_Instance;
	}

	VkSurfaceKHR VulkanContext::GetSurface() const
	{
		return m_Surface;
	}
}
