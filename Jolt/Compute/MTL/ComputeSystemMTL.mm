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

JPH_IMPLEMENT_RTTI_VIRTUAL(ComputeSystemMTL)
{
	JPH_ADD_BASE_CLASS(ComputeSystemMTL, ComputeSystem)
}

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

ComputeShaderResult ComputeSystemMTL::CreateComputeShader(const char *inName, uint32 inGroupSizeX, uint32 inGroupSizeY, uint32 inGroupSizeZ)
{
	ComputeShaderResult result;

	if (mShaderLibrary == nil)
	{
		// Load the shader library containing all shaders
		Array<uint8> *data = new Array<uint8>();
		String error;
		if (!mShaderLoader("Jolt.metallib", *data, error))
		{
			result.SetError(error);
			delete data;
			return result;
		}

		// Convert to dispatch data
		dispatch_data_t data_dispatch = dispatch_data_create(data->data(), data->size(), nullptr, ^{ delete data; });

		// Create the library
		NSError *ns_error = nullptr;
		mShaderLibrary = [mDevice newLibraryWithData: data_dispatch error: &ns_error];
		if (ns_error != nil)
		{
			result.SetError("Failed to laod shader library");
			return result;
		}
	}

	// Get the shader function
	id<MTLFunction> function = [mShaderLibrary newFunctionWithName: [NSString stringWithCString: inName encoding: NSUTF8StringEncoding]];
	if (function == nil)
	{
		result.SetError("Failed to instantiate compute shader");
		return result;
	}

	// Create the pipeline
	NSError *error = nil;
	MTLComputePipelineReflection *reflection = nil;
	// Note: MTLPipelineOptionBindingInfo was introduced in iOS 17 / macOS 14 SDK
	// (renamed from MTLPipelineOptionArgumentInfo). Fall back to the old name on
	// older SDKs so the file still compiles with Xcode <= 15.x toolchains.
	#if (defined(__IPHONE_OS_VERSION_MAX_ALLOWED) && __IPHONE_OS_VERSION_MAX_ALLOWED >= 170000) \
		|| (defined(__MAC_OS_X_VERSION_MAX_ALLOWED) && __MAC_OS_X_VERSION_MAX_ALLOWED >= 140000) \
		|| (defined(__TV_OS_VERSION_MAX_ALLOWED) && __TV_OS_VERSION_MAX_ALLOWED >= 170000)
		MTLPipelineOption pipeline_options = MTLPipelineOptionBindingInfo | MTLPipelineOptionBufferTypeInfo;
	#else
		MTLPipelineOption pipeline_options = MTLPipelineOptionArgumentInfo | MTLPipelineOptionBufferTypeInfo;
	#endif
	id<MTLComputePipelineState> pipeline_state = [mDevice newComputePipelineStateWithFunction: function options: pipeline_options reflection: &reflection error: &error];
	if (error != nil || pipeline_state == nil)
	{
		result.SetError("Failed to create compute pipeline");
		[function release];
		return result;
	}

	result.Set(new ComputeShaderMTL(pipeline_state, reflection, inGroupSizeX, inGroupSizeY, inGroupSizeZ));
	return result;
}

ComputeBufferResult ComputeSystemMTL::CreateComputeBuffer(ComputeBuffer::EType inType, uint64 inSize, uint inStride, const void *inData)
{
	ComputeBufferResult result;

	Ref<ComputeBufferMTL> buffer = new ComputeBufferMTL(this, inType, inSize, inStride);
	if (!buffer->Initialize(inData))
	{
		result.SetError("Failed to create compute buffer");
		return result;
	}

	result.Set(buffer.GetPtr());
	return result;
}

ComputeQueueResult ComputeSystemMTL::CreateComputeQueue()
{
	ComputeQueueResult result;
	result.Set(new ComputeQueueMTL(mDevice));
	return result;
}

JPH_NAMESPACE_END

#endif // JPH_USE_MTL
