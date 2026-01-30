// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#ifdef JPH_USE_WGPU

#include <Jolt/Compute/WGPU/ComputeSystemWGPU.h>

JPH_NAMESPACE_BEGIN

JPH_IMPLEMENT_RTTI_VIRTUAL(ComputeSystemWGPU)
{
	JPH_ADD_BASE_CLASS(ComputeSystemWGPU, ComputeSystem)
}

ComputeSystemWGPU::~ComputeSystemWGPU()
{
	if (mQueue != nullptr)
	{
		wgpuQueueRelease(mQueue);
		mQueue = nullptr;
	}

	if (mDevice != nullptr)
	{
		wgpuDeviceRelease(mDevice);
		mDevice = nullptr;
	}
}

bool ComputeSystemWGPU::Initialize(ComputeSystemResult &outResult)
{
	mDevice = emscripten_webgpu_get_device();
	if (mDevice == nullptr)
	{
		outResult.SetError("WebGPU: emscripten_webgpu_get_device returned null. WebGPU may not be available in this runtime.");
		return false;
	}

	mQueue = wgpuDeviceGetQueue(mDevice);
	if (mQueue == nullptr)
	{
		outResult.SetError("WebGPU: failed to get device queue.");
		return false;
	}

	return true;
}

ComputeShaderResult ComputeSystemWGPU::CreateComputeShader(const char *, uint32, uint32, uint32)
{
	ComputeShaderResult result;
	result.SetError("ComputeSystemWGPU::CreateComputeShader not implemented");
	return result;
}

ComputeBufferResult ComputeSystemWGPU::CreateComputeBuffer(ComputeBuffer::EType, uint64, uint, const void *)
{
	ComputeBufferResult result;
	result.SetError("ComputeSystemWGPU::CreateComputeBuffer not implemented");
	return result;
}

ComputeQueueResult ComputeSystemWGPU::CreateComputeQueue()
{
	ComputeQueueResult result;
	result.SetError("ComputeSystemWGPU::CreateComputeQueue not implemented");
	return result;
}

ComputeSystemResult CreateComputeSystemWGPU()
{
	ComputeSystemResult result;

	Ref<ComputeSystemWGPU> compute = new ComputeSystemWGPU();
	if (!compute->Initialize(result))
		return result;

	result.Set(compute.GetPtr());
	return result;
}

JPH_NAMESPACE_END

#endif // JPH_USE_WGPU
