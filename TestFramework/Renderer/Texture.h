// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Core/Reference.h>

/// Forward declares
class Renderer;
class Surface;

class Texture : public RefTarget<Texture>
{
public:
	/// Constructor, called by Renderer::CreateTexture
										Texture(Renderer *inRenderer, const Surface *inSurface);	// Create a normal texture
										Texture(Renderer *inRenderer, int inWidth, int inHeight);	// Create a render target (depth only)
										~Texture();

	/// Get dimensions of texture
	inline int							GetWidth() const		{ return mWidth; }
	inline int							GetHeight() const		{ return mHeight; }

	/// Bind texture to the pixel shader
	void								Bind(int inSlot) const;

	/// Clear this texture (only possible for render targets)
	void								ClearRenderTarget();

	/// Activate this texture as the current render target, called by Renderer::SetRenderTarget
	void								SetAsRenderTarget(bool inSet) const;

private:
	Renderer *							mRenderer;

	int									mWidth;
	int									mHeight;

	ComPtr<ID3D12Resource>				mTexture;				///< The texture data

	D3D12_CPU_DESCRIPTOR_HANDLE			mSRV { 0 };				///< Shader resource view to bind as texture
	D3D12_CPU_DESCRIPTOR_HANDLE			mDSV { 0 };				///< Depth shader view to bind as render target
};
