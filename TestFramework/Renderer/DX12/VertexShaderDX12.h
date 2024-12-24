// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Renderer/VertexShader.h>

/// Vertex shader handle for DirectX
class VertexShaderDX12 : public VertexShader
{
public:
	/// Constructor
							VertexShaderDX12(ComPtr<ID3DBlob> inShader)		: mShader(inShader) { }

	ComPtr<ID3DBlob>		mShader;										///< The compiled shader
};
