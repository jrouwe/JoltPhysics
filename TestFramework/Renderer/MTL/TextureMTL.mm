// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Renderer/MTL/TextureMTL.h>
#include <Renderer/MTL/RendererMTL.h>
#include <Renderer/MTL/FatalErrorIfFailedMTL.h>
#include <Image/BlitSurface.h>

TextureMTL::TextureMTL(RendererMTL *inRenderer, const Surface *inSurface) :
	Texture(inSurface->GetWidth(), inSurface->GetHeight())
{
}

TextureMTL::TextureMTL(RendererMTL *inRenderer, int inWidth, int inHeight) :
	Texture(inWidth, inHeight)
{
}

TextureMTL::~TextureMTL()
{
}

void TextureMTL::Bind() const
{
}
