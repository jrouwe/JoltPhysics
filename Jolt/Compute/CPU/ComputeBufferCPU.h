// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Compute/ComputeBuffer.h>

#ifdef JPH_USE_CPU_COMPUTE

JPH_NAMESPACE_BEGIN

/// Buffer that can be used with the CPU compute system
class JPH_EXPORT ComputeBufferCPU final : public ComputeBuffer
{
public:
	JPH_OVERRIDE_NEW_DELETE

	/// Constructor / destructor
									ComputeBufferCPU(EType inType, uint64 inSize, uint inStride, const void *inData);
	virtual							~ComputeBufferCPU() override;

	ComputeBufferResult				CreateReadBackBuffer() const override;

	void *							GetData() const										{ return mData; }

private:
	virtual void *					MapInternal(EMode inMode) override					{ return mData; }
	virtual void					UnmapInternal() override							{ /* Nothing to do */ }

	void *							mData;
};

JPH_NAMESPACE_END

#endif // JPH_USE_CPU_COMPUTE
