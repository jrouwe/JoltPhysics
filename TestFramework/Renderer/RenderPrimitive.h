// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Renderer/Renderer.h>
#include <Jolt/Core/Reference.h>

/// Simple wrapper around vertex and index buffers
class RenderPrimitive : public RefTarget<RenderPrimitive>
{
public:
	/// Constructor
							RenderPrimitive(Renderer *inRenderer, D3D_PRIMITIVE_TOPOLOGY inType)			: mRenderer(inRenderer), mType(inType) { }
							~RenderPrimitive()																{ Clear(); }

	/// Erase all primitive data
	void					Clear();

	/// Check if this primitive contains any data
	bool					IsEmpty() const																	{ return mNumVtx == 0 && mNumIdx == 0; }

	/// Vertex buffer management functions
	void					CreateVertexBuffer(int inNumVtx, int inVtxSize, const void *inData = nullptr);
	void					ReleaseVertexBuffer();
	void *					LockVertexBuffer();
	void					UnlockVertexBuffer();
	int						GetNumVtx() const																{ return mNumVtx; }
	int						GetNumVtxToDraw() const															{ return mNumVtxToDraw; }
	void					SetNumVtxToDraw(int inUsed)														{ mNumVtxToDraw = inUsed; }

	/// Index buffer management functions
	void					CreateIndexBuffer(int inNumIdx, const uint32 *inData = nullptr);
	void					ReleaseIndexBuffer();
	uint32 *				LockIndexBuffer();
	void					UnlockIndexBuffer();
	int						GetNumIdx() const																{ return mNumIdx; }
	int						GetNumIdxToDraw() const															{ return mNumIdxToDraw; }
	void					SetNumIdxToDraw(int inUsed)														{ mNumIdxToDraw = inUsed; }

	/// Draw the primitive
	void					Draw() const;

private:
	friend class RenderInstances;

	Renderer *				mRenderer;

	D3D_PRIMITIVE_TOPOLOGY	mType;

	ComPtr<ID3D12Resource>	mVtxBuffer;
	int						mNumVtx = 0;
	int						mNumVtxToDraw = 0;
	int						mVtxSize = 0;
	bool					mVtxBufferInUploadHeap = false;

	ComPtr<ID3D12Resource>	mIdxBuffer;
	int						mNumIdx = 0;
	int						mNumIdxToDraw = 0;
	bool					mIdxBufferInUploadHeap = false;
};
