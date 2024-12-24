// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Renderer/VK/RenderInstancesVK.h>
#include <Renderer/VK/RenderPrimitiveVK.h>

void RenderInstancesVK::Clear()
{
	mRenderer->FreeBuffer(mInstancesBuffer, mInstancesBufferMemory);
	mInstancesBuffer = VK_NULL_HANDLE;
	mInstancesBufferMemory = VK_NULL_HANDLE;
	mInstancesBufferSize = 0;
}

void RenderInstancesVK::CreateBuffer(int inNumInstances, int inInstanceSize)
{
	Clear();

	mInstancesBufferSize = VkDeviceSize(inNumInstances) * inInstanceSize;
	mRenderer->CreateBuffer(mInstancesBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, mInstancesBuffer, mInstancesBufferMemory);
}

void *RenderInstancesVK::Lock()
{
	void *data;
	vkMapMemory(mRenderer->GetDevice(), mInstancesBufferMemory, 0, mInstancesBufferSize, 0, &data);
	return data;
}

void RenderInstancesVK::Unlock()
{
	vkUnmapMemory(mRenderer->GetDevice(), mInstancesBufferMemory);
}

void RenderInstancesVK::Draw(RenderPrimitive *inPrimitive, int inStartInstance, int inNumInstances) const
{
	VkCommandBuffer command_buffer = mRenderer->GetCommandBuffer();
	RenderPrimitiveVK *primitive = static_cast<RenderPrimitiveVK *>(inPrimitive);

	VkBuffer buffers[] = { primitive->mVertexBuffer, mInstancesBuffer };
	VkDeviceSize offsets[] = { 0, 0 };
	vkCmdBindVertexBuffers(command_buffer, 0, 2, buffers, offsets);

	if (primitive->mIndexBuffer == nullptr)
	{
		vkCmdDraw(command_buffer, primitive->mNumVtxToDraw, inNumInstances, 0, inStartInstance);
	}
	else
	{
		vkCmdBindIndexBuffer(command_buffer, primitive->mIndexBuffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(command_buffer, primitive->mNumIdxToDraw, inNumInstances, 0, 0, inStartInstance);
	}
}
