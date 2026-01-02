// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#ifdef JPH_USE_VK

#include <Jolt/Compute/VK/ComputeQueueVK.h>
#include <Jolt/Compute/VK/ComputeBufferVK.h>
#include <Jolt/Compute/VK/ComputeSystemVK.h>

JPH_NAMESPACE_BEGIN

ComputeQueueVK::~ComputeQueueVK()
{
	Wait();

	VkDevice device = mComputeSystem->GetDevice();

	if (mCommandBuffer != VK_NULL_HANDLE)
		vkFreeCommandBuffers(device, mCommandPool, 1, &mCommandBuffer);

	if (mCommandPool != VK_NULL_HANDLE)
		vkDestroyCommandPool(device, mCommandPool, nullptr);

	if (mDescriptorPool != VK_NULL_HANDLE)
		vkDestroyDescriptorPool(device, mDescriptorPool, nullptr);

	if (mFence != VK_NULL_HANDLE)
		vkDestroyFence(device, mFence, nullptr);
}

bool ComputeQueueVK::Initialize(uint32 inComputeQueueIndex, ComputeQueueResult &outResult)
{
	// Get the queue
	VkDevice device = mComputeSystem->GetDevice();
	vkGetDeviceQueue(device, inComputeQueueIndex, 0, &mQueue);

	// Create a command pool
	VkCommandPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	pool_info.queueFamilyIndex = inComputeQueueIndex;
	if (VKFailed(vkCreateCommandPool(device, &pool_info, nullptr, &mCommandPool), outResult))
		return false;

	// Create descriptor pool
	VkDescriptorPoolSize descriptor_pool_sizes[] = {
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1024 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 16 * 1024 },
	};
	VkDescriptorPoolCreateInfo descriptor_info = {};
	descriptor_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptor_info.poolSizeCount = (uint32)std::size(descriptor_pool_sizes);
	descriptor_info.pPoolSizes = descriptor_pool_sizes;
	descriptor_info.maxSets = 256;
	if (VKFailed(vkCreateDescriptorPool(device, &descriptor_info, nullptr, &mDescriptorPool), outResult))
		return false;

	// Create a command buffer
	VkCommandBufferAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.commandPool = mCommandPool;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandBufferCount = 1;
	if (VKFailed(vkAllocateCommandBuffers(device, &alloc_info, &mCommandBuffer), outResult))
		return false;

	// Create a fence
	VkFenceCreateInfo fence_info = {};
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	if (VKFailed(vkCreateFence(device, &fence_info, nullptr, &mFence), outResult))
		return false;

	return true;
}

bool ComputeQueueVK::BeginCommandBuffer()
{
	if (!mCommandBufferRecording)
	{
		VkCommandBufferBeginInfo begin_info = {};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		if (VKFailed(vkBeginCommandBuffer(mCommandBuffer, &begin_info)))
			return false;
		mCommandBufferRecording = true;
	}
	return true;
}

void ComputeQueueVK::SetShader(const ComputeShader *inShader)
{
	mShader = static_cast<const ComputeShaderVK *>(inShader);
	mBufferInfos = mShader->GetBufferInfos();
}

void ComputeQueueVK::SetConstantBuffer(const char *inName, const ComputeBuffer *inBuffer)
{
	if (inBuffer == nullptr)
		return;
	JPH_ASSERT(inBuffer->GetType() == ComputeBuffer::EType::ConstantBuffer);

	if (!BeginCommandBuffer())
		return;

	const ComputeBufferVK *buffer = static_cast<const ComputeBufferVK *>(inBuffer);
	buffer->Barrier(mCommandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_UNIFORM_READ_BIT, false);

	uint index = mShader->NameToBufferInfoIndex(inName);
	JPH_ASSERT(mShader->GetLayoutBindings()[index].descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	mBufferInfos[index].buffer = buffer->GetBufferCPU();

	mUsedBuffers.insert(buffer);
}

void ComputeQueueVK::SyncCPUToGPU(const ComputeBufferVK *inBuffer)
{
	// Ensure that any CPU writes are visible to the GPU
	if (inBuffer->SyncCPUToGPU(mCommandBuffer)
		&& (inBuffer->GetType() == ComputeBuffer::EType::Buffer || inBuffer->GetType()  == ComputeBuffer::EType::RWBuffer))
	{
		// After the first upload, the CPU buffer is no longer needed for Buffer and RWBuffer types
		mDelayedFreedBuffers.push_back(inBuffer->ReleaseBufferCPU());
	}
}

void ComputeQueueVK::SetBuffer(const char *inName, const ComputeBuffer *inBuffer)
{
	if (inBuffer == nullptr)
		return;
	JPH_ASSERT(inBuffer->GetType() == ComputeBuffer::EType::UploadBuffer || inBuffer->GetType() == ComputeBuffer::EType::Buffer || inBuffer->GetType() == ComputeBuffer::EType::RWBuffer);

	if (!BeginCommandBuffer())
		return;

	const ComputeBufferVK *buffer = static_cast<const ComputeBufferVK *>(inBuffer);
	SyncCPUToGPU(buffer);
	buffer->Barrier(mCommandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT, false);

	uint index = mShader->NameToBufferInfoIndex(inName);
	JPH_ASSERT(mShader->GetLayoutBindings()[index].descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	mBufferInfos[index].buffer = buffer->GetBufferGPU();

	mUsedBuffers.insert(buffer);
}

void ComputeQueueVK::SetRWBuffer(const char *inName, ComputeBuffer *inBuffer, EBarrier inBarrier)
{
	if (inBuffer == nullptr)
		return;
	JPH_ASSERT(inBuffer->GetType() == ComputeBuffer::EType::RWBuffer);

	if (!BeginCommandBuffer())
		return;

	const ComputeBufferVK *buffer = static_cast<const ComputeBufferVK *>(inBuffer);
	SyncCPUToGPU(buffer);
	buffer->Barrier(mCommandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VkAccessFlagBits(VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT), inBarrier == EBarrier::Yes);

	uint index = mShader->NameToBufferInfoIndex(inName);
	JPH_ASSERT(mShader->GetLayoutBindings()[index].descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	mBufferInfos[index].buffer = buffer->GetBufferGPU();

	mUsedBuffers.insert(buffer);
}

void ComputeQueueVK::ScheduleReadback(ComputeBuffer *inDst, const ComputeBuffer *inSrc)
{
	if (inDst == nullptr || inSrc == nullptr)
		return;
	JPH_ASSERT(inDst->GetType() == ComputeBuffer::EType::ReadbackBuffer);

	if (!BeginCommandBuffer())
		return;

	const ComputeBufferVK *src_vk = static_cast<const ComputeBufferVK *>(inSrc);
	const ComputeBufferVK *dst_vk = static_cast<ComputeBufferVK *>(inDst);

	// Barrier to start reading from GPU buffer and writing to CPU buffer
	src_vk->Barrier(mCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_READ_BIT, false);
	dst_vk->Barrier(mCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, false);

	// Copy
	VkBufferCopy copy = {};
	copy.srcOffset = 0;
	copy.dstOffset = 0;
	copy.size = src_vk->GetSize() * src_vk->GetStride();
	vkCmdCopyBuffer(mCommandBuffer, src_vk->GetBufferGPU(), dst_vk->GetBufferCPU(), 1, &copy);

	// Barrier to indicate that CPU can read from the buffer
	dst_vk->Barrier(mCommandBuffer, VK_PIPELINE_STAGE_HOST_BIT, VK_ACCESS_HOST_READ_BIT, false);

	mUsedBuffers.insert(src_vk);
	mUsedBuffers.insert(dst_vk);
}

void ComputeQueueVK::Dispatch(uint inThreadGroupsX, uint inThreadGroupsY, uint inThreadGroupsZ)
{
	if (!BeginCommandBuffer())
		return;

	vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, mShader->GetPipeline());

	VkDevice device = mComputeSystem->GetDevice();
	const Array<VkDescriptorSetLayoutBinding> &ds_bindings = mShader->GetLayoutBindings();
	if (!ds_bindings.empty())
	{
		// Create a descriptor set
		VkDescriptorSetAllocateInfo alloc_info = {};
		alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		alloc_info.descriptorPool = mDescriptorPool;
		alloc_info.descriptorSetCount = 1;
		VkDescriptorSetLayout ds_layout = mShader->GetDescriptorSetLayout();
		alloc_info.pSetLayouts = &ds_layout;
		VkDescriptorSet descriptor_set;
		if (VKFailed(vkAllocateDescriptorSets(device, &alloc_info, &descriptor_set)))
			return;

		// Write the values to the descriptor set
		Array<VkWriteDescriptorSet> writes;
		writes.reserve(ds_bindings.size());
		for (uint32 i = 0; i < (uint32)ds_bindings.size(); ++i)
		{
			VkWriteDescriptorSet w = {};
			w.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			w.dstSet = descriptor_set;
			w.dstBinding = ds_bindings[i].binding;
			w.dstArrayElement = 0;
			w.descriptorCount = ds_bindings[i].descriptorCount;
			w.descriptorType = ds_bindings[i].descriptorType;
			w.pBufferInfo = &mBufferInfos[i];
			writes.push_back(w);
		}
		vkUpdateDescriptorSets(device, (uint32)writes.size(), writes.data(), 0, nullptr);

		// Bind the descriptor set
		vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, mShader->GetPipelineLayout(), 0, 1, &descriptor_set, 0, nullptr);
	}

	vkCmdDispatch(mCommandBuffer, inThreadGroupsX, inThreadGroupsY, inThreadGroupsZ);
}

void ComputeQueueVK::Execute()
{
	// End command buffer
	if (!mCommandBufferRecording)
		return;
	if (VKFailed(vkEndCommandBuffer(mCommandBuffer)))
		return;
	mCommandBufferRecording = false;

	// Reset fence
	VkDevice device = mComputeSystem->GetDevice();
	if (VKFailed(vkResetFences(device, 1, &mFence)))
		return;

	// Submit
	VkSubmitInfo submit = {};
	submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit.commandBufferCount = 1;
	submit.pCommandBuffers = &mCommandBuffer;
	if (VKFailed(vkQueueSubmit(mQueue, 1, &submit, mFence)))
		return;

	// Clear the current shader
	mShader = nullptr;

	// Mark that we're executing
	mIsExecuting = true;
}

void ComputeQueueVK::Wait()
{
	if (!mIsExecuting)
		return;

	// Wait for the work to complete
	VkDevice device = mComputeSystem->GetDevice();
	if (VKFailed(vkWaitForFences(device, 1, &mFence, VK_TRUE, UINT64_MAX)))
		return;

	// Reset command buffer so it can be reused
	if (mCommandBuffer != VK_NULL_HANDLE)
		vkResetCommandBuffer(mCommandBuffer, 0);

	// Allow reusing the descriptors for next run
	vkResetDescriptorPool(device, mDescriptorPool, 0);

	// Buffers can be freed now
	mUsedBuffers.clear();

	// Free delayed buffers
	for (BufferVK &buffer : mDelayedFreedBuffers)
		mComputeSystem->FreeBuffer(buffer);
	mDelayedFreedBuffers.clear();

	mIsExecuting = false;
}

JPH_NAMESPACE_END

#endif // JPH_USE_VK
