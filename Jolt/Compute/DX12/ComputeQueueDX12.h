// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#ifdef JPH_USE_DX12

#include <Jolt/Compute/ComputeQueue.h>
#include <Jolt/Compute/DX12/ComputeShaderDX12.h>
#include <Jolt/Core/UnorderedSet.h>

JPH_NAMESPACE_BEGIN

class ComputeBufferDX12;

/// A command queue for DirectX for executing compute workloads on the GPU.
class JPH_EXPORT ComputeQueueDX12 final : public ComputeQueue
{
public:
	JPH_OVERRIDE_NEW_DELETE

	/// Destructor
	virtual								~ComputeQueueDX12() override;

	/// Initialize the queue
	bool								Initialize(ID3D12Device *inDevice, D3D12_COMMAND_LIST_TYPE inType, ComputeQueueResult &outResult);

	/// Start the command list (requires waiting until the previous one is finished)
	ID3D12GraphicsCommandList *			Start();

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
	/// Copy the CPU buffer to the GPU buffer if needed
	void								SyncCPUToGPU(const ComputeBufferDX12 *inBuffer);

	ComPtr<ID3D12CommandQueue>			mCommandQueue;								///< The command queue that will hold command lists
	ComPtr<ID3D12CommandAllocator>		mCommandAllocator;							///< Allocator that holds the memory for the commands
	ComPtr<ID3D12GraphicsCommandList>	mCommandList;								///< The command list that will hold the render commands / state changes
	HANDLE								mFenceEvent = INVALID_HANDLE_VALUE;			///< Fence event, used to wait for rendering to complete
	ComPtr<ID3D12Fence>					mFence;										///< Fence object, used to signal the fence event
	UINT64								mFenceValue = 0;							///< Current fence value, each time we need to wait we will signal the fence with this value, wait for it and then increase the value
	RefConst<ComputeShaderDX12>			mShader = nullptr;							///< Current active shader
	bool								mIsStarted = false;							///< If the command list has been started (reset) and is ready to record commands
	bool								mIsExecuting = false;						///< If a command list is currently executing on the queue
	UnorderedSet<RefConst<ComputeBuffer>> mUsedBuffers;								///< Buffers that are in use by the current execution, these will be retained until execution is finished so that we don't free buffers that are in use
	Array<ComPtr<ID3D12Resource>>		mDelayedFreedBuffers;						///< Buffers freed during the execution
};

JPH_NAMESPACE_END

#endif // JPH_USE_DX12
