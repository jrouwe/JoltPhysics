// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Compute/ComputeSystem.h>

#ifdef JPH_USE_CPU_COMPUTE

#include <Jolt/Core/UnorderedMap.h>
#include <Jolt/Compute/CPU/ComputeShaderCPU.h>

JPH_NAMESPACE_BEGIN

/// Interface to run a workload on the CPU
/// This is intended mainly for debugging purposes and is not optimized for performance
class JPH_EXPORT ComputeSystemCPU : public ComputeSystem
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_EXPORT, ComputeSystemCPU)

	// See: ComputeSystem
	virtual ComputeShaderResult  	CreateComputeShader(const char *inName, uint32 inGroupSizeX, uint32 inGroupSizeY, uint32 inGroupSizeZ) override;
	virtual ComputeBufferResult  	CreateComputeBuffer(ComputeBuffer::EType inType, uint64 inSize, uint inStride, const void *inData = nullptr) override;
	virtual ComputeQueueResult  	CreateComputeQueue() override;

	using CreateShader = ComputeShaderCPU::CreateShader;

	void							RegisterShader(const char *inName, CreateShader inCreateShader)
	{
		mShaderRegistry[inName] = inCreateShader;
	}

private:
	using ShaderRegistry = UnorderedMap<string_view, CreateShader>;
	ShaderRegistry					mShaderRegistry;
};

// Internal helpers
#define JPH_SHADER_WRAPPER_FUNCTION_NAME(name)		RegisterShader##name
#define JPH_SHADER_WRAPPER_FUNCTION(sys, name)		void JPH_EXPORT JPH_SHADER_WRAPPER_FUNCTION_NAME(name)(ComputeSystemCPU *sys)

/// Macro to declare a shader register function
#define JPH_DECLARE_REGISTER_SHADER(name)			namespace JPH { class ComputeSystemCPU; JPH_SHADER_WRAPPER_FUNCTION(, name); }

/// Macro to register a shader
#define JPH_REGISTER_SHADER(sys, name)				JPH::JPH_SHADER_WRAPPER_FUNCTION_NAME(name)(sys)

JPH_NAMESPACE_END

#endif // JPH_USE_CPU_COMPUTE
