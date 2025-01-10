// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Renderer/VertexShader.h>

#include <MetalKit/MetalKit.h>

/// Vertex shader handle for Metal
class VertexShaderMTL : public VertexShader
{
public:
	/// Constructor
							VertexShaderMTL(id<MTLFunction> inFunction) : mFunction(inFunction) { }
	virtual					~VertexShaderMTL() override { [mFunction release]; }

	/// Access to the function
	id<MTLFunction>			GetFunction() const				{ return mFunction; }

private:
	id<MTLFunction>			mFunction;
};
