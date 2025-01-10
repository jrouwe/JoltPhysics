// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Renderer/PixelShader.h>

#include <MetalKit/MetalKit.h>

/// Pixel shader handle for Metal
class PixelShaderMTL : public PixelShader
{
public:
	/// Constructor
							PixelShaderMTL(id<MTLFunction> inFunction) : mFunction(inFunction) { }
	virtual					~PixelShaderMTL() override { [mFunction release]; }

	/// Access to the function
	id<MTLFunction>			GetFunction() const				{ return mFunction; }

private:
	id<MTLFunction>			mFunction;
};
