// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Core/UnorderedMap.h>
#include <Jolt/Compute/ComputeSystem.h>

#ifdef JPH_USE_DX12

#include <Jolt/Compute/DX12/IncludeDX12.h>

JPH_NAMESPACE_BEGIN

/// Interface to run a workload on the GPU using DirectX 12.
/// Minimal implementation that can integrate with your own DirectX 12 setup.
class JPH_EXPORT ComputeSystemDX12 : public ComputeSystem
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_EXPORT, ComputeSystemDX12)

	/// How we want to compile our shaders
	enum class EDebug
	{
		NoDebugSymbols,
		DebugSymbols
	};

	/// Initialize / shutdown
	void							Initialize(ID3D12Device *inDevice, EDebug inDebug);
	void							Shutdown();

	// See: ComputeSystem
	virtual ComputeShaderResult  	CreateComputeShader(const char *inName, uint32 inGroupSizeX, uint32 inGroupSizeY, uint32 inGroupSizeZ) override;
	virtual ComputeBufferResult  	CreateComputeBuffer(ComputeBuffer::EType inType, uint64 inSize, uint inStride, const void *inData = nullptr) override;
	virtual ComputeQueueResult  	CreateComputeQueue() override;

	/// Access to the DX12 device
	ID3D12Device *					GetDevice() const								{ return mDevice.Get(); }

	// Function to create a ID3D12Resource on specified heap with specified state
	ComPtr<ID3D12Resource>			CreateD3DResource(D3D12_HEAP_TYPE inHeapType, D3D12_RESOURCE_STATES inResourceState, D3D12_RESOURCE_FLAGS inFlags, uint64 inSize);

private:
	ComPtr<ID3D12Device>			mDevice;
	EDebug							mDebug = EDebug::NoDebugSymbols;
};

JPH_NAMESPACE_END

#endif // JPH_USE_DX12
