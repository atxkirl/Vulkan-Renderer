#pragma once

#include "meowpch.h"

#include "VulkanCommandBuffer.h"
#include "VulkanCommandPool.h"
#include "VulkanContext.h"
#include "VulkanDefines.h"
#include "VulkanFrameBuffer.h"
#include "VulkanPipeline.h"
#include "VulkanRenderPass.h"
#include "VulkanSyncObjects.h"
#include "VulkanVertexBuffer.h"
#include "VulkanIndexBuffer.h"


class MeowRenderer
{
	static std::unique_ptr<MeowRenderer> s_Instance;

	std::shared_ptr<Nya::VulkanRenderpass> m_RenderPass;
	std::shared_ptr<Nya::VulkanPipeline> m_Pipeline;

	std::shared_ptr<Nya::VulkanCommandPool> m_CommandPool;
	std::vector<std::shared_ptr<Nya::VulkanCommandBuffer>> m_CommandBuffers;

	std::shared_ptr<Nya::VulkanSyncObjects> m_SyncObjects;

	// TESTING VARIABLES.
	GLFWwindow* m_Window{};
	bool m_FrameBufferResized = false;
	int m_CurrentFrame = 0;
	std::shared_ptr<Nya::VulkanVertexBuffer> m_VertexBuffer;
	std::shared_ptr<Nya::VulkanIndexBuffer> m_IndexBuffer;
	// ~TESTING VARIABLES

public:
	static MeowRenderer& Get();

	void Init();

	void Update();

	void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
	void Draw();

	void Release();

	void FlagFrameBufferResized();
};