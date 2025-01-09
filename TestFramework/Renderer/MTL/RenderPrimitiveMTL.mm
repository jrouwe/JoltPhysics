// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Renderer/MTL/RenderPrimitiveMTL.h>
#include <Renderer/MTL/FatalErrorIfFailedMTL.h>

void RenderPrimitiveMTL::ReleaseVertexBuffer()
{
	mVertexBuffer = nil;

	RenderPrimitive::ReleaseVertexBuffer();
}

void RenderPrimitiveMTL::ReleaseIndexBuffer()
{
	mIndexBuffer = nil;

	RenderPrimitive::ReleaseIndexBuffer();
}

void RenderPrimitiveMTL::CreateVertexBuffer(int inNumVtx, int inVtxSize, const void *inData)
{
	RenderPrimitive::CreateVertexBuffer(inNumVtx, inVtxSize, inData);

	NSUInteger size = NSUInteger(inNumVtx) * inVtxSize;
	mVertexBuffer = [mRenderer->GetDevice() newBufferWithLength: size options: MTLResourceStorageModeShared];
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
	mIndexBuffer = [mRenderer->GetDevice() newBufferWithLength: size options: MTLResourceStorageModeShared];
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
}
