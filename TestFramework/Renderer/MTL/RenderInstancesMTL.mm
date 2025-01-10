// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Renderer/MTL/RenderInstancesMTL.h>
#include <Renderer/MTL/RenderPrimitiveMTL.h>

void RenderInstancesMTL::Clear()
{
	[mBuffer release];
	mBuffer = nil;
}

void RenderInstancesMTL::CreateBuffer(int inNumInstances, int inInstanceSize)
{
	mInstanceSize = NSUInteger(inInstanceSize);
	NSUInteger size = mInstanceSize * inNumInstances;
	if (mBuffer == nullptr || mBufferSize < size)
	{
		Clear();

		mBuffer = [mRenderer->GetView().device newBufferWithLength: size options: MTLResourceCPUCacheModeDefaultCache | MTLResourceStorageModeShared | MTLResourceHazardTrackingModeTracked];
		mBufferSize = size;
	}
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

	id<MTLRenderCommandEncoder> encoder = mRenderer->GetRenderEncoder();
	RenderPrimitiveMTL *prim = static_cast<RenderPrimitiveMTL *>(inPrimitive);

	[encoder setVertexBuffer: prim->mVertexBuffer offset: 0 atIndex: 0];
	[encoder setVertexBuffer: mBuffer offset: mInstanceSize * inStartInstance atIndex: 1];
	if (prim->mIndexBuffer == nil)
		[encoder drawPrimitives: prim->mPrimitiveType vertexStart: 0 vertexCount: prim->mNumVtxToDraw instanceCount: inNumInstances];
	else
		[encoder drawIndexedPrimitives: prim->mPrimitiveType indexCount: prim->mNumIdxToDraw indexType: MTLIndexTypeUInt32 indexBuffer: prim->mIndexBuffer indexBufferOffset: 0 instanceCount: inNumInstances];
}
