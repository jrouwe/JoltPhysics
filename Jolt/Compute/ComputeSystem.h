// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Compute/ComputeShader.h>
#include <Jolt/Compute/ComputeBuffer.h>
#include <Jolt/Compute/ComputeQueue.h>

JPH_NAMESPACE_BEGIN

/// Interface to run a workload on the GPU
class JPH_EXPORT ComputeSystem : public RefTarget<ComputeSystem>, public NonCopyable
{
public:
	JPH_OVERRIDE_NEW_DELETE

	/// Destructor
	virtual							~ComputeSystem() = default;

	/// Compile a compute shader
	virtual Ref<ComputeShader>		CreateComputeShader(const char *inName, uint32 inGroupSizeX, uint32 inGroupSizeY = 1, uint32 inGroupSizeZ = 1) = 0;

	/// Create a buffer for use with a compute shader
	virtual Ref<ComputeBuffer>		CreateComputeBuffer(ComputeBuffer::EType inType, uint64 inSize, uint inStride, const void *inData = nullptr) = 0;

	/// Create a queue for executing compute shaders
	virtual Ref<ComputeQueue>		CreateComputeQueue() = 0;

	/// Callback used when loading shaders
	using ShaderLoader = std::function<bool(const char *inName, Array<uint8> &outData)>;
	ShaderLoader					mShaderLoader = [](const char *, Array<uint8> &) { JPH_ASSERT(false, "Override this function"); return false; };
};

#ifdef JPH_USE_VK
/// Factory function to create a compute system using Vulkan
extern JPH_EXPORT ComputeSystem *	CreateComputeSystemVK();
#endif

#ifdef JPH_USE_DX12

/// Factory function to create a compute system using DirectX 12
extern JPH_EXPORT ComputeSystem *	CreateComputeSystemDX12();

/// Factory function to create the default compute system for this platform
inline ComputeSystem *				CreateComputeSystem()		{ return CreateComputeSystemDX12(); }

#elif defined(JPH_USE_MTL)

/// Factory function to create a compute system using Metal
extern JPH_EXPORT ComputeSystem *	CreateComputeSystemMTL();

/// Factory function to create the default compute system for this platform
inline ComputeSystem *				CreateComputeSystem()		{ return CreateComputeSystemMTL(); }

#elif defined(JPH_USE_VK)

/// Factory function to create the default compute system for this platform
inline ComputeSystem *				CreateComputeSystem()		{ return CreateComputeSystemVK(); }

#else

/// Fallback implementation when no compute system is available
inline ComputeSystem *				CreateComputeSystem()		{ return nullptr; }

#endif

JPH_NAMESPACE_END
