// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Renderer/RenderPrimitive.h>
#include <Renderer/MTL/RendererMTL.h>

/// Metal implementation of a render primitive
class RenderPrimitiveMTL : public RenderPrimitive
{
public:
	/// Constructor
							RenderPrimitiveMTL(RendererMTL *inRenderer)										: mRenderer(inRenderer) { }
	virtual					~RenderPrimitiveMTL() override													{ Clear(); }

	/// Vertex buffer management functions
	virtual void			CreateVertexBuffer(int inNumVtx, int inVtxSize, const void *inData = nullptr) override;
	virtual void			ReleaseVertexBuffer() override;
	virtual void *			LockVertexBuffer() override;
	virtual void			UnlockVertexBuffer() override;

	/// Index buffer management functions
	virtual void			CreateIndexBuffer(int inNumIdx, const uint32 *inData = nullptr) override;
	virtual void			ReleaseIndexBuffer() override;
	virtual uint32 *		LockIndexBuffer() override;
	virtual void			UnlockIndexBuffer() override;

	/// Draw the primitive
	virtual void			Draw() const override;

private:
	RendererMTL *			mRenderer;
};
