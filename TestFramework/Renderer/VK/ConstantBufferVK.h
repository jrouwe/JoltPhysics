// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <vulkan/vulkan.h>

class RendererVK;

/// A binary blob that can be used to pass constants to a shader
class ConstantBufferVK
{
public:
	/// Constructor
										ConstantBufferVK(RendererVK *inRenderer, VkDeviceSize inBufferSize);
										~ConstantBufferVK();

	/// Map / unmap buffer (get pointer to data). This will discard all data in the buffer.
	template <typename T> T *			Map()											{ return reinterpret_cast<T *>(MapInternal()); }
	void								Unmap();

	VkBuffer							GetBuffer() const								{ return mBuffer; }

private:
	void *								MapInternal();

	RendererVK *						mRenderer;
	VkBuffer							mBuffer = VK_NULL_HANDLE;
	VkDeviceMemory						mBufferMemory = VK_NULL_HANDLE;
	VkDeviceSize						mBufferSize;
};