// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#ifdef JPH_USE_MTL

#include <MetalKit/MetalKit.h>

#include <Jolt/Compute/ComputeQueue.h>

JPH_NAMESPACE_BEGIN

class ComputeShaderMTL;

/// A command queue for Metal for executing compute workloads on the GPU.
class JPH_EXPORT ComputeQueueMTL final : public ComputeQueue
{
public:
	JPH_OVERRIDE_NEW_DELETE

	/// Constructor / destructor
										ComputeQueueMTL(id<MTLDevice> inDevice);
	virtual								~ComputeQueueMTL() override;

	// See: ComputeQueue
	virtual void						SetShader(const ComputeShader *inShader) override;
	virtual void						SetConstantBuffer(const char *inName, const ComputeBuffer *inBuffer) override;
	virtual void						SetBuffer(const char *inName, const ComputeBuffer *inBuffer) override;
	virtual void 						SetRWBuffer(const char *inName, ComputeBuffer *inBuffer, EBarrier inBarrier = EBarrier::Yes) override;
	virtual void						ScheduleReadback(ComputeBuffer *inDst, const ComputeBuffer *inSrc) override;
	virtual void						Dispatch(uint inThreadGroupsX, uint inThreadGroupsY, uint inThreadGroupsZ) override;
	virtual void						Execute() override;
	virtual void						Wait() override;

private:
	void								BeginCommandBuffer();

	id<MTLCommandQueue>					mCommandQueue;
	id<MTLCommandBuffer> 				mCommandBuffer;
	id<MTLComputeCommandEncoder>		mComputeEncoder;
	RefConst<ComputeShaderMTL>			mShader;
	bool								mIsExecuting = false;
};

JPH_NAMESPACE_END

#endif // JPH_USE_MTL
