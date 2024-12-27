// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Renderer/PipelineState.h>
#include <Renderer/VK/VertexShaderVK.h>
#include <Renderer/VK/PixelShaderVK.h>

class RendererVK;

/// Vulkan pipeline state object
class PipelineStateVK : public PipelineState
{
public:
	/// Constructor
										PipelineStateVK(RendererVK *inRenderer, const VertexShaderVK *inVertexShader, const EInputDescription *inInputDescription, uint inInputDescriptionCount, const PixelShaderVK *inPixelShader, EDrawPass inDrawPass, EFillMode inFillMode, ETopology inTopology, EDepthTest inDepthTest, EBlendMode inBlendMode, ECullMode inCullMode);
	virtual								~PipelineStateVK() override;

	/// Make this pipeline state active (any primitives rendered after this will use this state)
	virtual void						Activate() override;

private:
	RendererVK *						mRenderer;
	RefConst<VertexShaderVK>			mVertexShader;
	RefConst<PixelShaderVK>				mPixelShader;

	VkPipeline							mGraphicsPipeline;
};
