// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#ifdef JPH_USE_MTL

#include <Jolt/Compute/MTL/ComputeQueueMTL.h>
#include <Jolt/Compute/MTL/ComputeShaderMTL.h>
#include <Jolt/Compute/MTL/ComputeBufferMTL.h>
#include <Jolt/Compute/MTL/ComputeSystemMTL.h>

JPH_NAMESPACE_BEGIN

ComputeQueueMTL::~ComputeQueueMTL()
{
	Wait();

	[mCommandQueue release];
}

ComputeQueueMTL::ComputeQueueMTL(id<MTLDevice> inDevice)
{
	// Create the command queue
	mCommandQueue = [inDevice newCommandQueue];
}

void ComputeQueueMTL::BeginCommandBuffer()
{
	if (mCommandBuffer == nil)
	{
		// Start a new command buffer
		mCommandBuffer = [mCommandQueue commandBuffer];
		mComputeEncoder = [mCommandBuffer computeCommandEncoder];
	}
}

void ComputeQueueMTL::SetShader(const ComputeShader *inShader)
{
	BeginCommandBuffer();

	mShader = static_cast<const ComputeShaderMTL *>(inShader);

	[mComputeEncoder setComputePipelineState: mShader->GetPipelineState()];
}

void ComputeQueueMTL::SetConstantBuffer(const char *inName, const ComputeBuffer *inBuffer)
{
	if (inBuffer == nullptr)
		return;
	JPH_ASSERT(inBuffer->GetType() == ComputeBuffer::EType::ConstantBuffer);

	BeginCommandBuffer();

	const ComputeBufferMTL *buffer = static_cast<const ComputeBufferMTL *>(inBuffer);
	[mComputeEncoder setBuffer: buffer->GetBuffer() offset: 0 atIndex: mShader->NameToBindingIndex(inName)];
}

void ComputeQueueMTL::SetBuffer(const char *inName, const ComputeBuffer *inBuffer)
{
	if (inBuffer == nullptr)
		return;
	JPH_ASSERT(inBuffer->GetType() == ComputeBuffer::EType::UploadBuffer || inBuffer->GetType() == ComputeBuffer::EType::Buffer || inBuffer->GetType() == ComputeBuffer::EType::RWBuffer);

	BeginCommandBuffer();

	const ComputeBufferMTL *buffer = static_cast<const ComputeBufferMTL *>(inBuffer);
	[mComputeEncoder setBuffer: buffer->GetBuffer() offset: 0 atIndex: mShader->NameToBindingIndex(inName)];
}

void ComputeQueueMTL::SetRWBuffer(const char *inName, ComputeBuffer *inBuffer, EBarrier inBarrier)
{
	if (inBuffer == nullptr)
		return;
	JPH_ASSERT(inBuffer->GetType() == ComputeBuffer::EType::RWBuffer);

	BeginCommandBuffer();

	const ComputeBufferMTL *buffer = static_cast<const ComputeBufferMTL *>(inBuffer);
	[mComputeEncoder setBuffer: buffer->GetBuffer() offset: 0 atIndex: mShader->NameToBindingIndex(inName)];
}

void ComputeQueueMTL::ScheduleReadback(ComputeBuffer *inDst, const ComputeBuffer *inSrc)
{
	JPH_ASSERT(inDst == inSrc); // Since ComputeBuffer::CreateReadBackBuffer returns the same buffer, we don't need to copy
}

void ComputeQueueMTL::Dispatch(uint inThreadGroupsX, uint inThreadGroupsY, uint inThreadGroupsZ)
{
	BeginCommandBuffer();

	MTLSize thread_groups = MTLSizeMake(inThreadGroupsX, inThreadGroupsY, inThreadGroupsZ);
	MTLSize group_size = MTLSizeMake(mShader->GetGroupSizeX(), mShader->GetGroupSizeY(), mShader->GetGroupSizeZ());
	[mComputeEncoder dispatchThreadgroups: thread_groups threadsPerThreadgroup: group_size];
}

void ComputeQueueMTL::Execute()
{
	// End command buffer
	if (mCommandBuffer == nil)
		return;

	[mComputeEncoder endEncoding];
	[mCommandBuffer commit];
	mShader = nullptr;
	mIsExecuting = true;
}

void ComputeQueueMTL::Wait()
{
	if (!mIsExecuting)
		return;

	[mCommandBuffer waitUntilCompleted];
	mComputeEncoder = nil;
	mCommandBuffer = nil;
	mIsExecuting = false;
}

JPH_NAMESPACE_END

#endif // JPH_USE_MTL
