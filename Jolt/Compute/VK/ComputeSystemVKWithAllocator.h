// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#ifdef JPH_USE_VK

#include <Jolt/Compute/VK/ComputeSystemVK.h>
#include <Jolt/Core/UnorderedMap.h>

JPH_NAMESPACE_BEGIN

/// This extends ComputeSystemVK to provide a default implementation for memory allocation and mapping.
/// It uses a simple block based allocator to reduce the number of allocations done to Vulkan.
class JPH_EXPORT ComputeSystemVKWithAllocator : public ComputeSystemVK
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_EXPORT, ComputeSystemVKWithAllocator)

	/// Allow the application to override buffer creation and memory mapping in case it uses its own allocator
	virtual bool					CreateBuffer(VkDeviceSize inSize, VkBufferUsageFlags inUsage, VkMemoryPropertyFlags inProperties, BufferVK &outBuffer) override;
	virtual void					FreeBuffer(BufferVK &ioBuffer) override;
	virtual void *					MapBuffer(BufferVK &ioBuffer) override;
	virtual void					UnmapBuffer(BufferVK &ioBuffer) override;

protected:
	virtual bool					InitializeMemory() override;
	virtual void					ShutdownMemory() override;

	uint32							FindMemoryType(uint32 inTypeFilter, VkMemoryPropertyFlags inProperties) const;
	void							AllocateMemory(VkDeviceSize inSize, uint32 inMemoryTypeBits, VkMemoryPropertyFlags inProperties, MemoryVK &ioMemory);
	void							FreeMemory(MemoryVK &ioMemory);

	VkPhysicalDeviceMemoryProperties mMemoryProperties;

private:
	// Smaller allocations (from cMinAllocSize to cMaxAllocSize) will be done in blocks of cBlockSize bytes.
	// We do this because there is a limit to the number of allocations that we can make in Vulkan.
	static constexpr VkDeviceSize	cMinAllocSize = 512;
	static constexpr VkDeviceSize	cMaxAllocSize = 65536;
	static constexpr VkDeviceSize	cBlockSize = 524288;

	struct MemoryKey
	{
		bool						operator == (const MemoryKey &inRHS) const
		{
			return mSize == inRHS.mSize && mProperties == inRHS.mProperties;
		}

		VkDeviceSize				mSize;
		VkMemoryPropertyFlags		mProperties;
	};

	JPH_MAKE_HASH_STRUCT(MemoryKey, MemoryKeyHasher, t.mProperties, t.mSize)

	struct Memory
	{
		Ref<MemoryVK>				mMemory;
		VkDeviceSize				mOffset;
	};

	using MemoryCache = UnorderedMap<MemoryKey, Array<Memory>, MemoryKeyHasher>;

	MemoryCache						mMemoryCache;
};

JPH_NAMESPACE_END

#endif // JPH_USE_VK
