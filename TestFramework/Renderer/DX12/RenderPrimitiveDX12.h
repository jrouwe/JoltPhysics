// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Renderer/RenderPrimitive.h>
#include <Renderer/DX12/RendererDX12.h>

/// DirectX 12 implementation of a render primitive
class RenderPrimitiveDX12 : public RenderPrimitive
{
public:
	JPH_OVERRIDE_NEW_DELETE

	/// Constructor
							RenderPrimitiveDX12(RendererDX12 *inRenderer, PipelineState::ETopology inType)	: mRenderer(inRenderer), mType(inType) { }
	virtual					~RenderPrimitiveDX12() override													{ Clear(); }

	/// Vertex buffer management functions
	virtual void			CreateVertexBuffer(int inNumVtx, int inVtxSize, const void *inData = nullptr) override;
	virtual void			ReleaseVertexBuffer() override;
	virtual void *			LockVertexBuffer() override;
	virtual void			UnlockVertexBuffer() override;

	/// Index buffer management functions
	virtual void			CreateIndexBuffer(int inNumIdx, const uint32 *inData = nullptr) override;
	virtual void			ReleaseIndexBuffer() override;
	virtual uint32 *		LockIndexBuffer() override;
	virtual void			UnlockIndexBuffer() override;

	/// Draw the primitive
	virtual void			Draw() const override;

private:
	friend class RenderInstancesDX12;

	RendererDX12 *			mRenderer;

	PipelineState::ETopology mType;

	ComPtr<ID3D12Resource>	mVtxBuffer;
	bool					mVtxBufferInUploadHeap = false;

	ComPtr<ID3D12Resource>	mIdxBuffer;
	bool					mIdxBufferInUploadHeap = false;
};
