// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Renderer/MTL/ConstantBufferMTL.h>
#include <Renderer/MTL/RendererMTL.h>

ConstantBufferMTL::ConstantBufferMTL(RendererMTL *inRenderer, uint inBufferSize)
{
	mBuffer = [inRenderer->GetDevice() newBufferWithLength: inBufferSize options: MTLResourceStorageModeShared];
}

ConstantBufferMTL::~ConstantBufferMTL()
{
}

void *ConstantBufferMTL::MapInternal()
{
	return mBuffer.contents;
}

void ConstantBufferMTL::Unmap()
{
}
