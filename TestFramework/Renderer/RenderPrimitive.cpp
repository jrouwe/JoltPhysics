// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Renderer/RenderPrimitive.h>

void RenderPrimitive::ReleaseVertexBuffer()
{
	mNumVtx = 0;
	mNumVtxToDraw = 0;
	mVtxSize = 0;
}

void RenderPrimitive::ReleaseIndexBuffer()
{
	mNumIdx = 0;
	mNumIdxToDraw = 0;
}

void RenderPrimitive::Clear()
{
	ReleaseVertexBuffer();
	ReleaseIndexBuffer();
}

void RenderPrimitive::CreateVertexBuffer(int inNumVtx, int inVtxSize, const void *inData)
{
	ReleaseVertexBuffer();

	mNumVtx = inNumVtx;
	mNumVtxToDraw = inNumVtx;
	mVtxSize = inVtxSize;
}

void RenderPrimitive::CreateIndexBuffer(int inNumIdx, const uint32 *inData)
{
	ReleaseIndexBuffer();

	mNumIdx = inNumIdx;
	mNumIdxToDraw = inNumIdx;
}
