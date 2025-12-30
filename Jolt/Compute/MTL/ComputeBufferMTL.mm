// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#ifdef JPH_USE_MTL

#include <Jolt/Compute/MTL/ComputeBufferMTL.h>

JPH_NAMESPACE_BEGIN

ComputeBufferMTL::ComputeBufferMTL(ComputeSystemMTL *inComputeSystem, EType inType, uint64 inSize, uint inStride, const void *inData) :
	ComputeBuffer(inType, inSize, inStride)
{
	NSUInteger size = NSUInteger(inSize) * inStride;
	if (inData != nullptr)
		mBuffer = [inComputeSystem->GetDevice() newBufferWithBytes: inData length: size options: MTLResourceCPUCacheModeDefaultCache | MTLResourceStorageModeShared | MTLResourceHazardTrackingModeTracked];
	else
		mBuffer = [inComputeSystem->GetDevice() newBufferWithLength: size options: MTLResourceCPUCacheModeDefaultCache | MTLResourceStorageModeShared | MTLResourceHazardTrackingModeTracked];
}

ComputeBufferMTL::~ComputeBufferMTL()
{
	[mBuffer release];
}

void *ComputeBufferMTL::MapInternal(EMode inMode)
{
	return mBuffer.contents;
}

void ComputeBufferMTL::Unmap()
{
}

Ref<ComputeBuffer> ComputeBufferMTL::CreateReadBackBuffer() const
{
	return const_cast<ComputeBufferMTL *>(this);
}

JPH_NAMESPACE_END

#endif // JPH_USE_MTL
