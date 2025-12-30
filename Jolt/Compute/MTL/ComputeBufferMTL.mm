// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#ifdef JPH_USE_MTL

#include <Jolt/Compute/MTL/ComputeBufferMTL.h>

JPH_NAMESPACE_BEGIN

ComputeBufferMTL::ComputeBufferMTL(ComputeSystemMTL *inComputeSystem, EType inType, uint64 inSize, uint inStride) :
	ComputeBuffer(inType, inSize, inStride),
	mComputeSystem(inComputeSystem)
{
}

bool ComputeBufferMTL::Initialize(const void *inData)
{
	NSUInteger size = NSUInteger(mSize) * mStride;
	if (inData != nullptr)
		mBuffer = [mComputeSystem->GetDevice() newBufferWithBytes: inData length: size options: MTLResourceCPUCacheModeDefaultCache | MTLResourceStorageModeShared | MTLResourceHazardTrackingModeTracked];
	else
		mBuffer = [mComputeSystem->GetDevice() newBufferWithLength: size options: MTLResourceCPUCacheModeDefaultCache | MTLResourceStorageModeShared | MTLResourceHazardTrackingModeTracked];
	return mBuffer != nil;
}

ComputeBufferMTL::~ComputeBufferMTL()
{
	[mBuffer release];
}

void *ComputeBufferMTL::MapInternal(EMode inMode)
{
	return mBuffer.contents;
}

void ComputeBufferMTL::UnmapInternal()
{
}

ComputeBufferResult ComputeBufferMTL::CreateReadBackBuffer() const
{
	ComputeBufferResult result;
	result.Set(const_cast<ComputeBufferMTL *>(this));
	return result;
}

JPH_NAMESPACE_END

#endif // JPH_USE_MTL
