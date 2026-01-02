// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#ifdef JPH_USE_DX12

#include <Jolt/Compute/DX12/ComputeQueueDX12.h>
#include <Jolt/Compute/DX12/ComputeShaderDX12.h>
#include <Jolt/Compute/DX12/ComputeBufferDX12.h>

JPH_NAMESPACE_BEGIN

ComputeQueueDX12::~ComputeQueueDX12()
{
	Wait();

	if (mFenceEvent != INVALID_HANDLE_VALUE)
		CloseHandle(mFenceEvent);
}

bool ComputeQueueDX12::Initialize(ID3D12Device *inDevice, D3D12_COMMAND_LIST_TYPE inType, ComputeQueueResult &outResult)
{
	D3D12_COMMAND_QUEUE_DESC queue_desc = {};
	queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queue_desc.Type = inType;
	queue_desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH;
	if (HRFailed(inDevice->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&mCommandQueue)), outResult))
		return false;

	if (HRFailed(inDevice->CreateCommandAllocator(inType, IID_PPV_ARGS(&mCommandAllocator)), outResult))
		return false;

	// Create the command list
	if (HRFailed(inDevice->CreateCommandList(0, inType, mCommandAllocator.Get(), nullptr, IID_PPV_ARGS(&mCommandList)), outResult))
		return false;

	// Command lists are created in the recording state, but there is nothing to record yet. The main loop expects it to be closed, so close it now
	if (HRFailed(mCommandList->Close(), outResult))
		return false;

	// Create synchronization object
	if (HRFailed(inDevice->CreateFence(mFenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence)), outResult))
		return false;

	// Increment fence value so we don't skip waiting the first time a command list is executed
	mFenceValue++;

	// Create an event handle to use for frame synchronization
	mFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (HRFailed(HRESULT_FROM_WIN32(GetLastError()), outResult))
		return false;

	return true;
}

ID3D12GraphicsCommandList *ComputeQueueDX12::Start()
{
	JPH_ASSERT(!mIsExecuting);

	if (!mIsStarted)
	{
		// Reset the allocator
		if (HRFailed(mCommandAllocator->Reset()))
			return nullptr;

		// Reset the command list
		if (HRFailed(mCommandList->Reset(mCommandAllocator.Get(), nullptr)))
			return nullptr;

		// Now we have started recording commands
		mIsStarted = true;
	}

	return mCommandList.Get();
}

void ComputeQueueDX12::SetShader(const ComputeShader *inShader)
{
	ID3D12GraphicsCommandList *command_list = Start();
	mShader = static_cast<const ComputeShaderDX12 *>(inShader);
	command_list->SetPipelineState(mShader->GetPipelineState());
	command_list->SetComputeRootSignature(mShader->GetRootSignature());
}

void ComputeQueueDX12::SyncCPUToGPU(const ComputeBufferDX12 *inBuffer)
{
	// Ensure that any CPU writes are visible to the GPU
	if (inBuffer->SyncCPUToGPU(mCommandList.Get())
		&& (inBuffer->GetType() == ComputeBuffer::EType::Buffer || inBuffer->GetType() == ComputeBuffer::EType::RWBuffer))
	{
		// After the first upload, the CPU buffer is no longer needed for Buffer and RWBuffer types
		mDelayedFreedBuffers.emplace_back(inBuffer->ReleaseResourceCPU());
	}
}

void ComputeQueueDX12::SetConstantBuffer(const char *inName, const ComputeBuffer *inBuffer)
{
	if (inBuffer == nullptr)
		return;
	JPH_ASSERT(inBuffer->GetType() == ComputeBuffer::EType::ConstantBuffer);

	ID3D12GraphicsCommandList *command_list = Start();
	const ComputeBufferDX12 *buffer = static_cast<const ComputeBufferDX12 *>(inBuffer);
	command_list->SetComputeRootConstantBufferView(mShader->NameToIndex(inName), buffer->GetResourceCPU()->GetGPUVirtualAddress());

	mUsedBuffers.insert(buffer);
}

void ComputeQueueDX12::SetBuffer(const char *inName, const ComputeBuffer *inBuffer)
{
	if (inBuffer == nullptr)
		return;
	JPH_ASSERT(inBuffer->GetType() == ComputeBuffer::EType::UploadBuffer || inBuffer->GetType() == ComputeBuffer::EType::Buffer || inBuffer->GetType() == ComputeBuffer::EType::RWBuffer);

	ID3D12GraphicsCommandList *command_list = Start();
	const ComputeBufferDX12 *buffer = static_cast<const ComputeBufferDX12 *>(inBuffer);
	uint parameter_index = mShader->NameToIndex(inName);
	SyncCPUToGPU(buffer);
	buffer->Barrier(command_list, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	command_list->SetComputeRootShaderResourceView(parameter_index, buffer->GetResourceGPU()->GetGPUVirtualAddress());

	mUsedBuffers.insert(buffer);
}

void ComputeQueueDX12::SetRWBuffer(const char *inName, ComputeBuffer *inBuffer, EBarrier inBarrier)
{
	if (inBuffer == nullptr)
		return;
	JPH_ASSERT(inBuffer->GetType() == ComputeBuffer::EType::RWBuffer);

	ID3D12GraphicsCommandList *command_list = Start();
	ComputeBufferDX12 *buffer = static_cast<ComputeBufferDX12 *>(inBuffer);
	uint parameter_index = mShader->NameToIndex(inName);
	SyncCPUToGPU(buffer);
	if (!buffer->Barrier(command_list, D3D12_RESOURCE_STATE_UNORDERED_ACCESS) && inBarrier == EBarrier::Yes)
		buffer->RWBarrier(command_list);
	command_list->SetComputeRootUnorderedAccessView(parameter_index, buffer->GetResourceGPU()->GetGPUVirtualAddress());

	mUsedBuffers.insert(buffer);
}

void ComputeQueueDX12::ScheduleReadback(ComputeBuffer *inDst, const ComputeBuffer *inSrc)
{
	if (inDst == nullptr || inSrc == nullptr)
		return;
	JPH_ASSERT(inDst->GetType() == ComputeBuffer::EType::ReadbackBuffer);

	ID3D12GraphicsCommandList *command_list = Start();
	ComputeBufferDX12 *dst = static_cast<ComputeBufferDX12 *>(inDst);
	const ComputeBufferDX12 *src = static_cast<const ComputeBufferDX12 *>(inSrc);
	dst->Barrier(command_list, D3D12_RESOURCE_STATE_COPY_DEST);
	src->Barrier(command_list, D3D12_RESOURCE_STATE_COPY_SOURCE);
	command_list->CopyResource(dst->GetResourceCPU(), src->GetResourceGPU());

	mUsedBuffers.insert(src);
	mUsedBuffers.insert(dst);
}

void ComputeQueueDX12::Dispatch(uint inThreadGroupsX, uint inThreadGroupsY, uint inThreadGroupsZ)
{
	ID3D12GraphicsCommandList *command_list = Start();
	command_list->Dispatch(inThreadGroupsX, inThreadGroupsY, inThreadGroupsZ);
}

void ComputeQueueDX12::Execute()
{
	JPH_ASSERT(mIsStarted);
	JPH_ASSERT(!mIsExecuting);

	// Close the command list
	if (HRFailed(mCommandList->Close()))
		return;

	// Execute the command list
	ID3D12CommandList *command_lists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists((UINT)std::size(command_lists), command_lists);

	// Schedule a Signal command in the queue
	if (HRFailed(mCommandQueue->Signal(mFence.Get(), mFenceValue)))
		return;

	// Clear the current shader
	mShader = nullptr;

	// Mark that we're executing
	mIsExecuting = true;
}

void ComputeQueueDX12::Wait()
{
	// Check if we've been started
	if (mIsExecuting)
	{
		if (mFence->GetCompletedValue() < mFenceValue)
		{
			// Wait until the fence has been processed
			if (HRFailed(mFence->SetEventOnCompletion(mFenceValue, mFenceEvent)))
				return;
			WaitForSingleObjectEx(mFenceEvent, INFINITE, FALSE);
		}

		// Increment the fence value
		mFenceValue++;

		// Buffers can be freed now
		mUsedBuffers.clear();

		// Free buffers
		mDelayedFreedBuffers.clear();

		// Done executing
		mIsExecuting = false;
		mIsStarted = false;
	}
}

JPH_NAMESPACE_END

#endif // JPH_USE_DX12
