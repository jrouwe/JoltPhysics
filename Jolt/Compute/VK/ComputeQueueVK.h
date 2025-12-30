// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Compute/ComputeQueue.h>

#ifdef JPH_USE_VK

#include <Jolt/Compute/VK/ComputeShaderVK.h>
#include <Jolt/Compute/VK/BufferVK.h>
#include <Jolt/Core/UnorderedMap.h>
#include <Jolt/Core/UnorderedSet.h>

JPH_NAMESPACE_BEGIN

class ComputeSystemVK;
class ComputeBufferVK;

/// A command queue for Vulkan for executing compute workloads on the GPU.
class JPH_EXPORT ComputeQueueVK final : public ComputeQueue
{
public:
	JPH_OVERRIDE_NEW_DELETE

	/// Constructor / Destructor
	explicit							ComputeQueueVK(ComputeSystemVK *inComputeSystem) : mComputeSystem(inComputeSystem) { }
	virtual								~ComputeQueueVK() override;

	/// Initialize the queue
	bool								Initialize(uint32 inComputeQueueIndex, ComputeQueueResult &outResult);

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
	bool								BeginCommandBuffer();

	// Copy the CPU buffer to the GPU buffer if needed
	void								SyncCPUToGPU(const ComputeBufferVK *inBuffer);

	ComputeSystemVK *					mComputeSystem;
	VkQueue								mQueue = VK_NULL_HANDLE;
	VkCommandPool						mCommandPool = VK_NULL_HANDLE;
	VkDescriptorPool					mDescriptorPool = VK_NULL_HANDLE;
	VkCommandBuffer						mCommandBuffer = VK_NULL_HANDLE;
	bool								mCommandBufferRecording = false;				///< If we are currently recording commands into the command buffer
	VkFence								mFence = VK_NULL_HANDLE;
	bool								mIsExecuting = false;							///< If Execute has been called and we are waiting for it to finish
	RefConst<ComputeShaderVK>			mShader;										///< Shader that has been activated
	Array<VkDescriptorBufferInfo>		mBufferInfos;									///< List of parameters that will be sent to the current shader
	UnorderedSet<RefConst<ComputeBuffer>> mUsedBuffers;									///< Buffers that are in use by the current execution, these will be retained until execution is finished so that we don't free buffers that are in use
	Array<BufferVK>						mDelayedFreedBuffers;							///< Hardware buffers that need to be freed after execution is done
};

JPH_NAMESPACE_END

#endif // JPH_USE_VK
