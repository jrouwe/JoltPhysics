// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Renderer/Renderer.h>
#include <Jolt/Core/Reference.h>

class RenderPrimitive;

/// Buffer that holds a list of instances (usually model transform etc.) for instance based rendering
class RenderInstances : public RefTarget<RenderInstances>
{
public:
	/// Constructor
							RenderInstances(Renderer *inRenderer)											: mRenderer(inRenderer) { }
							~RenderInstances()																{ Clear(); }

	/// Erase all instance data
	void					Clear();

	/// Instance buffer management functions
	void					CreateBuffer(int inNumInstances, int inInstanceSize);
	void *					Lock();
	void					Unlock();

	/// Draw the instances when context has been set by Renderer::BindShader
	void					Draw(RenderPrimitive *inPrimitive, int inStartInstance, int inNumInstances) const;

private:
	Renderer *				mRenderer;
	
	ComPtr<ID3D12Resource>	mInstanceBuffer;
	int						mInstanceBufferSize = 0;
	int						mInstanceSize = 0;
};
