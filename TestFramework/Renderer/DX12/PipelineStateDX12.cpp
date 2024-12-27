// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Renderer/DX12/PipelineStateDX12.h>
#include <Renderer/DX12/RendererDX12.h>
#include <Renderer/DX12/VertexShaderDX12.h>
#include <Renderer/DX12/PixelShaderDX12.h>
#include <Renderer/DX12/FatalErrorIfFailedDX12.h>

PipelineStateDX12::PipelineStateDX12(RendererDX12 *inRenderer, const VertexShaderDX12 *inVertexShader, const EInputDescription *inInputDescription, uint inInputDescriptionCount, const PixelShaderDX12 *inPixelShader, EDrawPass inDrawPass, EFillMode inFillMode, ETopology inTopology, EDepthTest inDepthTest, EBlendMode inBlendMode, ECullMode inCullMode) :
	mRenderer(inRenderer)
{
	D3D12_PRIMITIVE_TOPOLOGY_TYPE topology = inTopology == ETopology::Triangle? D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE : D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;

	Array<D3D12_INPUT_ELEMENT_DESC> input_description;
	uint vertex_offset = 0, instance_offset = 0;
	for (uint i = 0; i < inInputDescriptionCount; ++i)
		switch (inInputDescription[i])
		{
		case EInputDescription::Position:
			input_description.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, vertex_offset, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
			vertex_offset += 3 * sizeof(float);
			break;

		case EInputDescription::Color:
			input_description.push_back({ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, vertex_offset, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
			vertex_offset += 4 * sizeof(uint8);
			break;

		case EInputDescription::Normal:
			input_description.push_back({ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, vertex_offset, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
			vertex_offset += 3 * sizeof(float);
			break;

		case EInputDescription::TexCoord:
			input_description.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, vertex_offset, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
			vertex_offset += 2 * sizeof(float);
			break;

		case EInputDescription::InstanceColor:
			input_description.push_back({ "INSTANCE_COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 1, instance_offset, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 });
			instance_offset += 4 * sizeof(uint8);
			break;

		case EInputDescription::InstanceTransform:
		{
			for (uint j = 0; j < 4; ++j)
			{
				input_description.push_back({ "INSTANCE_TRANSFORM", j, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, instance_offset, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 });
				instance_offset += 4 * sizeof(float);
			}
			break;
		}

		case EInputDescription::InstanceInvTransform:
		{
			for (uint j = 0; j < 4; ++j)
			{
				input_description.push_back({ "INSTANCE_INV_TRANSFORM", j, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, instance_offset, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 });
				instance_offset += 4 * sizeof(float);
			}
			break;
		}
	}

	D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc = {};
	pso_desc.InputLayout = { input_description.data(), (UINT)input_description.size() };
	pso_desc.pRootSignature = mRenderer->GetRootSignature();
	pso_desc.VS = { inVertexShader->mShader->GetBufferPointer(), inVertexShader->mShader->GetBufferSize() };
	pso_desc.PS = { inPixelShader->mShader->GetBufferPointer(), inPixelShader->mShader->GetBufferSize() };

	pso_desc.RasterizerState.FillMode = inFillMode == EFillMode::Solid? D3D12_FILL_MODE_SOLID : D3D12_FILL_MODE_WIREFRAME;
	pso_desc.RasterizerState.CullMode = inCullMode == ECullMode::Backface? D3D12_CULL_MODE_FRONT : D3D12_CULL_MODE_BACK; // DX uses left handed system so we reverse the options
	pso_desc.RasterizerState.FrontCounterClockwise = FALSE;
	pso_desc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	pso_desc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	pso_desc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	pso_desc.RasterizerState.DepthClipEnable = TRUE;
	pso_desc.RasterizerState.MultisampleEnable = FALSE;
	pso_desc.RasterizerState.AntialiasedLineEnable = FALSE;
	pso_desc.RasterizerState.ForcedSampleCount = 0;
	pso_desc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	pso_desc.BlendState.AlphaToCoverageEnable = FALSE;
	pso_desc.BlendState.IndependentBlendEnable = FALSE;

	D3D12_RENDER_TARGET_BLEND_DESC &blend_desc = pso_desc.BlendState.RenderTarget[0];
	blend_desc.LogicOpEnable = FALSE;
	blend_desc.LogicOp = D3D12_LOGIC_OP_NOOP;
	blend_desc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	switch (inBlendMode)
	{
	case EBlendMode::Write:
		blend_desc.BlendEnable = FALSE;
		break;

	case EBlendMode::AlphaBlend:
		blend_desc.BlendEnable = TRUE;
		blend_desc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blend_desc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		blend_desc.BlendOp = D3D12_BLEND_OP_ADD;
		blend_desc.SrcBlendAlpha = D3D12_BLEND_ZERO;
		blend_desc.DestBlendAlpha = D3D12_BLEND_ZERO;
		blend_desc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		break;
	}

	pso_desc.DepthStencilState.DepthEnable = inDepthTest == EDepthTest::On? TRUE : FALSE;
	pso_desc.DepthStencilState.DepthWriteMask = inDepthTest == EDepthTest::On? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
	pso_desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_GREATER;
	pso_desc.DepthStencilState.StencilEnable = FALSE;

	pso_desc.SampleMask = UINT_MAX;
	pso_desc.PrimitiveTopologyType = topology;
	pso_desc.NumRenderTargets = 1;
	pso_desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	pso_desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	pso_desc.SampleDesc.Count = 1;

	FatalErrorIfFailed(mRenderer->GetDevice()->CreateGraphicsPipelineState(&pso_desc, IID_PPV_ARGS(&mPSO)));
}

PipelineStateDX12::~PipelineStateDX12()
{
	if (mPSO != nullptr)
		mRenderer->RecycleD3DObject(mPSO.Get());
}

void PipelineStateDX12::Activate()
{
	mRenderer->GetCommandList()->SetPipelineState(mPSO.Get());
}
