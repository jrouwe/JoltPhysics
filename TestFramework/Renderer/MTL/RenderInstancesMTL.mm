// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Renderer/MTL/RenderInstancesMTL.h>
#include <Renderer/MTL/RenderPrimitiveMTL.h>
#include <Renderer/MTL/FatalErrorIfFailedMTL.h>

void RenderInstancesMTL::Clear()
{
}

void RenderInstancesMTL::CreateBuffer(int inNumInstances, int inInstanceSize)
{
	Clear();
}

void *RenderInstancesMTL::Lock()
{
	return nullptr;
}

void RenderInstancesMTL::Unlock()
{
}

void RenderInstancesMTL::Draw(RenderPrimitive *inPrimitive, int inStartInstance, int inNumInstances) const
{
	if (inNumInstances <= 0)
		return;

	(void)mRenderer;
}
