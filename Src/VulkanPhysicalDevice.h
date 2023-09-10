/*!
\file		VulkanPhysicalDevice.h
\date		09/09/2023

\author		Adrian Tan
\email		t.xingkhiangadrian\@digipen.edu

\brief		Contains definition of VulkanPhysicalDevice class.

\copyright	All content © 2023 DigiPen (SINGAPORE) Corporation, all rights reserved.
			Reproduction or disclosure of this file or its contents without the
			prior written consent of DigiPen Institute of Technology is prohibited.
__________________________________________________________________________________*/
#pragma once

#include "VulkanDefines.h"

namespace Nya
{
	class VulkanPhysicalDevice
	{
		static std::unique_ptr<VulkanPhysicalDevice> s_Instance;

		VkPhysicalDevice m_PhysicalDevice{};

	public:
		static VulkanPhysicalDevice& Get();

		VkPhysicalDevice GetPhysicalDevice() const;

		void Init();
	};
}
