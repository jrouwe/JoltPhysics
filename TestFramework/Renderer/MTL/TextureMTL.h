// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Renderer/Texture.h>

#include <MetalKit/MetalKit.h>

class RendererMTL;

/// Metal texture
class TextureMTL : public Texture
{
public:
	/// Constructor, called by Renderer::CreateTextureMTL
										TextureMTL(RendererMTL *inRenderer, const Surface *inSurface);	// Create a normal Texture
										TextureMTL(RendererMTL *inRenderer, int inWidth, int inHeight);	// Create a render target (depth only)
	virtual								~TextureMTL() override;

	/// Bind texture to the pixel shader
	virtual void						Bind() const override;

	/// Access to the metal texture
	id<MTLTexture>						GetTexture() const					{ return mTexture; }

private:
	RendererMTL *						mRenderer;
	id<MTLTexture> 						mTexture;
};

