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
	bool							Initialize(VkPhysicalDevice inPhysicalDevice, VkDevice inDevice, uint32 inComputeQueueIndex, ComputeSystemResult &outResult);
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

protected:
	/// Initialize / shutdown the memory subsystem
	virtual bool					InitializeMemory() = 0;
	virtual void					ShutdownMemory() = 0;

	VkPhysicalDevice				mPhysicalDevice = VK_NULL_HANDLE;
	VkDevice						mDevice = VK_NULL_HANDLE;
	uint32							mComputeQueueIndex = 0;
	PFN_vkSetDebugUtilsObjectNameEXT mVkSetDebugUtilsObjectNameEXT = nullptr;

private:
	// Buffer that can be bound when we have no buffer
	BufferVK						mDummyBuffer;
};

JPH_NAMESPACE_END

#endif // JPH_USE_VK
