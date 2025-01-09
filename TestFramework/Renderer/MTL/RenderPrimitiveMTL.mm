// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Renderer/MTL/RenderPrimitiveMTL.h>
#include <Renderer/MTL/FatalErrorIfFailedMTL.h>

void RenderPrimitiveMTL::ReleaseVertexBuffer()
{
	RenderPrimitive::ReleaseVertexBuffer();
}

void RenderPrimitiveMTL::ReleaseIndexBuffer()
{
	RenderPrimitive::ReleaseIndexBuffer();
}

void RenderPrimitiveMTL::CreateVertexBuffer(int inNumVtx, int inVtxSize, const void *inData)
{
	RenderPrimitive::CreateVertexBuffer(inNumVtx, inVtxSize, inData);
}

void *RenderPrimitiveMTL::LockVertexBuffer()
{
	return nullptr;
}

void RenderPrimitiveMTL::UnlockVertexBuffer()
{
}

void RenderPrimitiveMTL::CreateIndexBuffer(int inNumIdx, const uint32 *inData)
{
	RenderPrimitive::CreateIndexBuffer(inNumIdx, inData);
}

uint32 *RenderPrimitiveMTL::LockIndexBuffer()
{
	return nullptr;
}

void RenderPrimitiveMTL::UnlockIndexBuffer()
{
}

void RenderPrimitiveMTL::Draw() const
{
	(void)mRenderer;
}
