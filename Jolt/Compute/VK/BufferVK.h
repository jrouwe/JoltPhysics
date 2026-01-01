// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Compute/VK/IncludeVK.h>
#include <Jolt/Core/Reference.h>
#include <Jolt/Core/NonCopyable.h>

JPH_NAMESPACE_BEGIN

/// Simple wrapper class to manage a Vulkan memory block
class MemoryVK : public RefTarget<MemoryVK>, public NonCopyable
{
public:
								~MemoryVK()
	{
		// We should have unmapped and freed the block before destruction
		JPH_ASSERT(mMappedCount == 0);
		JPH_ASSERT(mMemory == VK_NULL_HANDLE);
	}

	VkDeviceMemory				mMemory = VK_NULL_HANDLE;		///< The Vulkan memory handle
	VkDeviceSize				mSize = 0;						///< Size of the memory block
	VkDeviceSize				mBufferSize = 0;				///< Size of each of the buffers that this memory block has been divided into
	VkMemoryPropertyFlags		mProperties = 0;				///< Vulkan memory properties used to allocate this block
	int							mMappedCount = 0;				///< How often buffers using this memory block were mapped
	void *						mMappedPtr = nullptr;			///< The CPU address of the memory block when mapped
};

/// Simple wrapper class to manage a Vulkan buffer
class BufferVK
{
public:
	Ref<MemoryVK>				mMemory;						///< The memory block that contains the buffer (note that filling this in is optional if you do your own buffer allocation)
	VkBuffer					mBuffer = VK_NULL_HANDLE;		///< The Vulkan buffer handle
	VkDeviceSize				mOffset = 0;					///< Offset in the memory block where the buffer starts
	VkDeviceSize				mSize = 0;						///< Real size of the buffer
};

JPH_NAMESPACE_END
