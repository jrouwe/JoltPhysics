// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#ifdef JPH_USE_MTL

#include <Jolt/Compute/MTL/ComputeShaderMTL.h>

JPH_NAMESPACE_BEGIN

ComputeShaderMTL::ComputeShaderMTL(id<MTLComputePipelineState> inPipelineState, MTLComputePipelineReflection *inReflection, uint32 inGroupSizeX, uint32 inGroupSizeY, uint32 inGroupSizeZ) :
	ComputeShader(inGroupSizeX, inGroupSizeY, inGroupSizeZ),
	mPipelineState(inPipelineState)
{
	for (id<MTLBinding> binding in inReflection.bindings)
	{
		const char *name = [binding.name UTF8String];
		uint index = uint(binding.index);
		mNameToBindingIndex[name] = index;
	}
}

uint ComputeShaderMTL::NameToBindingIndex(const char *inName) const
{
	UnorderedMap<String, uint>::const_iterator it = mNameToBindingIndex.find(inName);
	JPH_ASSERT(it != mNameToBindingIndex.end());
	return it->second;
}

JPH_NAMESPACE_END

#endif // JPH_USE_MTL
