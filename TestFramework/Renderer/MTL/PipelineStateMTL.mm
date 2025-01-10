// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Renderer/MTL/PipelineStateMTL.h>
#include <Renderer/MTL/RendererMTL.h>
#include <Renderer/MTL/FatalErrorIfFailedMTL.h>

PipelineStateMTL::PipelineStateMTL(RendererMTL *inRenderer, const VertexShaderMTL *inVertexShader, const EInputDescription *inInputDescription, uint inInputDescriptionCount, const PixelShaderMTL *inPixelShader, EDrawPass inDrawPass, EFillMode inFillMode, ETopology inTopology, EDepthTest inDepthTest, EBlendMode inBlendMode, ECullMode inCullMode) :
	mRenderer(inRenderer),
	mVertexShader(inVertexShader),
	mPixelShader(inPixelShader)
{
	MTLRenderPipelineDescriptor *descriptor = [[MTLRenderPipelineDescriptor alloc] init];
	descriptor.vertexFunction = inVertexShader->GetFunction();
	descriptor.fragmentFunction = inPixelShader->GetFunction();
	descriptor.colorAttachments[0].pixelFormat = mRenderer->GetView().colorPixelFormat;

	NSError *error = nullptr;
	mPipelineState = [mRenderer->GetDevice() newRenderPipelineStateWithDescriptor: descriptor error: &error];
	FatalErrorIfFailed(error);
}

PipelineStateMTL::~PipelineStateMTL()
{
}

void PipelineStateMTL::Activate()
{
	[mRenderer->GetRenderEncoder() setRenderPipelineState: mPipelineState];
}
