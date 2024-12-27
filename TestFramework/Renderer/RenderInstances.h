// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Core/Reference.h>

class RenderPrimitive;

/// Buffer that holds a list of instances (usually model transform etc.) for instance based rendering
class RenderInstances : public RefTarget<RenderInstances>
{
public:
	/// Destructor
	virtual					~RenderInstances() = default;

	/// Erase all instance data
	virtual void			Clear() = 0;

	/// Instance buffer management functions
	virtual void			CreateBuffer(int inNumInstances, int inInstanceSize) = 0;
	virtual void *			Lock() = 0;
	virtual void			Unlock() = 0;

	/// Draw the instances when context has been set by Renderer::BindShader
	virtual void			Draw(RenderPrimitive *inPrimitive, int inStartInstance, int inNumInstances) const = 0;
};
