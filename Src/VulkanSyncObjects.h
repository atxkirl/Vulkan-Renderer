/*!
\file		VulkanSyncObjects.h
\date		07/09/2023

\author		Adrian Tan
\email		t.xingkhiangadrian\@digipen.edu

\brief		Contains definition of VulkanSyncObjects class.

\copyright	All content © 2023 DigiPen (SINGAPORE) Corporation, all rights reserved.
			Reproduction or disclosure of this file or its contents without the
			prior written consent of DigiPen Institute of Technology is prohibited.
__________________________________________________________________________________*/
#pragma once
#include <vulkan/vulkan_core.h>

namespace Nya
{
	class VulkanSyncObjects
	{
		std::vector<VkSemaphore> m_ImageAvailableSemaphores;
		std::vector<VkSemaphore> m_RenderFinishedSemaphores;
		std::vector<VkFence> m_InFlightFences;

	public:
		void Init();
		void Cleanup() const;

		VkSemaphore GetImageAvailableSemaphoreAt(size_t index) const;
		VkSemaphore GetRenderFinishedSemaphoreAt(size_t index) const;
		VkFence GetInFlightFenceAt(size_t index) const;
	};
}
