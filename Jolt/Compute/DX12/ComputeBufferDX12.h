// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Compute/ComputeBuffer.h>

#ifdef JPH_USE_DX12

#include <Jolt/Compute/DX12/IncludeDX12.h>

JPH_NAMESPACE_BEGIN

class ComputeSystemDX12;

/// Buffer that can be read from / written to by a compute shader
class JPH_EXPORT ComputeBufferDX12 final : public ComputeBuffer
{
public:
	JPH_OVERRIDE_NEW_DELETE

	/// Constructor
									ComputeBufferDX12(ComputeSystemDX12 *inComputeSystem, EType inType, uint64 inSize, uint inStride);

	bool							Initialize(const void *inData);

	ID3D12Resource *				GetResourceCPU() const									{ return mBufferCPU.Get(); }
	ID3D12Resource *				GetResourceGPU() const									{ return mBufferGPU.Get(); }
	ComPtr<ID3D12Resource>			ReleaseResourceCPU() const								{ return std::move(mBufferCPU); }

	bool							Barrier(ID3D12GraphicsCommandList *inCommandList, D3D12_RESOURCE_STATES inTo) const;
	void							RWBarrier(ID3D12GraphicsCommandList *inCommandList);
	bool							SyncCPUToGPU(ID3D12GraphicsCommandList *inCommandList) const;

	ComputeBufferResult				CreateReadBackBuffer() const override;

private:
	virtual void *					MapInternal(EMode inMode) override;
	virtual void					UnmapInternal() override;

	ComputeSystemDX12 *				mComputeSystem;
	mutable ComPtr<ID3D12Resource>	mBufferCPU;
	ComPtr<ID3D12Resource>			mBufferGPU;
	mutable bool					mNeedsSync = false;										///< If this buffer needs to be synced from CPU to GPU
	mutable D3D12_RESOURCE_STATES	mCurrentState = D3D12_RESOURCE_STATE_COPY_DEST;			///< State of the GPU buffer so we can do proper barriers
};

JPH_NAMESPACE_END

#endif // JPH_USE_DX12
