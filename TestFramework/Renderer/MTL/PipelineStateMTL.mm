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
	// Create a vertex descriptor
	MTLVertexDescriptor *vertex_descriptor = [[MTLVertexDescriptor alloc] init];
	uint vertex_offset = 0;
	uint instance_offset = 0, instance_alignment = 4;
	uint index = 0;
	for (uint i = 0; i < inInputDescriptionCount; ++i)
		switch (inInputDescription[i])
		{
		case EInputDescription::Position:
		case EInputDescription::Normal:
			vertex_descriptor.attributes[index].format = MTLVertexFormatFloat3;
			vertex_descriptor.attributes[index].offset = vertex_offset;
			vertex_descriptor.attributes[index].bufferIndex = 0;
			vertex_offset += 3 * sizeof(float);
			++index;
			break;

		case EInputDescription::Color:
			vertex_descriptor.attributes[index].format = MTLVertexFormatUChar4;
			vertex_descriptor.attributes[index].offset = vertex_offset;
			vertex_descriptor.attributes[index].bufferIndex = 0;
			vertex_offset += 4 * sizeof(uint8);
			++index;
			break;

		case EInputDescription::TexCoord:
			vertex_descriptor.attributes[index].format = MTLVertexFormatFloat2;
			vertex_descriptor.attributes[index].offset = vertex_offset;
			vertex_descriptor.attributes[index].bufferIndex = 0;
			vertex_offset += 2 * sizeof(float);
			++index;
			break;

		case EInputDescription::InstanceColor:
			vertex_descriptor.attributes[index].format = MTLVertexFormatUChar4;
			vertex_descriptor.attributes[index].offset = instance_offset;
			vertex_descriptor.attributes[index].bufferIndex = 1;
			instance_offset += 4 * sizeof(uint8);
			++index;
			break;

		case EInputDescription::InstanceTransform:
		case EInputDescription::InstanceInvTransform:
			instance_alignment = max(instance_alignment, 16u);
			instance_offset = AlignUp(instance_offset, 16u);
			for (int j = 0; j < 4; ++j)
			{
				vertex_descriptor.attributes[index].format = MTLVertexFormatFloat4;
				vertex_descriptor.attributes[index].offset = instance_offset;
				vertex_descriptor.attributes[index].bufferIndex = 1;
				instance_offset += 4 * sizeof(float);
				++index;
			}
			break;
		}

	// Configure layouts
	vertex_descriptor.layouts[0].stride = vertex_offset;
	vertex_descriptor.layouts[0].stepRate = 1;
	vertex_descriptor.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;

	if (instance_offset > 0)
	{
		vertex_descriptor.layouts[1].stride = AlignUp(instance_offset, instance_alignment);
		vertex_descriptor.layouts[1].stepRate = 1;
		vertex_descriptor.layouts[1].stepFunction = MTLVertexStepFunctionPerInstance;
	}

	// Create the pipeline descriptor
	MTLRenderPipelineDescriptor *descriptor = [[MTLRenderPipelineDescriptor alloc] init];
	descriptor.vertexFunction = inVertexShader->GetFunction();
	descriptor.fragmentFunction = inPixelShader->GetFunction();
	descriptor.vertexDescriptor = vertex_descriptor;
	switch (inDrawPass)
	{
	case EDrawPass::Shadow:
		descriptor.depthAttachmentPixelFormat = static_cast<TextureMTL *>(mRenderer->GetShadowMap())->GetTexture().pixelFormat;
		break;

	case EDrawPass::Normal:
		descriptor.colorAttachments[0].pixelFormat = mRenderer->GetView().colorPixelFormat;
		switch (inBlendMode)
		{
		case EBlendMode::Write:
			descriptor.colorAttachments[0].blendingEnabled = NO;
			break;

		case EBlendMode::AlphaBlend:
			descriptor.colorAttachments[0].blendingEnabled = YES;
			descriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
			descriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
			descriptor.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
			descriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorZero;
			descriptor.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorZero;
			descriptor.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
			break;
		}
		descriptor.depthAttachmentPixelFormat = mRenderer->GetView().depthStencilPixelFormat;
	}

	NSError *error = nullptr;
	mPipelineState = [mRenderer->GetDevice() newRenderPipelineStateWithDescriptor: descriptor error: &error];
	FatalErrorIfFailed(error);
	[descriptor release];
	[vertex_descriptor release];

	// Create depth descriptor
	MTLDepthStencilDescriptor *depth_descriptor = [[MTLDepthStencilDescriptor new] init];
	if (inDepthTest == EDepthTest::On)
	{
		depth_descriptor.depthCompareFunction = MTLCompareFunctionGreater;
		depth_descriptor.depthWriteEnabled = YES;
	}
	else
	{
		depth_descriptor.depthCompareFunction = MTLCompareFunctionAlways;
		depth_descriptor.depthWriteEnabled = NO;
	}
	mDepthState = [mRenderer->GetDevice() newDepthStencilStateWithDescriptor: depth_descriptor];
	[depth_descriptor release];

	// Determine cull mode
	if (inCullMode == ECullMode::FrontFace)
		mCullMode = MTLCullModeFront;
	else
		mCullMode = MTLCullModeBack;

	// Determine fill mode
	if (inFillMode == EFillMode::Solid)
		mFillMode = MTLTriangleFillModeFill;
	else
		mFillMode = MTLTriangleFillModeLines;
}

PipelineStateMTL::~PipelineStateMTL()
{
	[mPipelineState release];
	[mDepthState release];
}

void PipelineStateMTL::Activate()
{
	id<MTLRenderCommandEncoder> encoder = mRenderer->GetRenderEncoder();
	[encoder setRenderPipelineState: mPipelineState];
	[encoder setDepthStencilState: mDepthState];
	[encoder setCullMode: mCullMode];
	[encoder setTriangleFillMode: mFillMode];
}
