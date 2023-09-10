#include "VulkanRenderer.h"
#include "Vertex.h"

const uint32_t WIN_WIDTH = 800;		// Window width.
const uint32_t WIN_HEIGHT = 600;	// Window height.


static void FrameBufferResizedCallbackFn(GLFWwindow* window, int width, int height)
{
	auto app = reinterpret_cast<MeowRenderer*>(glfwGetWindowUserPointer(window));
	app->FlagFrameBufferResized();
}

const std::vector<Nya::Vertex> Vertices =
{
	{{-0.5f, -0.5f}, {1.f, 0.f, 0.f}},
	{{0.5f, -0.5f}, {0.f, 1.f, 0.f}},
	{{0.5f, 0.5f}, {0.f, 0.f, 1.f}},
	{{-0.5f, 0.5f}, {1.f, 1.f, 1.f}}
};
const std::vector<uint32_t> Indices =
{
	0, 1, 2, 2, 3, 0
};


std::unique_ptr<MeowRenderer> MeowRenderer::s_Instance = nullptr;
MeowRenderer& MeowRenderer::Get()
{
	if (!s_Instance)
		s_Instance = std::make_unique<MeowRenderer>();

	return *s_Instance;
}

void MeowRenderer::Init()
{
	// Initialize GLFW window.
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	m_Window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, "Vulkan Renderer", nullptr, nullptr);

	glfwSetWindowUserPointer(m_Window, this);
	glfwSetFramebufferSizeCallback(m_Window, FrameBufferResizedCallbackFn);
	

	// Initialize all singleton objects.
	VulkanContext::Get().Init(m_Window);
	VulkanPhysicalDevice::Get().Init();
	VulkanLogicalDevice::Get().Init();
	VulkanSwapchain::Get().Init();

	// Create render pass.
	m_RenderPass = std::make_shared<VulkanRenderpass>();
	m_RenderPass->Init();

	// Craete graphics pipeline.
	m_Pipeline = std::make_shared<VulkanPipeline>();
	m_Pipeline->Init(m_RenderPass->GetRenderPass());

	// Create frame buffers.
	for (size_t i = 0; i < VulkanSwapchain::Get().GetSwapChainImageViews().size(); ++i)
	{
		VkImageView attachments[] =
		{
			VulkanSwapchain::Get().GetSwapChainImageViews()[i]
		};

		auto framebuffer = std::make_shared<VulkanFramebuffer>();
		framebuffer->Init(i, attachments, m_RenderPass->GetRenderPass());

		m_Framebuffers.push_back(framebuffer);
	}

	// Create command pool.
	m_CommandPool = std::make_shared<VulkanCommandPool>();
	m_CommandPool->Init();

	// Create command buffers.
	for (size_t i = 0; i < g_MaxFramesInFlight; ++i)
	{
		auto commandBuffer = std::make_shared<VulkanCommandBuffer>();
		commandBuffer->Init(m_CommandPool->GetCommandPool());
		m_CommandBuffers.push_back(commandBuffer);
	}

	// Create sync objects (semaphores and fences).
	m_SyncObjects = std::make_shared<VulkanSyncObjects>();
	m_SyncObjects->Init();
}

void MeowRenderer::Update()
{
	while (!glfwWindowShouldClose(m_Window))
	{
		glfwPollEvents();
		Draw();
	}

	vkDeviceWaitIdle(VulkanLogicalDevice::Get().GetLogicalDevice());
}

void MeowRenderer::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0;
	beginInfo.pInheritanceInfo = nullptr;

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
		throw std::runtime_error("Failed to begin recording command buffer!");

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = m_RenderPass->GetRenderPass();
	renderPassInfo.framebuffer = m_Framebuffers[imageIndex]->GetFramebuffer();
	renderPassInfo.renderArea.offset = { 0,0 };
	renderPassInfo.renderArea.extent = VulkanSwapchain::Get().GetSwapChainImageExtents();

	VkClearValue clearColor = { 0.f, 0.f, 0.f, 1.f };
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline->GetPipeline());

	VkBuffer vertexBuffers[] =
	{
		m_VertexBuffer
	};
	VkDeviceSize offsets[] =
	{
		0
	};
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
	vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer, 0, VK_INDEX_TYPE_UINT32);

	VkViewport viewport;
	viewport.x = 0.f;
	viewport.y = 0.f;
	viewport.width = static_cast<float>(VulkanSwapchain::Get().GetSwapChainImageExtents().width);
	viewport.height = static_cast<float>(VulkanSwapchain::Get().GetSwapChainImageExtents().height);
	viewport.minDepth = 0.f;
	viewport.maxDepth = 1.f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = VulkanSwapchain::Get().GetSwapChainImageExtents();
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	//vkCmdDraw(commandBuffer, static_cast<uint32_t>(Vertices.size()), 1, 0, 0); // Redundant, doesn't use index buffer.
	vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(Indices.size()), 1, 0, 0, 0);
	vkCmdEndRenderPass(commandBuffer);

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
		throw std::runtime_error("Failed to record command buffer!");
}

void MeowRenderer::Draw()
{
	//-- Wait for sync fence.
	vkWaitForFences(VulkanLogicalDevice::Get().GetLogicalDevice(), 1, &m_SyncObjects->GetInFlightFenceAt(m_CurrentFrame), VK_TRUE, UINT64_MAX);

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

void MeowRenderer::Release()
{
}

void MeowRenderer::FlagFrameBufferResized()
{
#ifdef _DEBUG
	std::cout << "Frame buffer resized!" << std::endl;
#endif

	m_FrameBufferResized = true;
}
