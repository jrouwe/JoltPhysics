// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#ifdef JPH_USE_VK

#include <Jolt/Compute/VK/ComputeSystemVKWithAllocator.h>
#include <Jolt/Compute/VK/ComputeShaderVK.h>
#include <Jolt/Compute/VK/ComputeBufferVK.h>
#include <Jolt/Compute/VK/ComputeQueueVK.h>

JPH_NAMESPACE_BEGIN

JPH_IMPLEMENT_RTTI_VIRTUAL(ComputeSystemVKWithAllocator)
{
	JPH_ADD_BASE_CLASS(ComputeSystemVKWithAllocator, ComputeSystemVK)
}

bool ComputeSystemVKWithAllocator::InitializeMemory()
{
	// Get memory properties
	vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice, &mMemoryProperties);

	return true;
}

void ComputeSystemVKWithAllocator::ShutdownMemory()
{
	// Free all memory
	for (const MemoryCache::value_type &mc : mMemoryCache)
		for (const Memory &m : mc.second)
			if (m.mOffset == 0)
				FreeMemory(*m.mMemory);
	mMemoryCache.clear();
}

uint32 ComputeSystemVKWithAllocator::FindMemoryType(uint32 inTypeFilter, VkMemoryPropertyFlags inProperties) const
{
	for (uint32 i = 0; i < mMemoryProperties.memoryTypeCount; i++)
		if ((inTypeFilter & (1 << i))
			&& (mMemoryProperties.memoryTypes[i].propertyFlags & inProperties) == inProperties)
			return i;

	JPH_ASSERT(false, "Failed to find memory type!");
	return 0;
}

void ComputeSystemVKWithAllocator::AllocateMemory(VkDeviceSize inSize, uint32 inMemoryTypeBits, VkMemoryPropertyFlags inProperties, MemoryVK &ioMemory)
{
	JPH_ASSERT(ioMemory.mMemory == VK_NULL_HANDLE);

	ioMemory.mSize = inSize;
	ioMemory.mProperties = inProperties;

	VkMemoryAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize = inSize;
	alloc_info.memoryTypeIndex = FindMemoryType(inMemoryTypeBits, inProperties);
	vkAllocateMemory(mDevice, &alloc_info, nullptr, &ioMemory.mMemory);
}

void ComputeSystemVKWithAllocator::FreeMemory(MemoryVK &ioMemory)
{
	vkFreeMemory(mDevice, ioMemory.mMemory, nullptr);
	ioMemory.mMemory = VK_NULL_HANDLE;
}

bool ComputeSystemVKWithAllocator::CreateBuffer(VkDeviceSize inSize, VkBufferUsageFlags inUsage, VkMemoryPropertyFlags inProperties, BufferVK &outBuffer)
{
	// Create a new buffer
	outBuffer.mSize = inSize;

	VkBufferCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	create_info.size = inSize;
	create_info.usage = inUsage;
	create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	if (VKFailed(vkCreateBuffer(mDevice, &create_info, nullptr, &outBuffer.mBuffer)))
	{
		outBuffer.mBuffer = VK_NULL_HANDLE;
		return false;
	}

	VkMemoryRequirements mem_requirements;
	vkGetBufferMemoryRequirements(mDevice, outBuffer.mBuffer, &mem_requirements);

	if (mem_requirements.size > cMaxAllocSize)
	{
		// Allocate block directly
		Ref<MemoryVK> memory_vk = new MemoryVK();
		memory_vk->mBufferSize = mem_requirements.size;
		AllocateMemory(mem_requirements.size, mem_requirements.memoryTypeBits, inProperties, *memory_vk);
		outBuffer.mMemory = memory_vk;
		outBuffer.mOffset = 0;
	}
	else
	{
		// Round allocation to the next power of 2 so that we can use a simple block based allocator
		VkDeviceSize buffer_size = max(VkDeviceSize(GetNextPowerOf2(uint32(mem_requirements.size))), cMinAllocSize);

		// Ensure that we have memory available from the right pool
		Array<Memory> &mem_array = mMemoryCache[{ buffer_size, inProperties }];
		if (mem_array.empty())
		{
			// Allocate a bigger block
			Ref<MemoryVK> memory_vk = new MemoryVK();
			memory_vk->mBufferSize = buffer_size;
			AllocateMemory(cBlockSize, mem_requirements.memoryTypeBits, inProperties, *memory_vk);

			// Divide into sub blocks
			for (VkDeviceSize offset = 0; offset < cBlockSize; offset += buffer_size)
				mem_array.push_back({ memory_vk, offset });
		}

		// Claim memory from the pool
		Memory &memory = mem_array.back();
		outBuffer.mMemory = memory.mMemory;
		outBuffer.mOffset = memory.mOffset;
		mem_array.pop_back();
	}

	// Bind the memory to the buffer
	vkBindBufferMemory(mDevice, outBuffer.mBuffer, outBuffer.mMemory->mMemory, outBuffer.mOffset);
	return true;
}

void ComputeSystemVKWithAllocator::FreeBuffer(BufferVK &ioBuffer)
{
	if (ioBuffer.mBuffer != VK_NULL_HANDLE)
	{
		// Destroy the buffer
		vkDestroyBuffer(mDevice, ioBuffer.mBuffer, nullptr);
		ioBuffer.mBuffer = VK_NULL_HANDLE;

		// Hand the memory back to the cache
		VkDeviceSize buffer_size = ioBuffer.mMemory->mBufferSize;
		if (buffer_size > cMaxAllocSize)
			FreeMemory(*ioBuffer.mMemory);
		else
			mMemoryCache[{ buffer_size, ioBuffer.mMemory->mProperties }].push_back({ ioBuffer.mMemory, ioBuffer.mOffset });

		ioBuffer = BufferVK();
	}
}

void *ComputeSystemVKWithAllocator::MapBuffer(BufferVK& ioBuffer)
{
	if (++ioBuffer.mMemory->mMappedCount == 1
		&& VKFailed(vkMapMemory(mDevice, ioBuffer.mMemory->mMemory, 0, VK_WHOLE_SIZE, 0, &ioBuffer.mMemory->mMappedPtr)))
	{
		ioBuffer.mMemory->mMappedCount = 0;
		return nullptr;
	}

	return static_cast<uint8 *>(ioBuffer.mMemory->mMappedPtr) + ioBuffer.mOffset;
}

void ComputeSystemVKWithAllocator::UnmapBuffer(BufferVK& ioBuffer)
{
	JPH_ASSERT(ioBuffer.mMemory->mMappedCount > 0);
	if (--ioBuffer.mMemory->mMappedCount == 0)
	{
		vkUnmapMemory(mDevice, ioBuffer.mMemory->mMemory);
		ioBuffer.mMemory->mMappedPtr = nullptr;
	}
}

JPH_NAMESPACE_END

#endif // JPH_USE_VK
