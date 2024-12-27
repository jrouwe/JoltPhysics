// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <vulkan/vulkan.h>

/// Simple wrapper class to manage a Vulkan buffer
class BufferVK
{
public:
	/// Free memory associated with a buffer
	void						Free(VkDevice inDevice)
	{
		if (mBuffer != VK_NULL_HANDLE)
		{
			vkDestroyBuffer(inDevice, mBuffer, nullptr);
			mBuffer = VK_NULL_HANDLE;
		}

		if (mMemory != VK_NULL_HANDLE)
		{
			vkFreeMemory(inDevice, mMemory, nullptr);
			mMemory = VK_NULL_HANDLE;
		}
	}

	VkBuffer					mBuffer = VK_NULL_HANDLE;
	VkDeviceMemory				mMemory = VK_NULL_HANDLE;

	VkBufferUsageFlags			mUsage;
	VkMemoryPropertyFlags		mProperties;
	VkDeviceSize				mSize = 0;
};
