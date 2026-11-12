// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#ifdef JPH_USE_MTL

#include <MetalKit/MetalKit.h>

#include <Jolt/Compute/ComputeShader.h>
#include <Jolt/Core/UnorderedMap.h>

JPH_NAMESPACE_BEGIN

/// Compute shader handle for Metal
class JPH_EXPORT ComputeShaderMTL : public ComputeShader
{
public:
	JPH_OVERRIDE_NEW_DELETE

	/// Constructor
								ComputeShaderMTL(id<MTLComputePipelineState> inPipelineState, MTLComputePipelineReflection *inReflection, uint32 inGroupSizeX, uint32 inGroupSizeY, uint32 inGroupSizeZ);
	virtual						~ComputeShaderMTL() override 					{ [mPipelineState release]; }

	/// Access to the function
	id<MTLComputePipelineState>	GetPipelineState() const						{ return mPipelineState; }

	/// Get index of buffer name
	uint						NameToBindingIndex(const char *inName) const;

	/// Get all buffer binding indices used by this shader
	const Array<uint> &			GetBufferBindingIndices() const					{ return mBufferBindingIndices; }

private:
	id<MTLComputePipelineState>	mPipelineState;
	UnorderedMap<String, uint>	mNameToBindingIndex;
	Array<uint>					mBufferBindingIndices;
};

JPH_NAMESPACE_END

#endif // JPH_USE_MTL
