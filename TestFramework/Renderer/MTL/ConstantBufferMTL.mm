// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Renderer/MTL/ConstantBufferMTL.h>
#include <Renderer/MTL/RendererMTL.h>
#include <Renderer/MTL/FatalErrorIfFailedMTL.h>

ConstantBufferMTL::ConstantBufferMTL(RendererMTL *inRenderer, uint inBufferSize)
{
}

ConstantBufferMTL::~ConstantBufferMTL()
{
}

void *ConstantBufferMTL::MapInternal()
{
	return nullptr;
}

void ConstantBufferMTL::Unmap()
{
}
