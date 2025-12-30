// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#ifdef JPH_USE_DX12

#include <Jolt/Compute/DX12/ComputeBufferDX12.h>
#include <Jolt/Compute/DX12/ComputeSystemDX12.h>

JPH_NAMESPACE_BEGIN

ComputeBufferDX12::ComputeBufferDX12(ComputeSystemDX12 *inComputeSystem, EType inType, uint64 inSize, uint inStride) :
	ComputeBuffer(inType, inSize, inStride),
	mComputeSystem(inComputeSystem)
{
}

bool ComputeBufferDX12::Initialize(const void *inData)
{
	uint64 buffer_size = mSize * mStride;

	switch (mType)
	{
	case EType::UploadBuffer:
		mBufferCPU = mComputeSystem->CreateD3DResource(D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_FLAG_NONE, buffer_size);
		mBufferGPU = mComputeSystem->CreateD3DResource(D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_FLAG_NONE, buffer_size);
		if (mBufferCPU == nullptr || mBufferGPU == nullptr)
			return false;
		break;

	case EType::ConstantBuffer:
		mBufferCPU = mComputeSystem->CreateD3DResource(D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_FLAG_NONE, buffer_size);
		if (mBufferCPU == nullptr)
			return false;
		break;

	case EType::ReadbackBuffer:
		JPH_ASSERT(inData == nullptr, "Can't upload data to a readback buffer");
		mBufferCPU = mComputeSystem->CreateD3DResource(D3D12_HEAP_TYPE_READBACK, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_FLAG_NONE, buffer_size);
		if (mBufferCPU == nullptr)
			return false;
		break;

	case EType::Buffer:
		JPH_ASSERT(inData != nullptr);
		mBufferCPU = mComputeSystem->CreateD3DResource(D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_FLAG_NONE, buffer_size);
		mBufferGPU = mComputeSystem->CreateD3DResource(D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_FLAG_NONE, buffer_size);
		if (mBufferCPU == nullptr || mBufferGPU == nullptr)
			return false;
		mNeedsSync = true;
		break;

	case EType::RWBuffer:
		if (inData != nullptr)
		{
			mBufferCPU = mComputeSystem->CreateD3DResource(D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_FLAG_NONE, buffer_size);
			if (mBufferCPU == nullptr)
				return false;
			mNeedsSync = true;
		}
		mBufferGPU = mComputeSystem->CreateD3DResource(D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, buffer_size);
		if (mBufferGPU == nullptr)
			return false;
		break;
	}

	// Copy data to upload buffer
	if (inData != nullptr)
	{
		void *data = nullptr;
		D3D12_RANGE range = { 0, 0 }; // We're not going to read
		mBufferCPU->Map(0, &range, &data);
		memcpy(data, inData, size_t(buffer_size));
		mBufferCPU->Unmap(0, nullptr);
	}

	return true;
}

bool ComputeBufferDX12::Barrier(ID3D12GraphicsCommandList *inCommandList, D3D12_RESOURCE_STATES inTo) const
{
	// Check if state changed
	if (mCurrentState == inTo)
		return false;

	// Only buffers in GPU memory can change state
	if (mType != ComputeBuffer::EType::Buffer && mType != ComputeBuffer::EType::RWBuffer)
		return true;

	D3D12_RESOURCE_BARRIER barrier;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = GetResourceGPU();
	barrier.Transition.StateBefore = mCurrentState;
	barrier.Transition.StateAfter = inTo;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	inCommandList->ResourceBarrier(1, &barrier);

	mCurrentState = inTo;
	return true;
}

void ComputeBufferDX12::RWBarrier(ID3D12GraphicsCommandList *inCommandList)
{
	JPH_ASSERT(mCurrentState == D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	D3D12_RESOURCE_BARRIER barrier;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = GetResourceGPU();
	inCommandList->ResourceBarrier(1, &barrier);
}

bool ComputeBufferDX12::SyncCPUToGPU(ID3D12GraphicsCommandList *inCommandList) const
{
	if (!mNeedsSync)
		return false;

	Barrier(inCommandList, D3D12_RESOURCE_STATE_COPY_DEST);

	inCommandList->CopyResource(GetResourceGPU(), GetResourceCPU());

	mNeedsSync = false;
	return true;
}

void *ComputeBufferDX12::MapInternal(EMode inMode)
{
	void *mapped_resource = nullptr;

	switch (inMode)
	{
	case EMode::Read:
		JPH_ASSERT(mType == EType::ReadbackBuffer);
		if (HRFailed(mBufferCPU->Map(0, nullptr, &mapped_resource)))
			return nullptr;
		break;

	case EMode::Write:
		{
			JPH_ASSERT(mType == EType::UploadBuffer || mType == EType::ConstantBuffer);
			D3D12_RANGE range = { 0, 0 }; // We're not going to read
			if (HRFailed(mBufferCPU->Map(0, &range, &mapped_resource)))
				return nullptr;
			mNeedsSync = true;
		}
		break;
	}

	return mapped_resource;
}

void ComputeBufferDX12::UnmapInternal()
{
	mBufferCPU->Unmap(0, nullptr);
}

ComputeBufferResult ComputeBufferDX12::CreateReadBackBuffer() const
{
	return mComputeSystem->CreateComputeBuffer(EType::ReadbackBuffer, mSize, mStride);
}

JPH_NAMESPACE_END

#endif // JPH_USE_DX12
