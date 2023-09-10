/*!
\file		VulkanContext.h
\date		08/09/2023

\author		Adrian Tan
\email		t.xingkhiangadrian\@digipen.edu

\brief		Contains definition of VulkanContext class.

\copyright	All content © 2023 DigiPen (SINGAPORE) Corporation, all rights reserved.
			Reproduction or disclosure of this file or its contents without the
			prior written consent of DigiPen Institute of Technology is prohibited.
__________________________________________________________________________________*/
#pragma once

#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "VulkanDefines.h"

namespace Nya
{
	class VulkanContext
	{
		static std::unique_ptr<VulkanContext> s_Instance;

		//std::shared_ptr<Window> m_Window;
		GLFWwindow* m_Window;
		VkInstance m_Instance{};
		VkSurfaceKHR m_Surface{};

		std::vector<const char*> m_Extensions;

		void CreateInstance();
		void CreateSurface();

		bool CheckValidationLayerSupport() const;
		void GetRequiredExtensions();

	public:
		static VulkanContext& Get();

		void Init(GLFWwindow* window);
		void Cleanup() const;

		GLFWwindow* GetWindow() const;
		VkInstance GetInstance() const;
		VkSurfaceKHR GetSurface() const;
	};
}
