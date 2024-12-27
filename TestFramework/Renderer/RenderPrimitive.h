// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Core/Reference.h>

/// Simple wrapper around vertex and index buffers
class RenderPrimitive : public RefTarget<RenderPrimitive>, public RefTargetVirtual
{
public:
	/// Destructor
	virtual 				~RenderPrimitive() override = default;

	/// Erase all primitive data
	void					Clear();

	/// Check if this primitive contains any data
	bool					IsEmpty() const																	{ return mNumVtx == 0 && mNumIdx == 0; }

	/// Vertex buffer management functions
	virtual void			CreateVertexBuffer(int inNumVtx, int inVtxSize, const void *inData = nullptr) = 0;
	virtual void			ReleaseVertexBuffer();
	virtual void *			LockVertexBuffer() = 0;
	virtual void			UnlockVertexBuffer() = 0;
	int						GetNumVtx() const																{ return mNumVtx; }
	int						GetNumVtxToDraw() const															{ return mNumVtxToDraw; }
	void					SetNumVtxToDraw(int inUsed)														{ mNumVtxToDraw = inUsed; }

	/// Index buffer management functions
	virtual void			CreateIndexBuffer(int inNumIdx, const uint32 *inData = nullptr) = 0;
	virtual void			ReleaseIndexBuffer();
	virtual uint32 *		LockIndexBuffer() = 0;
	virtual void			UnlockIndexBuffer() = 0;
	int						GetNumIdx() const																{ return mNumIdx; }
	int						GetNumIdxToDraw() const															{ return mNumIdxToDraw; }
	void					SetNumIdxToDraw(int inUsed)														{ mNumIdxToDraw = inUsed; }

	/// Draw the primitive
	virtual void			Draw() const = 0;

	/// Implement RefTargetVirtual, so we can conveniently use this class as DebugRenderer::Batch
	virtual void			AddRef() override																{ RefTarget<RenderPrimitive>::AddRef(); }
	virtual void			Release() override																{ RefTarget<RenderPrimitive>::Release(); }

protected:
	int						mNumVtx = 0;
	int						mNumVtxToDraw = 0;
	int						mVtxSize = 0;

	int						mNumIdx = 0;
	int						mNumIdxToDraw = 0;
};
