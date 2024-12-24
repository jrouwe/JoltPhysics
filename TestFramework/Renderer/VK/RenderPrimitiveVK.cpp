// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Renderer/VK/RenderPrimitiveVK.h>
#include <Renderer/VK/FatalErrorIfFailedVK.h>

void RenderPrimitiveVK::ReleaseVertexBuffer()
{
	mRenderer->FreeBuffer(mVertexBuffer, mVertexBufferMemory);
	mVertexBufferDeviceLocal = false;

	RenderPrimitive::ReleaseVertexBuffer();
}

void RenderPrimitiveVK::ReleaseIndexBuffer()
{
	mRenderer->FreeBuffer(mIndexBuffer, mIndexBufferMemory);
	mIndexBufferDeviceLocal = false;

	RenderPrimitive::ReleaseIndexBuffer();
}

void RenderPrimitiveVK::CreateVertexBuffer(int inNumVtx, int inVtxSize, const void *inData)
{
	RenderPrimitive::CreateVertexBuffer(inNumVtx, inVtxSize, inData);

	VkDeviceSize size = VkDeviceSize(inNumVtx) * inVtxSize;
	if (inData != nullptr)
	{
		mRenderer->CreateDeviceLocalBuffer(inData, size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, mVertexBuffer, mVertexBufferMemory);
		mVertexBufferDeviceLocal = true;
	}
	else
		mRenderer->CreateBuffer(size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, mVertexBuffer, mVertexBufferMemory);
}

void *RenderPrimitiveVK::LockVertexBuffer()
{
	JPH_ASSERT(!mVertexBufferDeviceLocal);

	void *data;
	vkMapMemory(mRenderer->GetDevice(), mVertexBufferMemory, 0, VkDeviceSize(mNumVtx) * mVtxSize, 0, &data);
	return data;
}

void RenderPrimitiveVK::UnlockVertexBuffer()
{
	vkUnmapMemory(mRenderer->GetDevice(), mVertexBufferMemory);
}

void RenderPrimitiveVK::CreateIndexBuffer(int inNumIdx, const uint32 *inData)
{
	RenderPrimitive::CreateIndexBuffer(inNumIdx, inData);

	VkDeviceSize size = VkDeviceSize(inNumIdx) * sizeof(uint32);
	if (inData != nullptr)
	{
		mRenderer->CreateDeviceLocalBuffer(inData, size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, mIndexBuffer, mIndexBufferMemory);
		mIndexBufferDeviceLocal = true;
	}
	else
		mRenderer->CreateBuffer(size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, mIndexBuffer, mIndexBufferMemory);
}

uint32 *RenderPrimitiveVK::LockIndexBuffer()
{
	JPH_ASSERT(!mIndexBufferDeviceLocal);

	void *data;
	vkMapMemory(mRenderer->GetDevice(), mIndexBufferMemory, 0, VkDeviceSize(mNumIdx) * sizeof(uint32), 0, &data);
	return reinterpret_cast<uint32 *>(data);
}

void RenderPrimitiveVK::UnlockIndexBuffer()
{
	vkUnmapMemory(mRenderer->GetDevice(), mIndexBufferMemory);
}

void RenderPrimitiveVK::Draw() const
{
	VkCommandBuffer command_buffer = mRenderer->GetCommandBuffer();

	VkBuffer vertex_buffers[] = { mVertexBuffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);

	if (mIndexBuffer == nullptr)
	{
		vkCmdDraw(command_buffer, mNumVtxToDraw, 1, 0, 0);
	}
	else
	{
		vkCmdBindIndexBuffer(command_buffer, mIndexBuffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(command_buffer, mNumIdxToDraw, 1, 0, 0, 0);
	}
}
