// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Compute/ComputeSystem.h>

#ifdef JPH_USE_WGPU

#include <Jolt/Compute/WGPU/IncludeWGPU.h>

JPH_NAMESPACE_BEGIN

/// Interface to run a workload on the GPU using WebGPU
class JPH_EXPORT ComputeSystemWGPU : public ComputeSystem
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_EXPORT, ComputeSystemWGPU)

	/// Destructor
	virtual								~ComputeSystemWGPU() override;

	/// Initialize the compute system
	bool								Initialize(ComputeSystemResult &outResult);

	// See: ComputeSystem
	virtual ComputeShaderResult			CreateComputeShader(const char *inName, uint32 inGroupSizeX, uint32 inGroupSizeY = 1, uint32 inGroupSizeZ = 1) override;
	virtual ComputeBufferResult			CreateComputeBuffer(ComputeBuffer::EType inType, uint64 inSize, uint inStride, const void *inData = nullptr) override;
	virtual ComputeQueueResult			CreateComputeQueue() override;

private:
	WGPUDevice							mDevice = nullptr;
	WGPUQueue							mQueue = nullptr;
};

JPH_NAMESPACE_END

#endif // JPH_USE_WGPU
