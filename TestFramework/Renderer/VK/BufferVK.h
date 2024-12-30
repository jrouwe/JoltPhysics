// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <vulkan/vulkan.h>

/// Simple wrapper class to manage a Vulkan buffer
class BufferVK
{
public:
	VkBuffer					mBuffer = VK_NULL_HANDLE;
	VkDeviceMemory				mMemory = VK_NULL_HANDLE;
	VkDeviceSize				mOffset = 0;
	VkDeviceSize				mSize = 0;

	VkBufferUsageFlags			mUsage;
	VkMemoryPropertyFlags		mProperties;
	VkDeviceSize				mAllocatedSize;
};
