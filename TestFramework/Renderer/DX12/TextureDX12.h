// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Renderer/Texture.h>

class RendererDX12;

class TextureDX12 : public Texture
{
public:
	/// Constructor, called by Renderer::CreateTextureDX12
										TextureDX12(RendererDX12 *inRenderer, const Surface *inSurface);	// Create a normal TextureDX12
										TextureDX12(RendererDX12 *inRenderer, int inWidth, int inHeight);	// Create a render target (depth only)
	virtual								~TextureDX12() override;

	/// Bind texture to the pixel shader
	virtual void						Bind() const override;

	/// Activate this texture as the current render target, used by RendererDX12::BeginFrame, EndShadowPass
	void								SetAsRenderTarget(bool inSet) const;

private:
	RendererDX12 *						mRenderer;

	ComPtr<ID3D12Resource>				mTexture;				///< The texture data

	D3D12_CPU_DESCRIPTOR_HANDLE			mSRV { 0 };				///< Shader resource view to bind as texture
	D3D12_CPU_DESCRIPTOR_HANDLE			mDSV { 0 };				///< Depth shader view to bind as render target
};
