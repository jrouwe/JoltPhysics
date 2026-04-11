// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Compute/ComputeSystem.h>

#ifdef JPH_USE_VK

#include <Jolt/Compute/VK/ComputeQueueVK.h>

JPH_NAMESPACE_BEGIN

/// Interface to run a workload on the GPU using Vulkan.
/// Minimal implementation that can integrate with your own Vulkan setup.
class JPH_EXPORT ComputeSystemVK : public ComputeSystem
{
public:
	JPH_DECLARE_RTTI_ABSTRACT(JPH_EXPORT, ComputeSystemVK)

	// Initialize / shutdown the compute system
	bool							Initialize(VkPhysicalDevice inPhysicalDevice, PFN_vkGetDeviceProcAddr inVkGetDeviceProcAddr, VkDevice inDevice, uint32 inComputeQueueIndex, ComputeSystemResult &outResult);
	void							Shutdown();

	// See: ComputeSystem
	virtual ComputeShaderResult		CreateComputeShader(const char *inName, uint32 inGroupSizeX, uint32 inGroupSizeY, uint32 inGroupSizeZ) override;
	virtual ComputeBufferResult		CreateComputeBuffer(ComputeBuffer::EType inType, uint64 inSize, uint inStride, const void *inData = nullptr) override;
	virtual ComputeQueueResult		CreateComputeQueue() override;

	/// Access to the Vulkan device
	VkDevice						GetDevice() const												{ return mDevice; }

	/// Allow the application to override buffer creation and memory mapping in case it uses its own allocator
	virtual bool					CreateBuffer(VkDeviceSize inSize, VkBufferUsageFlags inUsage, VkMemoryPropertyFlags inProperties, BufferVK &outBuffer) = 0;
	virtual void					FreeBuffer(BufferVK &ioBuffer) = 0;
	virtual void *					MapBuffer(BufferVK &ioBuffer) = 0;
	virtual void					UnmapBuffer(BufferVK &ioBuffer) = 0;

	// Vulkan device function pointers
	PFN_vkAllocateCommandBuffers	mVkAllocateCommandBuffers = nullptr;
	PFN_vkAllocateDescriptorSets	mVkAllocateDescriptorSets = nullptr;
	PFN_vkAllocateMemory			mVkAllocateMemory = nullptr;
	PFN_vkBeginCommandBuffer		mVkBeginCommandBuffer = nullptr;
	PFN_vkBindBufferMemory			mVkBindBufferMemory = nullptr;
	PFN_vkCmdBindDescriptorSets		mVkCmdBindDescriptorSets = nullptr;
	PFN_vkCmdBindPipeline			mVkCmdBindPipeline = nullptr;
	PFN_vkCmdCopyBuffer				mVkCmdCopyBuffer = nullptr;
	PFN_vkCmdDispatch				mVkCmdDispatch = nullptr;
	PFN_vkCmdPipelineBarrier		mVkCmdPipelineBarrier = nullptr;
	PFN_vkCreateBuffer				mVkCreateBuffer = nullptr;
	PFN_vkCreateCommandPool			mVkCreateCommandPool = nullptr;
	PFN_vkCreateComputePipelines	mVkCreateComputePipelines = nullptr;
	PFN_vkCreateDescriptorPool		mVkCreateDescriptorPool = nullptr;
	PFN_vkCreateDescriptorSetLayout	mVkCreateDescriptorSetLayout = nullptr;
	PFN_vkCreateFence				mVkCreateFence = nullptr;
	PFN_vkCreatePipelineLayout		mVkCreatePipelineLayout = nullptr;
	PFN_vkCreateShaderModule		mVkCreateShaderModule = nullptr;
	PFN_vkDestroyBuffer				mVkDestroyBuffer = nullptr;
	PFN_vkDestroyCommandPool		mVkDestroyCommandPool = nullptr;
	PFN_vkDestroyDescriptorPool		mVkDestroyDescriptorPool = nullptr;
	PFN_vkDestroyDescriptorSetLayout mVkDestroyDescriptorSetLayout = nullptr;
	PFN_vkDestroyDevice				mVkDestroyDevice = nullptr;
	PFN_vkDestroyFence				mVkDestroyFence = nullptr;
	PFN_vkDestroyPipeline			mVkDestroyPipeline = nullptr;
	PFN_vkDestroyPipelineLayout		mVkDestroyPipelineLayout = nullptr;
	PFN_vkDestroyShaderModule		mVkDestroyShaderModule = nullptr;
	PFN_vkDeviceWaitIdle			mVkDeviceWaitIdle = nullptr;
	PFN_vkEndCommandBuffer			mVkEndCommandBuffer = nullptr;
	PFN_vkFreeCommandBuffers		mVkFreeCommandBuffers = nullptr;
	PFN_vkFreeMemory				mVkFreeMemory = nullptr;
	PFN_vkGetBufferMemoryRequirements mVkGetBufferMemoryRequirements = nullptr;
	PFN_vkGetDeviceQueue			mVkGetDeviceQueue = nullptr;
	PFN_vkMapMemory					mVkMapMemory = nullptr;
	PFN_vkQueueSubmit				mVkQueueSubmit = nullptr;
	PFN_vkResetCommandBuffer		mVkResetCommandBuffer = nullptr;
	PFN_vkResetDescriptorPool		mVkResetDescriptorPool = nullptr;
	PFN_vkResetFences				mVkResetFences = nullptr;
	PFN_vkSetDebugUtilsObjectNameEXT mVkSetDebugUtilsObjectNameEXT = nullptr;
	PFN_vkUnmapMemory				mVkUnmapMemory = nullptr;
	PFN_vkUpdateDescriptorSets		mVkUpdateDescriptorSets = nullptr;
	PFN_vkWaitForFences				mVkWaitForFences = nullptr;

protected:
	/// Initialize / shutdown the memory subsystem
	virtual bool					InitializeMemory() = 0;
	virtual void					ShutdownMemory() = 0;

	VkPhysicalDevice				mPhysicalDevice = VK_NULL_HANDLE;
	VkDevice						mDevice = VK_NULL_HANDLE;
	uint32							mComputeQueueIndex = 0;

private:
	// Buffer that can be bound when we have no buffer
	BufferVK						mDummyBuffer;
};

JPH_NAMESPACE_END

#endif // JPH_USE_VK
