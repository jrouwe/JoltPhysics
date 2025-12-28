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
									ComputeBufferMTL(ComputeSystemMTL *inComputeSystem, EType inType, uint64 inSize, uint inStride, const void *inData);
	virtual							~ComputeBufferMTL() override;

	virtual void					Unmap() override;

	virtual Ref<ComputeBuffer>		CreateReadBackBuffer() const override;

	id<MTLBuffer>					GetBuffer() const							{ return mBuffer; }

private:
	virtual void *					MapInternal(EMode inMode) override;

	id<MTLBuffer>					mBuffer;
};

JPH_NAMESPACE_END

#endif // JPH_USE_MTL
