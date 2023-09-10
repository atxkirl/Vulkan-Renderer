/*!
\file		VulkanDefines.h
\date		08/09/2023

\author		Adrian Tan
\email		t.xingkhiangadrian\@digipen.edu

\brief		

\copyright	All content © 2023 DigiPen (SINGAPORE) Corporation, all rights reserved.
			Reproduction or disclosure of this file or its contents without the
			prior written consent of DigiPen Institute of Technology is prohibited.
__________________________________________________________________________________*/

#pragma once

#include <vulkan/vulkan.h>

#include <vector>

namespace Nya
{
	// Validation Layers.
#ifdef _DEBUG
	constexpr bool g_EnableValidationLayers = true;
	const std::vector<const char*> g_ValidationLayers =
	{
		"VK_LAYER_KHRONOS_validation"
	};
#else
	constexpr bool g_EnableValidationLayers = false;
	const std::vector<const char*> g_ValidationLayers = {};
#endif

	// Device Extensions.
	const std::vector<const char*> g_DeviceExtensions =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	// Max pre-rendered frames.
	constexpr int g_MaxFramesInFlight = 2;

}
