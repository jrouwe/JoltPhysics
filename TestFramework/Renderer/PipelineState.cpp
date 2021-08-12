// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Renderer/PipelineState.h>
#include <Renderer/Renderer.h>
#include <Renderer/FatalErrorIfFailed.h>

PipelineState::PipelineState(Renderer *inRenderer, ID3DBlob *inVertexShader, const D3D12_INPUT_ELEMENT_DESC *inInputDescription, uint inInputDescriptionCount, ID3DBlob *inPixelShader, D3D12_FILL_MODE inFillMode, D3D12_PRIMITIVE_TOPOLOGY_TYPE inTopology, EDepthTest inDepthTest, EBlendMode inBlendMode, ECullMode inCullMode) :
	mRenderer(inRenderer)
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc = {};
    pso_desc.InputLayout = { inInputDescription, inInputDescriptionCount };
    pso_desc.pRootSignature = mRenderer->GetRootSignature();
    pso_desc.VS = { inVertexShader->GetBufferPointer(), inVertexShader->GetBufferSize() };
    pso_desc.PS = { inPixelShader->GetBufferPointer(), inPixelShader->GetBufferSize() };
    
    pso_desc.RasterizerState.FillMode = inFillMode;
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

	case EBlendMode::AlphaTest:
		pso_desc.BlendState.AlphaToCoverageEnable = TRUE;
		[[fallthrough]];

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
    pso_desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    pso_desc.DepthStencilState.StencilEnable = FALSE;

    pso_desc.SampleMask = UINT_MAX;
    pso_desc.PrimitiveTopologyType = inTopology;
    pso_desc.NumRenderTargets = 1;
    pso_desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    pso_desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    pso_desc.SampleDesc.Count = 1;

    FatalErrorIfFailed(mRenderer->GetDevice()->CreateGraphicsPipelineState(&pso_desc, IID_PPV_ARGS(&mPSO)));
}

PipelineState::~PipelineState()
{
	if (mPSO != nullptr)
		mRenderer->RecycleD3DObject(mPSO.Get());
}

void PipelineState::Activate()
{
    mRenderer->GetCommandList()->SetPipelineState(mPSO.Get());
}
