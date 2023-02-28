// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

class Renderer;

/// Defines how primitives should be rendered
class PipelineState
{
public:
	/// If depth write / depth test is on
	enum class EDepthTest
	{
		Off,
		On
	};

	/// How to blend the pixel from the shader in the back buffer
	enum class EBlendMode
	{
		Write,
		AlphaBlend,
		AlphaTest,						///< Alpha blend with alpha test enabled
	};

	/// How to cull triangles
	enum class ECullMode
	{
		Backface,
		FrontFace,
	};

	/// Constructor
										PipelineState(Renderer *inRenderer, ID3DBlob *inVertexShader, const D3D12_INPUT_ELEMENT_DESC *inInputDescription, uint inInputDescriptionCount, ID3DBlob *inPixelShader, D3D12_FILL_MODE inFillMode, D3D12_PRIMITIVE_TOPOLOGY_TYPE inTopology, EDepthTest inDepthTest, EBlendMode inBlendMode, ECullMode inCullMode);
										~PipelineState();

	/// Make this pipeline state active (any primitives rendered after this will use this state)
	void								Activate();

private:
	friend class Renderer;

	Renderer *							mRenderer;
    ComPtr<ID3D12PipelineState>			mPSO;
};
