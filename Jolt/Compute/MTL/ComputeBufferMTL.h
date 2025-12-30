// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#ifdef JPH_USE_MTL

#include <Jolt/Compute/MTL/ComputeSystemMTL.h>

JPH_NAMESPACE_BEGIN

/// Buffer that can be read from / written to by a compute shader
class JPH_EXPORT ComputeBufferMTL final : public ComputeBuffer
{
public:
	JPH_OVERRIDE_NEW_DELETE

	/// Constructor
									ComputeBufferMTL(ComputeSystemMTL *inComputeSystem, EType inType, uint64 inSize, uint inStride);
	virtual							~ComputeBufferMTL() override;

	bool							Initialize(const void *inData);

	virtual ComputeBufferResult		CreateReadBackBuffer() const override;

	id<MTLBuffer>					GetBuffer() const							{ return mBuffer; }

private:
	virtual void *					MapInternal(EMode inMode) override;
	virtual void					UnmapInternal() override;

	ComputeSystemMTL *				mComputeSystem;
	id<MTLBuffer>					mBuffer;
};

JPH_NAMESPACE_END

#endif // JPH_USE_MTL
