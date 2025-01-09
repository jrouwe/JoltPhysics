// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Renderer/MTL/RenderInstancesMTL.h>
#include <Renderer/MTL/RenderPrimitiveMTL.h>

void RenderInstancesMTL::Clear()
{
	mBuffer = nil;
}

void RenderInstancesMTL::CreateBuffer(int inNumInstances, int inInstanceSize)
{
	Clear();

	NSUInteger size = NSUInteger(inNumInstances) * inInstanceSize;
	mBuffer = [mRenderer->GetView().device newBufferWithLength: size options: MTLResourceStorageModeShared];
}

void *RenderInstancesMTL::Lock()
{
	return mBuffer.contents;
}

void RenderInstancesMTL::Unlock()
{
}

void RenderInstancesMTL::Draw(RenderPrimitive *inPrimitive, int inStartInstance, int inNumInstances) const
{
	if (inNumInstances <= 0)
		return;
}
