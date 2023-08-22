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
//-- Variables
private:
	GLFWwindow* m_Window;
	
	VkInstance m_Instance;
	VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
	VkDevice m_LogicalDevice;

	VkSurfaceKHR m_Surface;
	VkSwapchainKHR m_SwapChain;

	std::vector<VkImage> m_SwapChainImages;
	VkFormat m_SwapChainImageFormat;
	VkExtent2D m_SwapChainExtents;

	VkQueue m_GraphicsQueue;
	VkQueue m_PresentQueue;

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


//-- Functions
public:
	void Initialize();
	void Update();
	void Shutdown();

private:
	void InitGLFW();
	void InitVulkan();

	void CheckExtensionSupport();
	bool CheckValidationLayerSupport();
	bool CheckDeviceExtensionSupport(VkPhysicalDevice device);

	void CreateVulkanInstance();
	void SelectPhysicalGPU();
	void CreateLogicalDevice();
	void CreateSurface();
	void CreateSwapChain();

	bool IsDeviceSuitable(VkPhysicalDevice device);
	int RateDevice(VkPhysicalDevice device);

	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
};