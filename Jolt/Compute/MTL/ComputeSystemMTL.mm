// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#ifdef JPH_USE_MTL

#include <Jolt/Compute/MTL/ComputeSystemMTL.h>
#include <Jolt/Compute/MTL/ComputeBufferMTL.h>
#include <Jolt/Compute/MTL/ComputeShaderMTL.h>
#include <Jolt/Compute/MTL/ComputeQueueMTL.h>

JPH_NAMESPACE_BEGIN

bool ComputeSystemMTL::Initialize(id<MTLDevice> inDevice)
{
	mDevice = [inDevice retain];

	return true;
}

void ComputeSystemMTL::Shutdown()
{
	[mShaderLibrary release];
	[mDevice release];
}

Ref<ComputeShader> ComputeSystemMTL::CreateComputeShader(const char *inName, uint32 inGroupSizeX, uint32 inGroupSizeY, uint32 inGroupSizeZ)
{
	if (mShaderLibrary == nil)
	{
		// Load the shader library containing all shaders
		Array<uint8> *data = new Array<uint8>();
		if (!mShaderLoader("Jolt.metallib", *data))
		{
			JPH_ASSERT(false, "Failed to load shader library");
			delete data;
			return nullptr;
		}

		// Convert to dispatch data
		dispatch_data_t data_dispatch = dispatch_data_create(data->data(), data->size(), nullptr, ^{ delete data; });

		// Create the library
		NSError *error = nullptr;
		mShaderLibrary = [mDevice newLibraryWithData: data_dispatch error: &error];
		if (error != nil)
		{
			JPH_ASSERT(false, "Failed to load shader library");
			return nullptr;
		}
	}

	// Get the shader function
	id<MTLFunction> function = [mShaderLibrary newFunctionWithName: [NSString stringWithCString: inName encoding: NSUTF8StringEncoding]];
	if (function == nil)
	{
		Trace("Failed to create compute shader: %s", inName);
		return nullptr;
	}

	// Create the pipeline
	NSError *error = nil;
	MTLComputePipelineReflection *reflection = nil;
	id<MTLComputePipelineState> pipeline_state = [mDevice newComputePipelineStateWithFunction: function options: MTLPipelineOptionBindingInfo | MTLPipelineOptionBufferTypeInfo reflection: &reflection error: &error];
	if (error != nil || pipeline_state == nil)
	{
		JPH_ASSERT(false, "Failed to create compute pipeline");
		[function release];
		return nullptr;
	}

	return new ComputeShaderMTL(pipeline_state, reflection, inGroupSizeX, inGroupSizeY, inGroupSizeZ);
}

Ref<ComputeBuffer> ComputeSystemMTL::CreateComputeBuffer(ComputeBuffer::EType inType, uint64 inSize, uint inStride, const void *inData)
{
	return new ComputeBufferMTL(this, inType, inSize, inStride, inData);
}

Ref<ComputeQueue> ComputeSystemMTL::CreateComputeQueue()
{
	return new ComputeQueueMTL(mDevice);
}

JPH_NAMESPACE_END

#endif // JPH_USE_MTL
