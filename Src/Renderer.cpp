#include "Renderer.h"
#include <iostream>
#include <algorithm>

#include "FileLoader.h"


const uint32_t WIN_WIDTH = 800;		// Window width.
const uint32_t WIN_HEIGHT = 600;	// Window height.
const int MAX_FRAMES_IN_FLIGHT = 2;	// Max number of frames that should be processed concurrently. (AKA max number of pre-rendered frames.)

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

// No more hardcoding vertices in shader!
// Describes a triangle, same as before.
const std::vector<Vertex> Vertices =
{
	{{0.0f, -0.5f}, {1.f, 0.0f, 0.0f}},
	{{0.5f, 0.5f}, {0.f, 1.0f, 0.0f}},
	{{-0.5f, 0.5f}, {0.f, 0.0f, 1.0f}}
};


///- Static Functions
static void FrameBufferResizedCallbackFn(GLFWwindow* window, int width, int height)
{
	auto app = reinterpret_cast<Renderer*>(glfwGetWindowUserPointer(window));
	app->FlagFrameBufferResized();
}

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallbackFn(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	std::cerr << "Level " << messageSeverity << " -> Validation layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}


///- Public Functions
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
		DrawFrame();
	}

	vkDeviceWaitIdle(m_LogicalDevice);
}

void Renderer::Shutdown()
{
	std::cout << "Shutting down Renderer!\n";

	if (EnableValidationLayers)
		DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);

	// Cleanup frame semaphores and fences.
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		vkDestroySemaphore(m_LogicalDevice, m_ImageAvailableSemaphores[i], nullptr);
		vkDestroySemaphore(m_LogicalDevice, m_RenderFinishedSemaphores[i], nullptr);
		vkDestroyFence(m_LogicalDevice, m_InFlightFences[i], nullptr);
	}

	// Cleanup command pool.
	vkDestroyCommandPool(m_LogicalDevice, m_CommandPool, nullptr);

	// Cleanup render pipelines.
	vkDestroyPipeline(m_LogicalDevice, m_GraphicsPipeline, nullptr);
	vkDestroyPipelineLayout(m_LogicalDevice, m_PipelineLayout, nullptr);
	vkDestroyRenderPass(m_LogicalDevice, m_RenderPass, nullptr);

	// Cleanup swap-chains.
	DestroySwapChain();

	// Cleanup buffers.
	vkDestroyBuffer(m_LogicalDevice, m_VertexBuffer, nullptr);
	vkDestroyBuffer(m_LogicalDevice, m_IndiceBuffer, nullptr);
	vkFreeMemory(m_LogicalDevice, m_VertexBufferMemory, nullptr);

	// Cleanup vulkan logical device.
	vkDestroyDevice(m_LogicalDevice, nullptr);

	// Cleanup vulkan surface.
	vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
	// Cleanup vulkan instance.
	vkDestroyInstance(m_Instance, nullptr);

	// Cleanup GLFW.
	glfwDestroyWindow(m_Window);
	glfwTerminate();
}

void Renderer::FlagFrameBufferResized()
{
#ifdef _DEBUG
	std::cout << "Frame buffer resized!" << std::endl;
#endif

	m_FrameBufferResized = true;
}


///- Private Functions
//-- Main API Initializations.
void Renderer::InitGLFW()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	m_Window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, "Vulkan Renderer", nullptr, nullptr);

	glfwSetWindowUserPointer(m_Window, this);
	glfwSetFramebufferSizeCallback(m_Window, FrameBufferResizedCallbackFn);
}

void Renderer::InitVulkan()
{
	CreateVulkanInstance();
	SetupDebugMessenger();
	CreateSurface();

	SelectPhysicalGPU();
	CreateLogicalDevice();
	CreateSwapChain();
	CreateImageViews();

	CreateRenderPass();
	CreateGraphicsPipeline();

	CreateFramebuffers();
	CreateCommandPool();
	CreateVertexBuffer();
	CreateCommandBuffers();
	CreateSyncObjects();
}

//-- Debug.
VkResult Renderer::CreateDebugUtilMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr)
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	else
		return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void Renderer::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMsger, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr)
		func(instance, debugMsger, pAllocator);
}

void Renderer::SetupDebugMessenger()
{
	if (!EnableValidationLayers)
		return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo{};
	PopulateDebugMessengerCreateInfo(createInfo);

	if (CreateDebugUtilMessengerEXT(m_Instance, &createInfo, nullptr, &m_DebugMessenger) != VK_SUCCESS)
		throw std::runtime_error("Failed to set up debug messenger!");
}

void Renderer::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = DebugCallbackFn;
}

//-- Extension Checks.
void Renderer::CheckExtensionSupport()
{
	// Print out valid extensions for the selected GPU.
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

	std::cout << "\t" << "Available Vulkan extensions: \n";
	for (const auto& extension : extensions)
	{
		std::cout << "\t\t" << extension.extensionName << std::endl;
	}
}

bool Renderer::CheckValidationLayerSupport()
{
	uint32_t layerCount = 0;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	std::cout << "\t" << "Available Vulkan validation layers: \n";
	for (const char* layerName : ValidationLayers)
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

std::vector<const char*> Renderer::GetRequiredExtensions()
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (EnableValidationLayers)
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	return extensions;
}

//-- GPU Selection Helpers.
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

int Renderer::RateDevice(VkPhysicalDevice device)
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
	score += deviceProperties.limits.maxImageDimension2D;

	return score;
}

//-- Vulkan Initialization.
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

	auto extensions = GetRequiredExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	VkDebugUtilsMessengerCreateInfoEXT debugInfo{};
	if (EnableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
		createInfo.ppEnabledLayerNames = ValidationLayers.data();
		PopulateDebugMessengerCreateInfo(debugInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugInfo;
	}
	else
	{
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = VK_NULL_HANDLE;
	}

	if (vkCreateInstance(&createInfo, nullptr, &m_Instance) != VK_SUCCESS)
		throw std::runtime_error("Failed to create Vulkan instance!");
}

void Renderer::SelectPhysicalGPU()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);
	if (deviceCount == 0)
		throw std::runtime_error("Failed to find any GPUs with Vulkan support!");

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());

	// Rate and sort GPUs.
	std::multimap<int, VkPhysicalDevice> deviceCandidates;
	for (const auto& device : devices)
	{
		int score = RateDevice(device);
		deviceCandidates.insert({ score, device });
	}

	// Pick top GPU.
	if (deviceCandidates.rbegin()->first > 0)
	{
		m_PhysicalDevice = deviceCandidates.rbegin()->second;

#ifdef _DEBUG
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(m_PhysicalDevice, &deviceProperties);

		std::cout << "\t" << "Selected GPU:\n";
		std::cout << "\t\t" << "GPU Name: " << deviceProperties.deviceName << std::endl;
		std::cout << "\t\t" << "GPU Vendor ID: " << deviceProperties.vendorID << std::endl;
		std::cout << "\t\t" << "GPU Device ID: " << deviceProperties.deviceID << std::endl;
#endif
	}
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
	createInfo.pNext = VK_NULL_HANDLE;

	if (EnableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
		createInfo.ppEnabledLayerNames = ValidationLayers.data();
	}
	else
	{
		createInfo.enabledLayerCount = 0;
	}

	// Create logical device.
	if (vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_LogicalDevice) != VK_SUCCESS)
		throw std::runtime_error("Failed to create logical device!");

	// Retrieve handles to device queues:
	vkGetDeviceQueue(m_LogicalDevice, indices.m_GraphicsFamily.value(), 0, &m_GraphicsQueue);
	vkGetDeviceQueue(m_LogicalDevice, indices.m_PresentFamily.value(), 0, &m_PresentQueue);
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
	createInfo.clipped = VK_TRUE;				// Ignore pixels that are obscured by another window.
	createInfo.oldSwapchain = VK_NULL_HANDLE;	// Should store the old swapchain handle here, if swapchain is being recreated.

	if (vkCreateSwapchainKHR(m_LogicalDevice, &createInfo, nullptr, &m_SwapChain) != VK_SUCCESS)
		throw std::runtime_error("Failed to create swap chain!");

	vkGetSwapchainImagesKHR(m_LogicalDevice, m_SwapChain, &imageCount, nullptr);
	m_SwapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(m_LogicalDevice, m_SwapChain, &imageCount, m_SwapChainImages.data());

	m_SwapChainImageFormat = surfaceFormat.format;
	m_SwapChainExtents = extents;
}

void Renderer::CreateImageViews()
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

		if (vkCreateImageView(m_LogicalDevice, &createInfo, nullptr, &m_SwapChainImageViews[i]) != VK_SUCCESS)
			throw std::runtime_error("Failed to create image views at index " + i);
	}
}

void Renderer::CreateRenderPass()
{
	//-- Attachment Description.
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = m_SwapChainImageFormat;
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

	if (vkCreateRenderPass(m_LogicalDevice, &renderPassInfo, nullptr, &m_RenderPass) != VK_SUCCESS)
		throw std::runtime_error("Failed to create render pass!");
}

void Renderer::CreateGraphicsPipeline()
{
	//-- Shaders.
	auto vertShaderCode = FileLoader::ReadFile("Shaders/output/vert.spv");
	auto fragShaderCode = FileLoader::ReadFile("Shaders/output/frag.spv");

#ifdef _DEBUG
	std::cout << "\t" << "Vert shader byte size: " << vertShaderCode.size() << std::endl;
	std::cout << "\t" << "Frag shader byte size: " << fragShaderCode.size() << std::endl;
#endif

	// Create shader modules.
	VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode);
	VkShaderModule fragShaderModule = CreateShaderModule(fragShaderCode);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main"; // Name of the entrypoint function in the shader.

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main"; // Name of the entrypoint function in the shader.

	// Create array of shader stages.
	VkPipelineShaderStageCreateInfo shaderStages[] =
	{
		vertShaderStageInfo,
		fragShaderStageInfo
	};

	//-- Vertex input.
	auto bindingDescription = Vertex::GetBindingDescription();
	auto attributeDescription = Vertex::GetAttributeDescription();
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescription.size());
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescription.data();

	//-- Input assembly.
	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	//-- Viewport and Scissors.
	VkViewport viewport;
	viewport.x = 0.f;
	viewport.y = 0.f;
	viewport.width = static_cast<float>(m_SwapChainExtents.width);
	viewport.height = static_cast<float>(m_SwapChainExtents.height);
	viewport.minDepth = 0.f;
	viewport.maxDepth = 1.f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = m_SwapChainExtents;

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	//viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	//viewportState.pScissors = &scissor;

	//-- Rasterizer.
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;	// VK_POLYGON_MODE_LINE, VK_POLYGON_MODE_POINT
	rasterizer.lineWidth = 1.f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;	// Cull only BACK facing, FRONT facing, or BOTH
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE; // Orientation of front facing triangles.
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.f;		// Optional.
	rasterizer.depthBiasClamp = 0.f;				// Optional.
	rasterizer.depthBiasSlopeFactor = 0.f;			// Optional.

	//-- Multisampling.
	/// Disabled for now, will enable later down the road.
	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.f;			// Optional.
	multisampling.pSampleMask = nullptr;			// Optional.
	multisampling.alphaToCoverageEnable = VK_FALSE;	// Optional.
	multisampling.alphaToOneEnable = VK_FALSE;		// Optional.

	//-- Depth and Stencil testing.
	/// Disabled for now, so just passing nullptr when creating pipeline.

	//-- Color blending.
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.f;
	colorBlending.blendConstants[1] = 0.f;
	colorBlending.blendConstants[2] = 0.f;
	colorBlending.blendConstants[3] = 0.f;

	//-- Fixed Functions.
	std::vector<VkDynamicState> dynamicStates =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};
	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();

	//-- Create pipeline layout.
	VkPipelineLayoutCreateInfo pipelineLayout{};
	pipelineLayout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayout.setLayoutCount = 0;				// Optional.
	pipelineLayout.pSetLayouts = nullptr;			// Optional.
	pipelineLayout.pushConstantRangeCount = 0;		// Optional.
	pipelineLayout.pPushConstantRanges = nullptr;	// Optional.

	if (vkCreatePipelineLayout(m_LogicalDevice, &pipelineLayout, nullptr, &m_PipelineLayout) != VK_SUCCESS)
		throw std::runtime_error("Failed to create pipeline layout!");

	//-- Create pipeline.
	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = m_PipelineLayout;
	pipelineInfo.renderPass = m_RenderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	if (vkCreateGraphicsPipelines(m_LogicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_GraphicsPipeline) != VK_SUCCESS)
		throw std::runtime_error("Failed to create graphics pipeline!");

	//-- Cleanup.
	vkDestroyShaderModule(m_LogicalDevice, vertShaderModule, nullptr);
	vkDestroyShaderModule(m_LogicalDevice, fragShaderModule, nullptr);
}

void Renderer::CreateFramebuffers()
{
	m_SwapChainFramebuffers.resize(m_SwapChainImageViews.size());

	for (size_t i = 0; i < m_SwapChainImageViews.size(); ++i)
	{
		VkImageView attachments[] =
		{
			m_SwapChainImageViews[i]
		};

		VkFramebufferCreateInfo frameBufferInfo{};
		frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frameBufferInfo.renderPass = m_RenderPass;
		frameBufferInfo.attachmentCount = 1;
		frameBufferInfo.pAttachments = attachments;
		frameBufferInfo.width = m_SwapChainExtents.width;
		frameBufferInfo.height = m_SwapChainExtents.height;
		frameBufferInfo.layers = 1;

		if (vkCreateFramebuffer(m_LogicalDevice, &frameBufferInfo, nullptr, &m_SwapChainFramebuffers[i]) != VK_SUCCESS)
			throw std::runtime_error("Failed to create framebuffer at index " + i);
	}
}

void Renderer::CreateCommandPool()
{
	QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(m_PhysicalDevice);

	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = queueFamilyIndices.m_GraphicsFamily.value();

	if (vkCreateCommandPool(m_LogicalDevice, &poolInfo, nullptr, &m_CommandPool) != VK_SUCCESS)
		throw std::runtime_error("Failed to create command pool!");
}

void Renderer::CreateCommandBuffers()
{
	m_CommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.commandPool = m_CommandPool;
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocateInfo.commandBufferCount = static_cast<uint32_t>(m_CommandBuffers.size());

	if (vkAllocateCommandBuffers(m_LogicalDevice, &allocateInfo, m_CommandBuffers.data()) != VK_SUCCESS)
		throw std::runtime_error("Failed to allocate command buffers!");
}

void Renderer::CreateSyncObjects()
{
	m_ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	m_RenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	m_InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		if (vkCreateSemaphore(m_LogicalDevice, &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]) != VK_SUCCESS)
			throw std::runtime_error("Failed to create semaphore for 'image available' for a frame!");
		if (vkCreateSemaphore(m_LogicalDevice, &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]) != VK_SUCCESS)
			throw std::runtime_error("Failed to create semaphore for 'render finished' for a frame!");
		if (vkCreateFence(m_LogicalDevice, &fenceInfo, nullptr, &m_InFlightFences[i]) != VK_SUCCESS)
			throw std::runtime_error("Failed to create fence for 'image in flight' for a frame!");
	}
}

void Renderer::CreateVertexBuffer()
{
	VkDeviceSize bufferSize = sizeof(Vertex) * Vertices.size();
	VkMemoryPropertyFlags memProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	// Staging buffer.
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, memProperties, stagingBuffer, stagingBufferMemory);

	// Map memory to CPU.
	void* data;
	vkMapMemory(m_LogicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, Vertices.data(), (size_t)bufferSize);
	vkUnmapMemory(m_LogicalDevice, stagingBufferMemory);

	// Vertex buffer, created in high-performance memory in GPU.
	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, memProperties, m_VertexBuffer, m_VertexBufferMemory);
	// Copy from staging buffer to vertex buffer.
	CopyBuffer(stagingBuffer, m_VertexBuffer, bufferSize);

	// Cleanup staging buffer.
	vkDestroyBuffer(m_LogicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(m_LogicalDevice, stagingBufferMemory, nullptr);
}

//-- Vulkan Rendering.
void Renderer::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0;
	beginInfo.pInheritanceInfo = nullptr;

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
		throw std::runtime_error("Failed to begin recording command buffer!");

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = m_RenderPass;
	renderPassInfo.framebuffer = m_SwapChainFramebuffers[imageIndex];
	renderPassInfo.renderArea.offset = { 0,0 };
	renderPassInfo.renderArea.extent = m_SwapChainExtents;

	VkClearValue clearColor = { 0.f, 0.f, 0.f, 1.f };
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline);

	VkBuffer vertexBuffers[] =
	{
		m_VertexBuffer
	};
	VkDeviceSize offsets[] =
	{
		0
	};
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

	VkViewport viewport;
	viewport.x = 0.f;
	viewport.y = 0.f;
	viewport.width = static_cast<float>(m_SwapChainExtents.width);
	viewport.height = static_cast<float>(m_SwapChainExtents.height);
	viewport.minDepth = 0.f;
	viewport.maxDepth = 1.f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = m_SwapChainExtents;
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	vkCmdDraw(commandBuffer, static_cast<uint32_t>(Vertices.size()), 1, 0, 0);
	vkCmdEndRenderPass(commandBuffer);

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
		throw std::runtime_error("Failed to record command buffer!");
}

void Renderer::DrawFrame()
{
	//-- Wait for sync fence.
	vkWaitForFences(m_LogicalDevice, 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);

	//-- Acquire image from swap chain.
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(m_LogicalDevice, m_SwapChain, UINT64_MAX, m_ImageAvailableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &imageIndex);

	//-- Check if swap chain is out of date.
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		RecreateSwapChain();
		return;
	}
	else if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to present swap-chain image!");

	//-- Reset fence if image acquired.
	vkResetFences(m_LogicalDevice, 1, &m_InFlightFences[m_CurrentFrame]);

	//-- Recording the command buffer.
	vkResetCommandBuffer(m_CommandBuffers[m_CurrentFrame], 0);
	RecordCommandBuffer(m_CommandBuffers[m_CurrentFrame], imageIndex);

	//-- Submit command buffer.
	VkSemaphore waitSemaphores[] =
	{
		m_ImageAvailableSemaphores[m_CurrentFrame]
	};
	VkPipelineStageFlags waitStages[] =
	{
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
	};
	VkSemaphore signalSemaphores[] =
	{
		m_RenderFinishedSemaphores[m_CurrentFrame]
	};

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_CommandBuffers[m_CurrentFrame];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, m_InFlightFences[m_CurrentFrame]) != VK_SUCCESS)
		throw std::runtime_error("Failed to submit draw command buffer!");

	//-- Presentation
	VkSwapchainKHR swapChains[] =
	{
		m_SwapChain
	};

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

	result = vkQueuePresentKHR(m_PresentQueue, &presentInfo);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_FrameBufferResized)
	{
		m_FrameBufferResized = !m_FrameBufferResized;
		RecreateSwapChain();
	}
	else if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to present swap-chain image!");

	// Increment frame counter.
	m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

//-- Buffer Stuffer.
void Renderer::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
	// Create vertex buffer.
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // Only used by graphics queue, so leave as exclusive.

	if (vkCreateBuffer(m_LogicalDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
		throw std::runtime_error("Failed to create vertex buffer!");

	// Allocate memory for vertex buffer.
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(m_LogicalDevice, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(m_LogicalDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
		throw std::runtime_error("Failed to allocate memory for vertex buffer!");
	vkBindBufferMemory(m_LogicalDevice, buffer, bufferMemory, 0);
}

void Renderer::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = m_CommandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(m_LogicalDevice, &allocInfo, &commandBuffer);

	// Record the command buffer.
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	vkEndCommandBuffer(commandBuffer);

	// Submit command buffer.
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(m_GraphicsQueue);

	// Release command buffer.
	vkFreeCommandBuffers(m_LogicalDevice, m_CommandPool, 1, &commandBuffer);
}

//-- Swap-Chain Recreation.
void Renderer::RecreateSwapChain()
{
	// Wait for minimize to end, or for device.
	int width = 0;
	int height = 0;
	glfwGetFramebufferSize(m_Window, &width, &height);
	while (width == 0 || height == 0)
	{
#ifdef _DEBUG
		std::cout << "Framebuffer minimized!" << std::endl;
#endif
		glfwGetFramebufferSize(m_Window, &width, &height);
		glfwWaitEvents();
	}
	vkDeviceWaitIdle(m_LogicalDevice);

	// Cleanup current swap chain.
	DestroySwapChain();

	// Recreate swap chain anew.
	CreateSwapChain();
	CreateImageViews();
	CreateFramebuffers();
}

void Renderer::DestroySwapChain()
{
	// Cleanup swap-chain frame buffers.
	for (auto frameBuffer : m_SwapChainFramebuffers)
		vkDestroyFramebuffer(m_LogicalDevice, frameBuffer, nullptr);

	// Cleanup swap-chain image views.
	for (auto imageView : m_SwapChainImageViews)
		vkDestroyImageView(m_LogicalDevice, imageView, nullptr);

	// Cleanup swap-chain.
	vkDestroySwapchainKHR(m_LogicalDevice, m_SwapChain, nullptr);
}

//-- Swap-Chain Settings.
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

//-- GPU Querying.
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

uint32_t Renderer::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &memoryProperties);

	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
	{
		if  (typeFilter & (1 << i) && 
			(memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
			return i;
	}

	throw std::runtime_error("Failed to find suitable memory type in GPU!");
}

//-- Graphics Pipeline.
VkShaderModule Renderer::CreateShaderModule(std::vector<char>& shaderCode)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = shaderCode.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(m_LogicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		throw std::runtime_error("Failed to create shader module!");

	return shaderModule;
}
