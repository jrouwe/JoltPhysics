// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#ifdef JPH_USE_CPU_COMPUTE

#include <Jolt/Compute/CPU/ComputeBufferCPU.h>

JPH_NAMESPACE_BEGIN

ComputeBufferCPU::ComputeBufferCPU(EType inType, uint64 inSize, uint inStride, const void *inData) :
	ComputeBuffer(inType, inSize, inStride)
{
	size_t buffer_size = size_t(mSize) * mStride;
	mData = Allocate(buffer_size);
	if (inData != nullptr)
		memcpy(mData, inData, buffer_size);
}

ComputeBufferCPU::~ComputeBufferCPU()
{
	Free(mData);
}

ComputeBufferResult ComputeBufferCPU::CreateReadBackBuffer() const
{
	ComputeBufferResult result;
	result.Set(const_cast<ComputeBufferCPU *>(this));
	return result;
}

JPH_NAMESPACE_END

#endif // JPH_USE_CPU_COMPUTE
