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

JPH_IMPLEMENT_RTTI_ABSTRACT(ComputeSystemVK)
{
	JPH_ADD_BASE_CLASS(ComputeSystemVK, ComputeSystem)
}

bool ComputeSystemVK::Initialize(VkPhysicalDevice inPhysicalDevice, PFN_vkGetDeviceProcAddr inVkGetDeviceProcAddr, VkDevice inDevice, uint32 inComputeQueueIndex, ComputeSystemResult &outResult)
{
	mPhysicalDevice = inPhysicalDevice;
	mDevice = inDevice;
	mComputeQueueIndex = inComputeQueueIndex;

	// Load Vulkan device functions
	#define JPH_LOAD_VK(name) mVk##name = reinterpret_cast<PFN_vk##name>(reinterpret_cast<void *>(inVkGetDeviceProcAddr(mDevice, "vk" #name))); JPH_ASSERT(mVk##name != nullptr)
	JPH_LOAD_VK(AllocateCommandBuffers);
	JPH_LOAD_VK(AllocateDescriptorSets);
	JPH_LOAD_VK(AllocateMemory);
	JPH_LOAD_VK(BeginCommandBuffer);
	JPH_LOAD_VK(BindBufferMemory);
	JPH_LOAD_VK(CmdBindDescriptorSets);
	JPH_LOAD_VK(CmdBindPipeline);
	JPH_LOAD_VK(CmdCopyBuffer);
	JPH_LOAD_VK(CmdDispatch);
	JPH_LOAD_VK(CmdPipelineBarrier);
	JPH_LOAD_VK(CreateBuffer);
	JPH_LOAD_VK(CreateCommandPool);
	JPH_LOAD_VK(CreateComputePipelines);
	JPH_LOAD_VK(CreateDescriptorPool);
	JPH_LOAD_VK(CreateDescriptorSetLayout);
	JPH_LOAD_VK(CreateFence);
	JPH_LOAD_VK(CreatePipelineLayout);
	JPH_LOAD_VK(CreateShaderModule);
	JPH_LOAD_VK(DestroyBuffer);
	JPH_LOAD_VK(DestroyCommandPool);
	JPH_LOAD_VK(DestroyDescriptorPool);
	JPH_LOAD_VK(DestroyDescriptorSetLayout);
	JPH_LOAD_VK(DestroyDevice);
	JPH_LOAD_VK(DestroyFence);
	JPH_LOAD_VK(DestroyPipeline);
	JPH_LOAD_VK(DestroyPipelineLayout);
	JPH_LOAD_VK(DestroyShaderModule);
	JPH_LOAD_VK(DeviceWaitIdle);
	JPH_LOAD_VK(EndCommandBuffer);
	JPH_LOAD_VK(FreeCommandBuffers);
	JPH_LOAD_VK(FreeMemory);
	JPH_LOAD_VK(GetBufferMemoryRequirements);
	JPH_LOAD_VK(GetDeviceQueue);
	JPH_LOAD_VK(MapMemory);
	JPH_LOAD_VK(QueueSubmit);
	JPH_LOAD_VK(ResetCommandBuffer);
	JPH_LOAD_VK(ResetDescriptorPool);
	JPH_LOAD_VK(ResetFences);
	JPH_LOAD_VK(UnmapMemory);
	JPH_LOAD_VK(UpdateDescriptorSets);
	JPH_LOAD_VK(WaitForFences);
	#undef JPH_LOAD_VK

	// Get function to set a debug name
	mVkSetDebugUtilsObjectNameEXT = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(reinterpret_cast<void *>(inVkGetDeviceProcAddr(mDevice, "vkSetDebugUtilsObjectNameEXT")));

	if (!InitializeMemory())
	{
		outResult.SetError("Failed to initialize memory subsystem");
		return false;
	}

	// Create the dummy buffer. This is used to bind to shaders for which we have no buffer. We can't rely on VK_EXT_robustness2 being available to set nullDescriptor = VK_TRUE (it is unavailable on macOS).
	if (!CreateBuffer(1024, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mDummyBuffer))
	{
		outResult.SetError("Failed to create dummy buffer");
		return false;
	}

	return true;
}

void ComputeSystemVK::Shutdown()
{
	if (mDevice != VK_NULL_HANDLE)
		mVkDeviceWaitIdle(mDevice);

	// Free the dummy buffer
	FreeBuffer(mDummyBuffer);

	ShutdownMemory();
}

ComputeShaderResult ComputeSystemVK::CreateComputeShader(const char *inName, uint32 inGroupSizeX, uint32 inGroupSizeY, uint32 inGroupSizeZ)
{
	ComputeShaderResult result;

	// Read shader source file
	Array<uint8> data;
	String file_name = String(inName) + ".spv";
	String error;
	if (!mShaderLoader(file_name.c_str(), data, error))
	{
		result.SetError(error);
		return result;
	}

	Ref<ComputeShaderVK> shader = new ComputeShaderVK(this, inGroupSizeX, inGroupSizeY, inGroupSizeZ);
	if (!shader->Initialize(data, mDummyBuffer.mBuffer, result))
		return result;

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

	result.Set(shader.GetPtr());
	return result;
}

ComputeBufferResult ComputeSystemVK::CreateComputeBuffer(ComputeBuffer::EType inType, uint64 inSize, uint inStride, const void *inData)
{
	ComputeBufferResult result;

	Ref<ComputeBufferVK> buffer = new ComputeBufferVK(this, inType, inSize, inStride);
	if (!buffer->Initialize(inData))
	{
		result.SetError("Failed to create compute buffer");
		return result;
	}

	result.Set(buffer.GetPtr());
	return result;
}

ComputeQueueResult ComputeSystemVK::CreateComputeQueue()
{
	ComputeQueueResult result;
	Ref<ComputeQueueVK> q = new ComputeQueueVK(this);
	if (!q->Initialize(mComputeQueueIndex, result))
		return result;
	result.Set(q.GetPtr());
	return result;
}

JPH_NAMESPACE_END

#endif // JPH_USE_VK
