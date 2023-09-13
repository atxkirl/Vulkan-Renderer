#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>

#include <optional>
#include <set>
#include <vector>
#include <map>
#include <array>

struct Vertex
{
	glm::vec2 pos;
	glm::vec3 color;

	static VkVertexInputBindingDescription GetBindingDescription()
	{
		VkVertexInputBindingDescription description{};
		description.binding = 0;
		description.stride = sizeof(Vertex);
		description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return description;
	}

	static std::array<VkVertexInputAttributeDescription, 2> GetAttributeDescription()
	{
		std::array<VkVertexInputAttributeDescription, 2> attributes{};
		// Vertex position.
		attributes[0].binding = 0;
		attributes[0].location = 0;
		attributes[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributes[0].offset = offsetof(Vertex, pos);
		// Vertex colour;
		attributes[1].binding = 0;
		attributes[1].location = 1;
		attributes[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributes[1].offset = offsetof(Vertex, color);

		return attributes;
	}
};

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
		VkSurfaceCapabilitiesKHR m_Capabilities{};
		std::vector<VkSurfaceFormatKHR> m_Formats;
		std::vector<VkPresentModeKHR> m_PresentModes;
	};


//-- Variables
private:
	GLFWwindow* m_Window{};
	
	VkInstance m_Instance{};
	VkDebugUtilsMessengerEXT m_DebugMessenger{};

	VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
	VkDevice m_LogicalDevice{};

	VkSurfaceKHR m_Surface{};
	VkSwapchainKHR m_SwapChain{};
	bool m_FrameBufferResized = false;

	std::vector<VkImage> m_SwapChainImages;
	std::vector<VkImageView> m_SwapChainImageViews;
	std::vector<VkFramebuffer> m_SwapChainFramebuffers;

	VkFormat m_SwapChainImageFormat{};
	VkExtent2D m_SwapChainExtents{};

	VkQueue m_GraphicsQueue{};
	VkQueue m_PresentQueue{};

	VkRenderPass m_RenderPass{};
	VkPipelineLayout m_PipelineLayout{};
	VkPipeline m_GraphicsPipeline{};

	VkCommandPool m_CommandPool{};
	std::vector<VkCommandBuffer> m_CommandBuffers;			// Command buffers for each frame.
	std::vector<VkSemaphore> m_ImageAvailableSemaphores;	// Semaphores for each frame.
	std::vector<VkSemaphore> m_RenderFinishedSemaphores;	// Semaphores for each frame.
	std::vector<VkFence> m_InFlightFences;					// Fences for each frame.
	uint32_t m_CurrentFrame = 0;							// Counter for the current frame. Between 0 and MAX_FRAMES_IN_FLIGHT.

	VkBuffer m_VertexBuffer{};
	VkDeviceMemory m_VertexBufferMemory{};

	VkBuffer m_IndexBuffer{};
	VkDeviceMemory m_IndexBufferMemory{};

	// TESTING
	std::vector<const char*> m_Extensions;


//-- Functions
public:
	void Initialize();
	void Update();
	void Shutdown();

	void FlagFrameBufferResized();

private:
	//-- Main API initialization.
	void InitGLFW();
	void InitVulkan();

	//-- Debug Messenger.
	VkResult CreateDebugUtilMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMsger, const VkAllocationCallbacks* pAllocator);
	void SetupDebugMessenger();
	void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

	//-- Extension checks.
	void CheckExtensionSupport();
	bool CheckValidationLayerSupport();
	bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
	void GetRequiredExtensions();

	//-- GPU Selection.
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
	void CreateVertexBuffer();
	void CreateIndexBuffer();


	//-- Vulkan Rendering!
	void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
	void DrawFrame();

	//-- Buffer Stuffer.
	void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	//-- Swap Chain Recreation.
	void DestroySwapChain();
	void RecreateSwapChain();

	//-- Select swap chain settings.
	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	//-- GPU querying.
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	//-- Grahics Pipeline.
	VkShaderModule CreateShaderModule(std::vector<char>& shaderCode);
};