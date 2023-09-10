/*!
\file		VulkanFramebuffer.h
\date		07/09/2023

\author		Adrian Tan
\email		t.xingkhiangadrian\@digipen.edu

\brief		Contains definition of VulkanFramebuffer class.

\copyright	All content © 2023 DigiPen (SINGAPORE) Corporation, all rights reserved.
			Reproduction or disclosure of this file or its contents without the
			prior written consent of DigiPen Institute of Technology is prohibited.
__________________________________________________________________________________*/
#pragma once

#include <vulkan/vulkan_core.h>

namespace Nya
{
	// Will need 2 framebuffers:
	// - 1 for rendering Game view.
	// - 1 for rendering Editor view.

	class VulkanFramebuffer
	{
		VkFramebuffer m_Framebuffer{};

	public:
		void Init(size_t index, VkImageView attachments[], VkRenderPass renderPass);
		void Cleanup() const;

		VkFramebuffer GetFramebuffer() const;
	};
}
