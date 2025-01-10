// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Renderer/PipelineState.h>
#include <Renderer/MTL/VertexShaderMTL.h>
#include <Renderer/MTL/PixelShaderMTL.h>

class RendererMTL;

/// Metal pipeline state object
class PipelineStateMTL : public PipelineState
{
public:
	/// Constructor
										PipelineStateMTL(RendererMTL *inRenderer, const VertexShaderMTL *inVertexShader, const EInputDescription *inInputDescription, uint inInputDescriptionCount, const PixelShaderMTL *inPixelShader, EDrawPass inDrawPass, EFillMode inFillMode, ETopology inTopology, EDepthTest inDepthTest, EBlendMode inBlendMode, ECullMode inCullMode);
	virtual								~PipelineStateMTL() override;

	/// Make this pipeline state active (any primitives rendered after this will use this state)
	virtual void						Activate() override;

private:
	RendererMTL *						mRenderer;
	RefConst<VertexShaderMTL>			mVertexShader;
	RefConst<PixelShaderMTL>			mPixelShader;
	id<MTLRenderPipelineState> 			mPipelineState;
	id<MTLDepthStencilState>			mDepthState;
	MTLCullMode							mCullMode;
	MTLTriangleFillMode					mFillMode;
};
