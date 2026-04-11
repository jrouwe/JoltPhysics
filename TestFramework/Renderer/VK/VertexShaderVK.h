// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Renderer/VertexShader.h>
#include <Renderer/VK/RendererVK.h>

#include <vulkan/vulkan.h>

/// Vertex shader handle for Vulkan
class VertexShaderVK : public VertexShader
{
public:
	/// Constructor
							VertexShaderVK(RendererVK *inRenderer, VkShaderModule inShaderModule) :
		mRenderer(inRenderer)
	{
		mStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		mStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		mStageInfo.module = inShaderModule;
		mStageInfo.pName = "main";
	}

	/// Destructor
	virtual					~VertexShaderVK() override
	{
		mRenderer->mVkDestroyShaderModule(mRenderer->GetDevice(), mStageInfo.module, nullptr);
	}

	RendererVK *			mRenderer;
	VkPipelineShaderStageCreateInfo mStageInfo = {};
};
