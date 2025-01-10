// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Renderer/MTL/RendererMTL.h>
#include <Renderer/RenderInstances.h>

class RenderPrimitive;

/// Metal implementation of a render instances object
class RenderInstancesMTL : public RenderInstances
{
public:
	/// Constructor
							RenderInstancesMTL(RendererMTL *inRenderer)											: mRenderer(inRenderer) { }
	virtual					~RenderInstancesMTL() override														{ Clear(); }

	/// Erase all instance data
	virtual void			Clear() override;

	/// Instance buffer management functions
	virtual void			CreateBuffer(int inNumInstances, int inInstanceSize) override;
	virtual void *			Lock() override;
	virtual void			Unlock() override;

	/// Draw the instances when context has been set by Renderer::BindShader
	virtual void			Draw(RenderPrimitive *inPrimitive, int inStartInstance, int inNumInstances) const override;

private:
	RendererMTL *			mRenderer;
	id<MTLBuffer>			mBuffer;
	NSUInteger				mBufferSize;
	NSUInteger				mInstanceSize;
};
