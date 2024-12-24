// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

/// DirectX descriptor heap, used to allocate handles for resources to bind them to shaders
class DescriptorHeapDX12
{
public:
	/// Initialize the heap
	/// @param inDevice The DirectX device
	/// @param inType Type of heap
	/// @param inFlags Flags for the heap
	/// @param inNumber Number of handles to reserve
	void								Init(ID3D12Device *inDevice, D3D12_DESCRIPTOR_HEAP_TYPE inType, D3D12_DESCRIPTOR_HEAP_FLAGS inFlags, uint inNumber)
	{
		// Create the heap
		D3D12_DESCRIPTOR_HEAP_DESC heap_desc = {};
		heap_desc.NumDescriptors = inNumber;
		heap_desc.Type = inType;
		heap_desc.Flags = inFlags;
		FatalErrorIfFailed(inDevice->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(&mHeap)));

		// Delta between descriptor elements
		mDescriptorSize = inDevice->GetDescriptorHandleIncrementSize(heap_desc.Type);

		// Delta between the CPU and GPU heap
		if (inFlags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
			mGPUOffset = mHeap->GetGPUDescriptorHandleForHeapStart().ptr - mHeap->GetCPUDescriptorHandleForHeapStart().ptr;

		// Populate the freelist
		mFreeList.reserve(inNumber);
		for (uint i = 0; i < inNumber; ++i)
			mFreeList.push_back(i);
	}

	/// Allocate and return a new handle
	D3D12_CPU_DESCRIPTOR_HANDLE			Allocate()
	{
		JPH_ASSERT(!mFreeList.empty());

		D3D12_CPU_DESCRIPTOR_HANDLE handle = mHeap->GetCPUDescriptorHandleForHeapStart();

		uint index = mFreeList.back();
		mFreeList.pop_back();

		handle.ptr += index * mDescriptorSize;
		return handle;
	}

	/// Free a handle and return it to the freelist
	void								Free(D3D12_CPU_DESCRIPTOR_HANDLE inHandle)
	{
		uint index = uint((inHandle.ptr - mHeap->GetCPUDescriptorHandleForHeapStart().ptr) / mDescriptorSize);

		mFreeList.push_back(index);
	}

	/// Convert from a CPU to a GPU handle
	D3D12_GPU_DESCRIPTOR_HANDLE			ConvertToGPUHandle(D3D12_CPU_DESCRIPTOR_HANDLE inHandle)
	{
		JPH_ASSERT(mGPUOffset != -1);
		return { UINT64(inHandle.ptr) + mGPUOffset };
	}

	/// Access to the underlying DirectX structure
	ID3D12DescriptorHeap *				Get()
	{
		return mHeap.Get();
	}

private:
	ComPtr<ID3D12DescriptorHeap>		mHeap;
	uint								mDescriptorSize;				///< The size (in bytes) of a single heap descriptor
	Array<uint>							mFreeList;						///< List of indices in the heap that are still free
	INT64								mGPUOffset = -1;				///< Offset between CPU and GPU handles
};
