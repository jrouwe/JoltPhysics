// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Renderer/MTL/RenderPrimitiveMTL.h>

void RenderPrimitiveMTL::ReleaseVertexBuffer()
{
	[mVertexBuffer release];
	mVertexBuffer = nil;

	RenderPrimitive::ReleaseVertexBuffer();
}

void RenderPrimitiveMTL::ReleaseIndexBuffer()
{
	[mIndexBuffer release];
	mIndexBuffer = nil;

	RenderPrimitive::ReleaseIndexBuffer();
}

void RenderPrimitiveMTL::CreateVertexBuffer(int inNumVtx, int inVtxSize, const void *inData)
{
	RenderPrimitive::CreateVertexBuffer(inNumVtx, inVtxSize, inData);

	NSUInteger size = NSUInteger(inNumVtx) * inVtxSize;
	if (inData != nullptr)
		mVertexBuffer = [mRenderer->GetDevice() newBufferWithBytes: inData length: size options: MTLResourceCPUCacheModeDefaultCache | MTLResourceStorageModeShared | MTLResourceHazardTrackingModeTracked];
	else
		mVertexBuffer = [mRenderer->GetDevice() newBufferWithLength: size options: MTLResourceCPUCacheModeDefaultCache | MTLResourceStorageModeShared | MTLResourceHazardTrackingModeTracked];
}

void *RenderPrimitiveMTL::LockVertexBuffer()
{
	return mVertexBuffer.contents;
}

void RenderPrimitiveMTL::UnlockVertexBuffer()
{
}

void RenderPrimitiveMTL::CreateIndexBuffer(int inNumIdx, const uint32 *inData)
{
	RenderPrimitive::CreateIndexBuffer(inNumIdx, inData);

	NSUInteger size = NSUInteger(inNumIdx) * sizeof(uint32);
	if (inData != nullptr)
		mIndexBuffer = [mRenderer->GetDevice() newBufferWithBytes: inData length: size options: MTLResourceCPUCacheModeDefaultCache | MTLResourceStorageModeManaged | MTLResourceHazardTrackingModeTracked];
	else
		mIndexBuffer = [mRenderer->GetDevice() newBufferWithLength: size options: MTLResourceCPUCacheModeDefaultCache | MTLResourceStorageModeShared | MTLResourceHazardTrackingModeTracked];
}

uint32 *RenderPrimitiveMTL::LockIndexBuffer()
{
	return (uint32 *)mIndexBuffer.contents;
}

void RenderPrimitiveMTL::UnlockIndexBuffer()
{
}

void RenderPrimitiveMTL::Draw() const
{
	id<MTLRenderCommandEncoder> encoder = mRenderer->GetRenderEncoder();

	[encoder setVertexBuffer: mVertexBuffer offset: 0 atIndex: 0];
	if (mIndexBuffer == nil)
		[encoder drawPrimitives: mPrimitiveType vertexStart: 0 vertexCount: mNumVtxToDraw];
	else
		[encoder drawIndexedPrimitives: mPrimitiveType indexCount: mNumIdxToDraw indexType: MTLIndexTypeUInt32 indexBuffer: mIndexBuffer indexBufferOffset: 0];
}
