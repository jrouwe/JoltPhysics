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
	uint vertex_offset = 0, vertex_alignment = 4;
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
			vertex_alignment = max(vertex_alignment, 16u);
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
	vertex_descriptor.layouts[0].stride = AlignUp(vertex_offset, vertex_alignment);
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
	descriptor.colorAttachments[0].pixelFormat = mRenderer->GetView().colorPixelFormat;
	descriptor.vertexDescriptor = vertex_descriptor;

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
