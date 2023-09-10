#pragma once

#include "VulkanBuffers.h"
#include "VulkanCommandBuffer.h"
#include "VulkanCommandPool.h"
#include "VulkanContext.h"
#include "VulkanDebugger.h"
#include "VulkanDefines.h"
#include "VulkanFrameBuffer.h"
#include "VulkanLogicalDevice.h"
#include "VulkanPhysicalDevice.h"
#include "VulkanPipeline.h"
#include "VulkanQuery.h"
#include "VulkanRenderPass.h"
#include "VulkanSwapChain.h"
#include "VulkanSyncObjects.h"
using namespace Nya;

#include "meowpch.h"

class MeowRenderer
{
	static std::unique_ptr<MeowRenderer> s_Instance;

	GLFWwindow* m_Window;

	std::shared_ptr<VulkanRenderpass> m_RenderPass;
	std::shared_ptr<VulkanPipeline> m_Pipeline;
	std::vector<std::shared_ptr<VulkanFramebuffer>> m_Framebuffers;

	std::shared_ptr<VulkanCommandPool> m_CommandPool;
	std::vector<std::shared_ptr<VulkanCommandBuffer>> m_CommandBuffers;

	std::shared_ptr<VulkanSyncObjects> m_SyncObjects;




	bool m_FrameBufferResized = false;

public:
	static MeowRenderer& Get();

	void Init();

	void Update();

	void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
	void Draw();

	void Release();

	void FlagFrameBufferResized();
};