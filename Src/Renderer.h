#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <optional>
#include <set>

class Renderer
{
//-- Variables
private:
	GLFWwindow* m_Window;
	VkInstance m_Instance;
	VkSurfaceKHR m_Surface;

	VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
	VkDevice m_LogicalDevice;
	
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

	void CreateVulkanInstance();
	void SelectPhysicalGPU();
	void CreateLogicalDevice();
	void CreateSurface();

	bool IsDeviceSuitable(VkPhysicalDevice device);
	int RateDevice(VkPhysicalDevice device);
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
};