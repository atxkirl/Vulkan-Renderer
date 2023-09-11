/*!
\file		VulkanVertexBuffer.h
\date		11/09/2023

\author		Adrian Tan
\email		t.xingkhiangadrian\@digipen.edu

\brief		Contains definition of VulkanVertexBuffer class.

\copyright	All content © 2023 DigiPen (SINGAPORE) Corporation, all rights reserved.
			Reproduction or disclosure of this file or its contents without the
			prior written consent of DigiPen Institute of Technology is prohibited.
__________________________________________________________________________________*/
#pragma once

#include "VulkanBuffers.h"

namespace Nya
{
	class VulkanCommandPool;
	struct Vertex;

	class VulkanVertexBuffer : public VulkanBuffer
	{
	public:
		void Init(const std::vector<Vertex>& vertices, VkCommandPool commandPool);
	};
}
