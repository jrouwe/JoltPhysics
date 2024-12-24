// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Renderer/PixelShader.h>

/// Pixel shader handle for DirectX
class PixelShaderDX12 : public PixelShader
{
public:
	/// Constructor
							PixelShaderDX12(ComPtr<ID3DBlob> inShader)		: mShader(inShader) { }

	ComPtr<ID3DBlob>		mShader;										///< The compiled shader
};
