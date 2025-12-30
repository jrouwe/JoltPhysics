// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Compute/ComputeBuffer.h>

#ifdef JPH_USE_VK

#include <Jolt/Compute/VK/BufferVK.h>

JPH_NAMESPACE_BEGIN

class ComputeSystemVK;

/// Buffer that can be read from / written to by a compute shader
class JPH_EXPORT ComputeBufferVK final : public ComputeBuffer
{
public:
	JPH_OVERRIDE_NEW_DELETE

	/// Constructor
									ComputeBufferVK(ComputeSystemVK *inComputeSystem, EType inType, uint64 inSize, uint inStride);
	virtual							~ComputeBufferVK() override;

	bool							Initialize(const void *inData);

	virtual ComputeBufferResult		CreateReadBackBuffer() const override;

	VkBuffer						GetBufferCPU() const									{ return mBufferCPU.mBuffer; }
	VkBuffer						GetBufferGPU() const									{ return mBufferGPU.mBuffer; }
	BufferVK						ReleaseBufferCPU() const								{ BufferVK tmp = mBufferCPU; mBufferCPU = BufferVK(); return tmp; }

	void							Barrier(VkCommandBuffer inCommandBuffer, VkPipelineStageFlags inToStage, VkAccessFlagBits inToFlags, bool inForce) const;
	bool							SyncCPUToGPU(VkCommandBuffer inCommandBuffer) const;

private:
	virtual void *					MapInternal(EMode inMode) override;
	virtual void					UnmapInternal() override;

	ComputeSystemVK *				mComputeSystem;
	mutable BufferVK				mBufferCPU;
	BufferVK						mBufferGPU;
	mutable bool					mNeedsSync = false;										///< If this buffer needs to be synced from CPU to GPU
	mutable VkAccessFlagBits		mAccessFlagBits = VK_ACCESS_SHADER_READ_BIT;			///< Access flags of the last usage, used for barriers
	mutable VkPipelineStageFlags	mAccessStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;	///< Pipeline stage of the last usage, used for barriers
};

JPH_NAMESPACE_END

#endif // JPH_USE_VK
