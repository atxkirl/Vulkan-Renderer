/*!
\file		VulkanPipeline.cpp
\date		07/09/2023

\author		Adrian Tan
\email		t.xingkhiangadrian\@digipen.edu

\brief		Contains implementation for VulkanPipeline class functions.

\copyright	All content © 2023 DigiPen (SINGAPORE) Corporation, all rights reserved.
			Reproduction or disclosure of this file or its contents without the
			prior written consent of DigiPen Institute of Technology is prohibited.
__________________________________________________________________________________*/

#include "meowpch.h"

#include "VulkanPipeline.h"
#include "VulkanLogicalDevice.h"
#include "VulkanSwapchain.h"
#include "Vertex.h"

#include <iostream>
#include <fstream>

namespace Nya
{
	//-- Helpers.
	std::vector<char> ReadShaderFile(const std::string& filePath)
	{
		std::ifstream file(filePath, std::ios_base::ate | std::ios_base::binary);

		if (!file.is_open())
			throw std::runtime_error("Failed to open file [" + filePath + "]!");

		size_t fileSize = static_cast<size_t>(file.tellg());
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();
		return buffer;
	}

	VkShaderModule CreateShaderModule(const std::vector<char>& shaderCode)
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = shaderCode.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(VulkanLogicalDevice::Get().GetLogicalDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
			throw std::runtime_error("Failed to create shader module!");

		return shaderModule;
	}
	

	//-- VulkanPipeline Functions.
	void VulkanPipeline::Init(VkRenderPass renderPass)
	{
		//-- Shaders.
		auto vertShaderCode = ReadShaderFile("Shaders/output/vert.spv");
		auto fragShaderCode = ReadShaderFile("Shaders/output/frag.spv");

#ifdef _DEBUG
		std::cout << "\t" << "Vert shader byte size: " << vertShaderCode.size() << std::endl;
		std::cout << "\t" << "Frag shader byte size: " << fragShaderCode.size() << std::endl;
#endif

		// Create shader modules.
		VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode);
		VkShaderModule fragShaderModule = CreateShaderModule(fragShaderCode);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main"; // Name of the entrypoint function in the shader.

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main"; // Name of the entrypoint function in the shader.

		// Create array of shader stages.
		VkPipelineShaderStageCreateInfo shaderStages[] =
		{
			vertShaderStageInfo,
			fragShaderStageInfo
		};

		//-- Vertex input.
		auto bindingDescription = Vertex::GetBindingDescription();
		auto attributeDescription = Vertex::GetAttributeDescription();
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescription.size());
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescription.data();

		//-- Input assembly.
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		//-- Viewport and Scissors.
		VkViewport viewport;
		viewport.x = 0.f;
		viewport.y = 0.f;
		viewport.width = static_cast<float>(VulkanSwapchain::Get().GetSwapChainImageExtents().width);
		viewport.height = static_cast<float>(VulkanSwapchain::Get().GetSwapChainImageExtents().height);
		viewport.minDepth = 0.f;
		viewport.maxDepth = 1.f;

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = VulkanSwapchain::Get().GetSwapChainImageExtents();

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		//viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		//viewportState.pScissors = &scissor;

		//-- Rasterizer.
		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;	// VK_POLYGON_MODE_LINE, VK_POLYGON_MODE_POINT
		rasterizer.lineWidth = 1.f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;	// Cull only BACK facing, FRONT facing, or BOTH
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE; // Orientation of front facing triangles.
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.f;		// Optional.
		rasterizer.depthBiasClamp = 0.f;				// Optional.
		rasterizer.depthBiasSlopeFactor = 0.f;			// Optional.

		//-- Multisampling.
		/// Disabled for now, will enable later down the road.
		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.f;			// Optional.
		multisampling.pSampleMask = nullptr;			// Optional.
		multisampling.alphaToCoverageEnable = VK_FALSE;	// Optional.
		multisampling.alphaToOneEnable = VK_FALSE;		// Optional.

		//-- Depth and Stencil testing.
		/// Disabled for now, so just passing nullptr when creating pipeline.

		//-- Color blending.
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.f;
		colorBlending.blendConstants[1] = 0.f;
		colorBlending.blendConstants[2] = 0.f;
		colorBlending.blendConstants[3] = 0.f;

		//-- Fixed Functions.
		std::vector<VkDynamicState> dynamicStates =
		{
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};
		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		//-- Create pipeline layout.
		VkPipelineLayoutCreateInfo pipelineLayout{};
		pipelineLayout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayout.setLayoutCount = 0;				// Optional.
		pipelineLayout.pSetLayouts = nullptr;			// Optional.
		pipelineLayout.pushConstantRangeCount = 0;		// Optional.
		pipelineLayout.pPushConstantRanges = nullptr;	// Optional.

		if (vkCreatePipelineLayout(VulkanLogicalDevice::Get().GetLogicalDevice(), &pipelineLayout, nullptr, &m_PipelineLayout) != VK_SUCCESS)
			throw std::runtime_error("Failed to create pipeline layout!");

		//-- Create pipeline.
		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = m_PipelineLayout;
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		if (vkCreateGraphicsPipelines(VulkanLogicalDevice::Get().GetLogicalDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_GraphicsPipeline) != VK_SUCCESS)
			throw std::runtime_error("Failed to create graphics pipeline!");

		//-- Cleanup.
		vkDestroyShaderModule(VulkanLogicalDevice::Get().GetLogicalDevice(), vertShaderModule, nullptr);
		vkDestroyShaderModule(VulkanLogicalDevice::Get().GetLogicalDevice(), fragShaderModule, nullptr);
	}

	void VulkanPipeline::Cleanup() const
	{
		vkDestroyPipeline(VulkanLogicalDevice::Get().GetLogicalDevice(), m_GraphicsPipeline, nullptr);
		vkDestroyPipelineLayout(VulkanLogicalDevice::Get().GetLogicalDevice(), m_PipelineLayout, nullptr);
	}

	VkPipeline VulkanPipeline::GetPipeline() const
	{
		return m_GraphicsPipeline;
	}

	VkPipelineLayout VulkanPipeline::GetLayout() const
	{
		return m_PipelineLayout;
	}
}
