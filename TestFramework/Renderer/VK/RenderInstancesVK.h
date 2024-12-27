// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Renderer/VK/RendererVK.h>
#include <Renderer/RenderInstances.h>

class RenderPrimitive;

/// Vulkan implementation of a render instances object
class RenderInstancesVK : public RenderInstances
{
public:
	/// Constructor
							RenderInstancesVK(RendererVK *inRenderer)											: mRenderer(inRenderer) { }
	virtual					~RenderInstancesVK() override														{ Clear(); }

	/// Erase all instance data
	virtual void			Clear() override;

	/// Instance buffer management functions
	virtual void			CreateBuffer(int inNumInstances, int inInstanceSize) override;
	virtual void *			Lock() override;
	virtual void			Unlock() override;

	/// Draw the instances when context has been set by Renderer::BindShader
	virtual void			Draw(RenderPrimitive *inPrimitive, int inStartInstance, int inNumInstances) const override;

private:
	RendererVK *			mRenderer;

	BufferVK				mInstancesBuffer;
};
