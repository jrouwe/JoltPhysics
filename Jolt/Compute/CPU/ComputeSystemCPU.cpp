// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#ifdef JPH_USE_CPU_COMPUTE

#include <Jolt/Compute/CPU/ComputeSystemCPU.h>
#include <Jolt/Compute/CPU/ComputeQueueCPU.h>
#include <Jolt/Compute/CPU/ComputeBufferCPU.h>

JPH_NAMESPACE_BEGIN

JPH_IMPLEMENT_RTTI_VIRTUAL(ComputeSystemCPU)
{
	JPH_ADD_BASE_CLASS(ComputeSystemCPU, ComputeSystem)
}

ComputeShaderResult ComputeSystemCPU::CreateComputeShader(const char *inName, uint32 inGroupSizeX, uint32 inGroupSizeY, uint32 inGroupSizeZ)
{
	ComputeShaderResult result;
	const ShaderRegistry::const_iterator it = mShaderRegistry.find(inName);
	if (it == mShaderRegistry.end())
	{
		result.SetError("Compute shader not found");
		return result;
	}
	result.Set(new ComputeShaderCPU(it->second, inGroupSizeX, inGroupSizeY, inGroupSizeZ));
	return result;
}

ComputeBufferResult ComputeSystemCPU::CreateComputeBuffer(ComputeBuffer::EType inType, uint64 inSize, uint inStride, const void *inData)
{
	ComputeBufferResult result;
	result.Set(new ComputeBufferCPU(inType, inSize, inStride, inData));
	return result;
}

ComputeQueueResult ComputeSystemCPU::CreateComputeQueue()
{
	ComputeQueueResult result;
	result.Set(new ComputeQueueCPU());
	return result;
}

ComputeSystemResult CreateComputeSystemCPU()
{
	ComputeSystemResult result;
	result.Set(new ComputeSystemCPU());
	return result;
}

JPH_NAMESPACE_END

#endif // JPH_USE_CPU_COMPUTE
