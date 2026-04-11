// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Renderer/PixelShader.h>
#include <Renderer/VK/RendererVK.h>

#include <vulkan/vulkan.h>

/// Pixel shader handle for Vulkan
class PixelShaderVK : public PixelShader
{
public:
	/// Constructor
							PixelShaderVK(RendererVK *inRenderer, VkShaderModule inShaderModule) :
		mRenderer(inRenderer)
	{
		mStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		mStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		mStageInfo.module = inShaderModule;
		mStageInfo.pName = "main";
	}

	/// Destructor
	virtual					~PixelShaderVK() override
	{
		mRenderer->mVkDestroyShaderModule(mRenderer->GetDevice(), mStageInfo.module, nullptr);
	}

	RendererVK *			mRenderer;
	VkPipelineShaderStageCreateInfo mStageInfo = {};
};
