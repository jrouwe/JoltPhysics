// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Compute/ComputeSystem.h>

#ifdef JPH_USE_MTL

#include <MetalKit/MetalKit.h>

JPH_NAMESPACE_BEGIN

/// Interface to run a workload on the GPU
class JPH_EXPORT ComputeSystemMTL : public ComputeSystem
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_EXPORT, ComputeSystemMTL)

	// Initialize / shutdown the compute system
	bool							Initialize(id<MTLDevice> inDevice);
	void							Shutdown();

	// See: ComputeSystem
	virtual ComputeShaderResult		CreateComputeShader(const char *inName, uint32 inGroupSizeX, uint32 inGroupSizeY, uint32 inGroupSizeZ) override;
	virtual ComputeBufferResult		CreateComputeBuffer(ComputeBuffer::EType inType, uint64 inSize, uint inStride, const void *inData = nullptr) override;
	virtual ComputeQueueResult		CreateComputeQueue() override;

	/// Get the metal device
	id<MTLDevice>					GetDevice() const						{ return mDevice; }

private:
	id<MTLDevice>					mDevice;
	id<MTLLibrary>					mShaderLibrary;
};

JPH_NAMESPACE_END

#endif // JPH_USE_MTL
