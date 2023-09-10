/*!
\file		VulkanCommandBuffer.cpp
\date		07/09/2023

\author		Adrian Tan
\email		t.xingkhiangadrian\@digipen.edu

\brief		Contains implementation for VulkanCommandBuffer class functions.

\copyright	All content © 2023 DigiPen (SINGAPORE) Corporation, all rights reserved.
			Reproduction or disclosure of this file or its contents without the
			prior written consent of DigiPen Institute of Technology is prohibited.
__________________________________________________________________________________*/

#include "meowpch.h"

#include "VulkanCommandBuffer.h"

#include "VulkanLogicalDevice.h"

namespace Nya
{
	void VulkanCommandBuffer::Init(VkCommandPool commandPool)
	{
		VkCommandBufferAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocateInfo.commandPool = commandPool;
		allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocateInfo.commandBufferCount = 1;

		if (vkAllocateCommandBuffers(VulkanLogicalDevice::Get().GetLogicalDevice(), &allocateInfo, &m_CommandBuffer) != VK_SUCCESS)
			throw std::runtime_error("Failed to create command buffer!");
	}

	VkCommandBuffer VulkanCommandBuffer::GetCommandBuffer() const
	{
		return m_CommandBuffer;
	}
}
