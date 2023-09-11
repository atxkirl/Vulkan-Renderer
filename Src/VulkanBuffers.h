/*!
\file		VulkanBuffers.h
\date		07/09/2023

\author		Adrian Tan
\email		t.xingkhiangadrian\@digipen.edu

\brief		Contains definition of VulkanBuffers class.

\copyright	All content © 2023 DigiPen (SINGAPORE) Corporation, all rights reserved.
			Reproduction or disclosure of this file or its contents without the
			prior written consent of DigiPen Institute of Technology is prohibited.
__________________________________________________________________________________*/
#pragma once

#include <vulkan/vulkan.h>

namespace Nya
{
	class VulkanBuffer
	{
	protected:
		VkBuffer m_Buffer{};
		VkDeviceMemory m_BufferMemory{};

		void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkCommandPool commandPool);

	public:
		void Cleanup() const;

		VkBuffer GetBuffer() const;
		VkDeviceMemory GetBufferMemory() const;
	};
}
