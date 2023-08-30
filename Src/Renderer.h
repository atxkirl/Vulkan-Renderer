#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <optional>
#include <set>
#include <vector>
#include <map>

class Renderer
{
//-- Structures
	struct QueueFamilyIndices
	{
		std::optional<uint32_t> m_GraphicsFamily;
		std::optional<uint32_t> m_PresentFamily;

		bool IsComplete()
		{
			return
				m_GraphicsFamily.has_value() &&
				m_PresentFamily.has_value();
		}
	};
	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR m_Capabilities;
		std::vector<VkSurfaceFormatKHR> m_Formats;
		std::vector<VkPresentModeKHR> m_PresentModes;
	};


//-- Variables
private:
	GLFWwindow* m_Window;
	
	VkInstance m_Instance;
	VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
	VkDevice m_LogicalDevice;

	VkSurfaceKHR m_Surface;
	VkSwapchainKHR m_SwapChain;

	std::vector<VkImage> m_SwapChainImages;
	std::vector<VkImageView> m_SwapChainImageViews;
	std::vector<VkFramebuffer> m_SwapChainFramebuffers;

	VkFormat m_SwapChainImageFormat;
	VkExtent2D m_SwapChainExtents;

	VkQueue m_GraphicsQueue;
	VkQueue m_PresentQueue;

	VkRenderPass m_RenderPass;
	VkPipelineLayout m_PipelineLayout;
	VkPipeline m_GraphicsPipeline;

	VkCommandPool m_CommandPool;
	std::vector<VkCommandBuffer> m_CommandBuffers;			// Command buffers for each frame.
	std::vector<VkSemaphore> m_ImageAvailableSemaphores;	// Semaphores for each frame.
	std::vector<VkSemaphore> m_RenderFinishedSemaphores;	// Semaphores for each frame.
	std::vector<VkFence> m_InFlightFences;					// Fences for each frame.
	uint32_t m_CurrentFrame;								// Counter for the current frame. Between 0 and MAX_FRAMES_IN_FLIGHT.


//-- Functions
public:
	void Initialize();
	void Update();
	void Shutdown();

private:
	//-- Main API initialization.
	void InitGLFW();
	void InitVulkan();

	//-- Extension checks.
	void CheckExtensionSupport();
	bool CheckValidationLayerSupport();
	bool CheckDeviceExtensionSupport(VkPhysicalDevice device);

	//-- GPU Selection Helpers.
	bool IsDeviceSuitable(VkPhysicalDevice device);
	int RateDevice(VkPhysicalDevice device);

	//-- Vulkan initialization.
	void CreateVulkanInstance();
	void SelectPhysicalGPU();
	void CreateLogicalDevice();
	void CreateSurface();
	void CreateSwapChain();
	void CreateImageViews();
	void CreateRenderPass();
	void CreateGraphicsPipeline();
	void CreateFramebuffers();
	void CreateCommandPool();
	void CreateCommandBuffers();
	void CreateSyncObjects();

	// Vulkan rendering!
	void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
	void DrawFrame();

	//-- Select swap chain settings.
	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	//-- GPU querying.
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);

	//-- Grahics Pipeline.
	VkShaderModule CreateShaderModule(std::vector<char>& shaderCode);
};