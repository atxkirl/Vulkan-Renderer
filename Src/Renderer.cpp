#include "Renderer.h"
#include <iostream>
#include <algorithm>

const uint32_t WIN_WIDTH = 800;
const uint32_t WIN_HEIGHT = 600;

#ifdef _DEBUG
const bool EnableValidationLayers = true;
#else
const bool EnableValidationLayers = false;
#endif

const std::vector<const char*> ValidationLayers =
{
	"VK_LAYER_KHRONOS_validation"
};
const std::vector<const char*> DeviceExtensions =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

//-- Private Functions
void Renderer::InitGLFW()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	m_Window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, "Vulkan Renderer", nullptr, nullptr);
}

void Renderer::InitVulkan()
{
	CreateVulkanInstance();
	CreateSurface();
	SelectPhysicalGPU();
	CreateLogicalDevice();
	CreateSwapChain();
}

void Renderer::CreateVulkanInstance()
{
	CheckExtensionSupport();
	if (EnableValidationLayers && !CheckValidationLayerSupport())
	{
		throw std::runtime_error("Vulkan validation layers requested, but not available!");
	}

	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Vulkan Renderer";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "MEOW";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledLayerCount = 0;
	if (EnableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
		createInfo.ppEnabledLayerNames = ValidationLayers.data();
	}
	else
	{
		createInfo.enabledLayerCount = 0;
		createInfo.ppEnabledLayerNames = nullptr;
	}

	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	createInfo.enabledExtensionCount = glfwExtensionCount;
	createInfo.ppEnabledExtensionNames = glfwExtensions;
	createInfo.enabledLayerCount = 0;

	if (vkCreateInstance(&createInfo, nullptr, &m_Instance) != VK_SUCCESS)
		throw std::runtime_error("Failed to create Vulkan instance!");
}

void Renderer::CheckExtensionSupport()
{
	// Print out valid extensions for the selected GPU.
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	
	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
	
	std::cout << "Available Vulkan extensions: \n";
	for (const auto& extension : extensions)
	{
		std::cout << "\t" << extension.extensionName << std::endl;
	}
}

bool Renderer::CheckValidationLayerSupport()
{
	uint32_t layerCount = 0;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	std::cout << "Available Vulkan validation layers: \n";
	for (const char* layerName : ValidationLayers)
	{
		bool found = false;
		for (const auto& layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				found = true;

				std::cout << "\t" << layerProperties.layerName << std::endl;
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

bool Renderer::CheckDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(DeviceExtensions.begin(), DeviceExtensions.end());
	for (const auto& extension : availableExtensions)
	{
		requiredExtensions.erase(extension.extensionName);
	}

	if (!requiredExtensions.empty())
	{
		std::cout << "\t" << "Device is missing support for these extensions:\n";
		for (const auto& name : requiredExtensions)
		{
			std::cout << "\t" << name << std::endl;
		}
		return false;
	}

	return true;
}

void Renderer::SelectPhysicalGPU()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);
	if (deviceCount == 0)
		throw std::runtime_error("Failed to find any GPUs with Vulkan support!");

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());

	std::multimap<int, VkPhysicalDevice> deviceCandidates;
	for (const auto& device : devices)
	{
		int score = RateDevice(device);
		deviceCandidates.insert({ score, device });
	}
	if (deviceCandidates.rbegin()->first > 0)
		m_PhysicalDevice = deviceCandidates.rbegin()->second;
	else
		throw std::runtime_error("Failed to find suitable GPU!");
}

void Renderer::CreateLogicalDevice()
{
	QueueFamilyIndices indices = FindQueueFamilies(m_PhysicalDevice);

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
	createInfo.enabledExtensionCount = static_cast<uint32_t>(DeviceExtensions.size());
	createInfo.ppEnabledExtensionNames = DeviceExtensions.data();

	if (EnableValidationLayers)
		createInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
	else
		createInfo.enabledLayerCount = 0;

	// Create logical device.
	if (vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_LogicalDevice) != VK_SUCCESS)
		throw std::runtime_error("Failed to create logical device!");
	// Retrieve handles to device queues:
	vkGetDeviceQueue(m_LogicalDevice, indices.m_GraphicsFamily.value(), 0, &m_GraphicsQueue);
	vkGetDeviceQueue(m_LogicalDevice, indices.m_PresentFamily.value(), 0, &m_PresentQueue);
}

bool Renderer::IsDeviceSuitable(VkPhysicalDevice device)
{
	QueueFamilyIndices indices = FindQueueFamilies(device);
	bool extensionsSupported = CheckDeviceExtensionSupport(device);
	bool swapChainAdequate = false;
	if (extensionsSupported)
	{
		SwapChainSupportDetails details = QuerySwapChainSupport(device);
		swapChainAdequate = !details.m_Formats.empty() && !details.m_PresentModes.empty();
	}

	return 
		indices.IsComplete() &&
		extensionsSupported &&
		swapChainAdequate;
}

void Renderer::CreateSurface()
{
	if (glfwCreateWindowSurface(m_Instance, m_Window, nullptr, &m_Surface) != VK_SUCCESS)
		throw std::runtime_error("Failed to create window surface with GLFW!");
}

void Renderer::CreateSwapChain()
{
	SwapChainSupportDetails details = QuerySwapChainSupport(m_PhysicalDevice);

	VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(details.m_Formats);
	VkPresentModeKHR presentMode = ChooseSwapPresentMode(details.m_PresentModes);
	VkExtent2D extents = ChooseSwapExtent(details.m_Capabilities);

	uint32_t imageCount = details.m_Capabilities.minImageCount + 1;
	if (details.m_Capabilities.maxImageCount > 0 && imageCount > details.m_Capabilities.maxImageCount)
		imageCount = details.m_Capabilities.maxImageCount;

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = m_Surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extents;
	createInfo.imageArrayLayers = 1;								// Leave as 1 layer per image, unless doing stereoscopic (red & blue) 3D applications.
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;	// VK_IMAGE_USAGE_TRANSFER_DST_BIT if doing post-processing. For now rendering directly.

	QueueFamilyIndices indices = FindQueueFamilies(m_PhysicalDevice);
	uint32_t queueFamilyIndices[] = { indices.m_GraphicsFamily.value(), indices.m_PresentFamily.value() };

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
	createInfo.clipped = VK_TRUE; // Ignore pixels that are obscured by another window.
	createInfo.oldSwapchain = VK_NULL_HANDLE; ///!!IMPORTANT!! Need to specify reference to old swap chain, if recreating a new one due to window resize.

	if (vkCreateSwapchainKHR(m_LogicalDevice, &createInfo, nullptr, &m_SwapChain) != VK_SUCCESS)
		throw std::runtime_error("Failed to create swap chain!");

	vkGetSwapchainImagesKHR(m_LogicalDevice, m_SwapChain, &imageCount, nullptr);
	m_SwapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(m_LogicalDevice, m_SwapChain, &imageCount, m_SwapChainImages.data());

	m_SwapChainImageFormat = surfaceFormat.format;
	m_SwapChainExtents = extents;
}

int Renderer::RateDevice(VkPhysicalDevice device)
{
	int score = 0;
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	// GPU *must* support geometry shaders!
	if (!deviceFeatures.geometryShader)
		return 0;

	// Discrete GPUs first and foremost.
	if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		score += 1000;
	// Larger maximum texture size is better.
	score += deviceProperties.limits.maxImageDimension2D;

	return score;
}

VkSurfaceFormatKHR Renderer::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const auto& availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
			availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			return availableFormat;
	}

	return availableFormats.front();
}

VkPresentModeKHR Renderer::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	for (const auto& availablePresentMode : availablePresentModes)
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			return availablePresentMode;
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Renderer::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		return capabilities.currentExtent;

	int width;
	int height;
	glfwGetFramebufferSize(m_Window, &width, &height);

	VkExtent2D actualExtents
	{
		static_cast<uint32_t>(width),
		static_cast<uint32_t>(height)
	};
	actualExtents.width = std::clamp(actualExtents.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
	actualExtents.height = std::clamp(actualExtents.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

	return actualExtents;
}

Renderer::QueueFamilyIndices Renderer::FindQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		// Check for graphics families.
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			indices.m_GraphicsFamily = i;
		// Check for present families.
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_Surface, &presentSupport);
		if (presentSupport)
			indices.m_PresentFamily = i;

		// Note: Can potentially add some code in RateDevice() to
		//		 prefer devices that have the same queue family
		//		 for both m_GraphicsFamily and m_PresentFamily, to
		//		 improve GPU performance.

		if (indices.IsComplete())
			break;
		++i;
	}

	return indices;
}

Renderer::SwapChainSupportDetails Renderer::QuerySwapChainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_Surface, &details.m_Capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, nullptr);
	if (formatCount != 0)
	{
		details.m_Formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, details.m_Formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, nullptr);
	if (presentModeCount != 0)
	{
		details.m_PresentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, details.m_PresentModes.data());
	}

	return details;
}


//-- Public Functions
void Renderer::Initialize()
{
	std::cout << "Initializing Renderer!\n";

	InitGLFW();
	InitVulkan();
}

void Renderer::Update()
{
	std::cout << "Starting Renderer Update loop!\n";

	while (!glfwWindowShouldClose(m_Window))
	{
		glfwPollEvents();
	}
}

void Renderer::Shutdown()
{
	std::cout << "Shutting down Renderer!\n";

	vkDestroySwapchainKHR(m_LogicalDevice, m_SwapChain, nullptr);
	vkDestroyDevice(m_LogicalDevice, nullptr);

	vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
	vkDestroyInstance(m_Instance, nullptr);

	glfwDestroyWindow(m_Window);
	glfwTerminate();
}
