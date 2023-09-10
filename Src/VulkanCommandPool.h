/*!
\file		VulkanCommandPool.h
\date		10/09/2023

\author		Adrian Tan
\email		t.xingkhiangadrian\@digipen.edu

\brief		Contains definition of VulkanCommandPool class.

\copyright	All content © 2023 DigiPen (SINGAPORE) Corporation, all rights reserved.
			Reproduction or disclosure of this file or its contents without the
			prior written consent of DigiPen Institute of Technology is prohibited.
__________________________________________________________________________________*/
#pragma once
#include <vulkan/vulkan_core.h>

namespace Nya
{
	class VulkanCommandPool
	{
		VkCommandPool m_CommandPool{};

	public:
		void Init();
		void Cleanup() const;

		VkCommandPool GetCommandPool() const;
	};
}
