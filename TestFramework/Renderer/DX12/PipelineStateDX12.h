// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Renderer/PipelineState.h>

class RendererDX12;
class VertexShaderDX12;
class PixelShaderDX12;

/// DirectX 12 pipeline state object
class PipelineStateDX12 : public PipelineState
{
public:
	/// Constructor
										PipelineStateDX12(RendererDX12 *inRenderer, const VertexShaderDX12 *inVertexShader, const EInputDescription *inInputDescription, uint inInputDescriptionCount, const PixelShaderDX12 *inPixelShader, EDrawPass inDrawPass, EFillMode inFillMode, ETopology inTopology, EDepthTest inDepthTest, EBlendMode inBlendMode, ECullMode inCullMode);
	virtual								~PipelineStateDX12() override;

	/// Make this pipeline state active (any primitives rendered after this will use this state)
	virtual void						Activate() override;

private:
	RendererDX12 *						mRenderer;
	ComPtr<ID3D12PipelineState>			mPSO;
};
