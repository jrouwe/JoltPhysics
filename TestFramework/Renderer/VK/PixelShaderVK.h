// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Renderer/PixelShader.h>

#include <vulkan/vulkan.h>

/// Pixel shader handle for Vulkan
class PixelShaderVK : public PixelShader
{
public:
	/// Constructor
							PixelShaderVK(VkDevice inDevice, VkShaderModule inShaderModule) :
		mDevice(inDevice),
		mStageInfo()
	{
		mStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		mStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		mStageInfo.module = inShaderModule;
		mStageInfo.pName = "main";
	}

	/// Destructor
	virtual					~PixelShaderVK() override
	{
		vkDestroyShaderModule(mDevice, mStageInfo.module, nullptr);
	}

	VkDevice				mDevice;
	VkPipelineShaderStageCreateInfo mStageInfo;
};
