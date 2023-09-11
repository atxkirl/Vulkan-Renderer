/*!
\file		VulkanBuffers.cpp
\date		07/09/2023

\author		Adrian Tan
\email		t.xingkhiangadrian\@digipen.edu

\brief		Contains implementation for VulkanBuffers class functions.

\copyright	All content © 2023 DigiPen (SINGAPORE) Corporation, all rights reserved.
			Reproduction or disclosure of this file or its contents without the
			prior written consent of DigiPen Institute of Technology is prohibited.
__________________________________________________________________________________*/

#include "meowpch.h"

#include "VulkanBuffers.h"
#include "VulkanLogicalDevice.h"
#include "VulkanPhysicalDevice.h"
#include "VulkanQuery.h"

namespace Nya
{
	void VulkanBuffer::CreateBuffer(const VkDeviceSize size, const VkBufferUsageFlags usage, const VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
	{
		// Create vertex buffer.
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // Only used by graphics queue, so leave as exclusive.

		if (vkCreateBuffer(VulkanLogicalDevice::Get().GetLogicalDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
			throw std::runtime_error("Failed to create vertex buffer!");

		// Allocate memory for vertex buffer.
		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(VulkanLogicalDevice::Get().GetLogicalDevice(), buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = VulkanQuery::FindMemoryType(VulkanPhysicalDevice::Get().GetPhysicalDevice(), memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(VulkanLogicalDevice::Get().GetLogicalDevice(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
			throw std::runtime_error("Failed to allocate memory for vertex buffer!");
		vkBindBufferMemory(VulkanLogicalDevice::Get().GetLogicalDevice(), buffer, bufferMemory, 0);
	}

	void VulkanBuffer::CopyBuffer(const VkBuffer srcBuffer, const VkBuffer dstBuffer, const VkDeviceSize size, const VkCommandPool commandPool)
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = commandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(VulkanLogicalDevice::Get().GetLogicalDevice(), &allocInfo, &commandBuffer);

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

		vkQueueSubmit(VulkanLogicalDevice::Get().GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(VulkanLogicalDevice::Get().GetGraphicsQueue());

		// Release command buffer.
		vkFreeCommandBuffers(VulkanLogicalDevice::Get().GetLogicalDevice(), commandPool, 1, &commandBuffer);
	}

	void VulkanBuffer::Cleanup() const
	{
		vkDestroyBuffer(VulkanLogicalDevice::Get().GetLogicalDevice(), m_Buffer, nullptr);
		vkFreeMemory(VulkanLogicalDevice::Get().GetLogicalDevice(), m_BufferMemory, nullptr);
	}

	VkBuffer VulkanBuffer::GetBuffer() const
	{
		return m_Buffer;
	}

	VkDeviceMemory VulkanBuffer::GetBufferMemory() const
	{
		return m_BufferMemory;
	}
}
