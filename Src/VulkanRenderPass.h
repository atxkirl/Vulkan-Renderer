/*!
\file		VulkanRenderpass.h
\date		07/09/2023

\author		Adrian Tan
\email		t.xingkhiangadrian\@digipen.edu

\brief		Contains definition of VulkanRenderpass class.

\copyright	All content © 2023 DigiPen (SINGAPORE) Corporation, all rights reserved.
			Reproduction or disclosure of this file or its contents without the
			prior written consent of DigiPen Institute of Technology is prohibited.
__________________________________________________________________________________*/
#pragma once

#include "VulkanDefines.h"

namespace Nya
{
	class VulkanRenderpass
	{
		VkRenderPass m_RenderPass{};

	public:
		void Init();
		void Cleanup() const;

		VkRenderPass GetRenderPass() const;
	};
}
