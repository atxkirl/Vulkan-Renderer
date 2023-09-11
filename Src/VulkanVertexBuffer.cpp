/*!
\file		VulkanVertexBuffer.cpp
\date		11/09/2023

\author		Adrian Tan
\email		t.xingkhiangadrian\@digipen.edu

\brief		Contains implementation for VulkanVertexBuffer class functions.

\copyright	All content © 2023 DigiPen (SINGAPORE) Corporation, all rights reserved.
			Reproduction or disclosure of this file or its contents without the
			prior written consent of DigiPen Institute of Technology is prohibited.
__________________________________________________________________________________*/

#include "meowpch.h"

#include "VulkanVertexBuffer.h"
#include "VulkanLogicalDevice.h"
#include "VulkanCommandPool.h"
#include "Vertex.h"

namespace Nya
{
	void VulkanVertexBuffer::Init(const std::vector<Vertex>& vertices, VkCommandPool commandPool)
	{
		const VkDeviceSize bufferSize = sizeof(Vertex) * vertices.size();
		constexpr VkMemoryPropertyFlags memProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		// Staging buffer.
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, memProperties, stagingBuffer, stagingBufferMemory);

		// Map memory to CPU.
		void* data;
		vkMapMemory(VulkanLogicalDevice::Get().GetLogicalDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), (size_t)bufferSize);
		vkUnmapMemory(VulkanLogicalDevice::Get().GetLogicalDevice(), stagingBufferMemory);

		// Vertex buffer, created in high-performance memory in GPU.
		CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_Buffer, m_BufferMemory);
		// Copy from staging buffer to vertex buffer.
		CopyBuffer(stagingBuffer, m_Buffer, bufferSize, commandPool);

		// Cleanup staging buffer.
		vkDestroyBuffer(VulkanLogicalDevice::Get().GetLogicalDevice(), stagingBuffer, nullptr);
		vkFreeMemory(VulkanLogicalDevice::Get().GetLogicalDevice(), stagingBufferMemory, nullptr);
	}
}
