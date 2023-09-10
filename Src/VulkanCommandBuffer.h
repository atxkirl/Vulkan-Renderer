/*!
\file		VulkanCommandBuffer.h
\date		07/09/2023

\author		Adrian Tan
\email		t.xingkhiangadrian\@digipen.edu

\brief		Contains definition of VulkanCommandBuffer class.

\copyright	All content © 2023 DigiPen (SINGAPORE) Corporation, all rights reserved.
			Reproduction or disclosure of this file or its contents without the
			prior written consent of DigiPen Institute of Technology is prohibited.
__________________________________________________________________________________*/
#pragma once

#include <vulkan/vulkan.h>

namespace Nya
{
	class VulkanCommandBuffer
	{
		VkCommandBuffer m_CommandBuffer{};

	public:
		void Init(VkCommandPool commandPool);

		VkCommandBuffer GetCommandBuffer() const;
	};
}
