// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#ifdef JPH_USE_VK

#include <Jolt/Compute/VK/ComputeBufferVK.h>
#include <Jolt/Compute/VK/ComputeSystemVK.h>

JPH_NAMESPACE_BEGIN

ComputeBufferVK::ComputeBufferVK(ComputeSystemVK *inComputeSystem, EType inType, uint64 inSize, uint inStride) :
	ComputeBuffer(inType, inSize, inStride),
	mComputeSystem(inComputeSystem)
{
}

bool ComputeBufferVK::Initialize(const void *inData)
{
	VkDeviceSize buffer_size = VkDeviceSize(mSize * mStride);

	switch (mType)
	{
	case EType::Buffer:
		JPH_ASSERT(inData != nullptr);
		[[fallthrough]];

	case EType::UploadBuffer:
	case EType::RWBuffer:
		if (!mComputeSystem->CreateBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT, mBufferCPU))
			return false;
		if (!mComputeSystem->CreateBuffer(buffer_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mBufferGPU))
			return false;
		if (inData != nullptr)
		{
			void *data = mComputeSystem->MapBuffer(mBufferCPU);
			memcpy(data, inData, size_t(buffer_size));
			mComputeSystem->UnmapBuffer(mBufferCPU);
			mNeedsSync = true;
		}
		break;

	case EType::ConstantBuffer:
		if (!mComputeSystem->CreateBuffer(buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT, mBufferCPU))
			return false;
		if (inData != nullptr)
		{
			void* data = mComputeSystem->MapBuffer(mBufferCPU);
			memcpy(data, inData, size_t(buffer_size));
			mComputeSystem->UnmapBuffer(mBufferCPU);
		}
		break;

	case EType::ReadbackBuffer:
		JPH_ASSERT(inData == nullptr, "Can't upload data to a readback buffer");
		if (!mComputeSystem->CreateBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT, mBufferCPU))
			return false;
		break;
	}

	return true;
}

ComputeBufferVK::~ComputeBufferVK()
{
	mComputeSystem->FreeBuffer(mBufferGPU);
	mComputeSystem->FreeBuffer(mBufferCPU);
}

void ComputeBufferVK::Barrier(VkCommandBuffer inCommandBuffer, VkPipelineStageFlags inToStage, VkAccessFlagBits inToFlags, bool inForce) const
{
	if (mAccessStage == inToStage && mAccessFlagBits == inToFlags && !inForce)
		return;

	VkBufferMemoryBarrier b = {};
	b.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	b.srcAccessMask = mAccessFlagBits;
	b.dstAccessMask = inToFlags;
	b.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	b.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	b.buffer = mBufferGPU.mBuffer != VK_NULL_HANDLE? mBufferGPU.mBuffer : mBufferCPU.mBuffer;
	b.offset = 0;
	b.size = VK_WHOLE_SIZE;
	vkCmdPipelineBarrier(inCommandBuffer, mAccessStage, inToStage, 0, 0, nullptr, 1, &b, 0, nullptr);

	mAccessStage = inToStage;
	mAccessFlagBits = inToFlags;
}

bool ComputeBufferVK::SyncCPUToGPU(VkCommandBuffer inCommandBuffer) const
{
	if (!mNeedsSync)
		return false;

	// Barrier before write
	Barrier(inCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, false);

	// Copy from CPU to GPU
	VkBufferCopy copy = {};
	copy.srcOffset = 0;
	copy.dstOffset = 0;
	copy.size = GetSize() * GetStride();
	vkCmdCopyBuffer(inCommandBuffer, mBufferCPU.mBuffer, mBufferGPU.mBuffer, 1, &copy);

	mNeedsSync = false;
	return true;
}

void *ComputeBufferVK::MapInternal(EMode inMode)
{
	switch (inMode)
	{
	case EMode::Read:
		JPH_ASSERT(mType == EType::ReadbackBuffer);
		break;

	case EMode::Write:
		JPH_ASSERT(mType == EType::UploadBuffer || mType == EType::ConstantBuffer);
		mNeedsSync = true;
		break;
	}

	return mComputeSystem->MapBuffer(mBufferCPU);
}

void ComputeBufferVK::UnmapInternal()
{
	mComputeSystem->UnmapBuffer(mBufferCPU);
}

ComputeBufferResult ComputeBufferVK::CreateReadBackBuffer() const
{
	return mComputeSystem->CreateComputeBuffer(ComputeBuffer::EType::ReadbackBuffer, mSize, mStride);
}

JPH_NAMESPACE_END

#endif // JPH_USE_VK
