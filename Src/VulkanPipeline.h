/*!
\file		VulkanPipeline.h
\date		07/09/2023

\author		Adrian Tan
\email		t.xingkhiangadrian\@digipen.edu

\brief		Contains definition of VulkanPipeline class.

\copyright	All content © 2023 DigiPen (SINGAPORE) Corporation, all rights reserved.
			Reproduction or disclosure of this file or its contents without the
			prior written consent of DigiPen Institute of Technology is prohibited.
__________________________________________________________________________________*/
#pragma once

#include <vulkan/vulkan_core.h>

namespace Nya
{
	class VulkanPipeline
	{
		VkPipelineLayout m_PipelineLayout{};
		VkPipeline m_GraphicsPipeline{};

	public:
		void Init(VkRenderPass renderPass);
		void Cleanup() const;

		VkPipeline GetPipeline() const;
		VkPipelineLayout GetLayout() const;
	};
}
