// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#ifdef JPH_USE_VK

#include <Jolt/Compute/VK/ComputeSystemVK.h>
#include <Jolt/Compute/VK/ComputeShaderVK.h>
#include <Jolt/Compute/VK/ComputeBufferVK.h>
#include <Jolt/Compute/VK/ComputeQueueVK.h>

JPH_NAMESPACE_BEGIN

bool ComputeSystemVK::Initialize(VkPhysicalDevice inPhysicalDevice, VkDevice inDevice, uint32 inComputeQueueIndex)
{
	mPhysicalDevice = inPhysicalDevice;
	mDevice = inDevice;
	mComputeQueueIndex = inComputeQueueIndex;

	// Get function to set a debug name
	mVkSetDebugUtilsObjectNameEXT = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(reinterpret_cast<void *>(vkGetDeviceProcAddr(mDevice, "vkSetDebugUtilsObjectNameEXT")));

	if (!InitializeMemory())
		return false;

	// Create the dummy buffer. This is used to bind to shaders for which we have no buffer. We can't rely on VK_EXT_robustness2 being available to set nullDescriptor = VK_TRUE (it is unavailable on macOS).
	CreateBuffer(1024, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mDummyBuffer);

	return true;
}

void ComputeSystemVK::Shutdown()
{
	if (mDevice != VK_NULL_HANDLE)
		vkDeviceWaitIdle(mDevice);

	// Free the dummy buffer
	FreeBuffer(mDummyBuffer);

	ShutdownMemory();
}

Ref<ComputeShader> ComputeSystemVK::CreateComputeShader(const char *inName, uint32 inGroupSizeX, uint32 inGroupSizeY, uint32 inGroupSizeZ)
{
	// Read shader source file
	Array<uint8> data;
	String file_name = String(inName) + ".spv";
	if (!mShaderLoader(file_name.c_str(), data))
		return nullptr;

	Ref<ComputeShaderVK> shader = new ComputeShaderVK(mDevice, inGroupSizeX, inGroupSizeY, inGroupSizeZ);
	if (!shader->Initialize(data, mDummyBuffer.mBuffer))
	{
		Trace("Failed to create compute shader: %s", file_name.c_str());
		return nullptr;
	}

	// Name the pipeline so we can easily find it in a profile
	if (mVkSetDebugUtilsObjectNameEXT != nullptr)
	{
		VkDebugUtilsObjectNameInfoEXT info = {};
		info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		info.pNext = nullptr;
		info.objectType = VK_OBJECT_TYPE_PIPELINE;
		info.objectHandle = (uint64)shader->GetPipeline();
		info.pObjectName = inName;
		mVkSetDebugUtilsObjectNameEXT(mDevice, &info);
	}

	return shader.GetPtr();
}

Ref<ComputeBuffer> ComputeSystemVK::CreateComputeBuffer(ComputeBuffer::EType inType, uint64 inSize, uint inStride, const void *inData)
{
	return new ComputeBufferVK(this, inType, inSize, inStride, inData);
}

Ref<ComputeQueue> ComputeSystemVK::CreateComputeQueue()
{
	Ref<ComputeQueueVK> q = new ComputeQueueVK(this);
	if (!q->Initialize(mComputeQueueIndex))
		return nullptr;
	return q.GetPtr();
}

JPH_NAMESPACE_END

#endif // JPH_USE_VK
