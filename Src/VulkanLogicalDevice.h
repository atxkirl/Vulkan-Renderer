/*!
\file		VulkanLogicalDevice.h
\date		09/09/2023

\author		Adrian Tan
\email		t.xingkhiangadrian\@digipen.edu

\brief		Contains definition of VulkanLogicalDevice class.

\copyright	All content © 2023 DigiPen (SINGAPORE) Corporation, all rights reserved.
			Reproduction or disclosure of this file or its contents without the
			prior written consent of DigiPen Institute of Technology is prohibited.
__________________________________________________________________________________*/
#pragma once

#include "VulkanDefines.h"

namespace Nya
{
	class VulkanContext;
	class VulkanPhysicalDevice;

	class VulkanLogicalDevice
	{
		//static std::unique_ptr<VulkanLogicalDevice> s_Instance;
		static VulkanLogicalDevice* s_Instance;

		VkDevice m_LogicalDevice{};
		VkQueue m_GraphicsQueue{};
		VkQueue m_PresentQueue{};

	public:
		static VulkanLogicalDevice& Get();

		void Init();
		void Cleanup() const;

		VkDevice GetLogicalDevice() const;
		VkQueue GetGraphicsQueue() const;
		VkQueue GetPresentQueue() const;
	};
}
