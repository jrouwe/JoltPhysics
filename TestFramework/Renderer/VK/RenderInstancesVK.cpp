// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Renderer/VK/RenderInstancesVK.h>
#include <Renderer/VK/RenderPrimitiveVK.h>
#include <Renderer/VK/FatalErrorIfFailedVK.h>

void RenderInstancesVK::Clear()
{
	mRenderer->FreeBuffer(mInstancesBuffer);
}

void RenderInstancesVK::CreateBuffer(int inNumInstances, int inInstanceSize)
{
	Clear();

	mRenderer->CreateBuffer(VkDeviceSize(inNumInstances) * inInstanceSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, mInstancesBuffer);
}

void *RenderInstancesVK::Lock()
{
	void *data;
	FatalErrorIfFailed(vkMapMemory(mRenderer->GetDevice(), mInstancesBuffer.mMemory, mInstancesBuffer.mOffset, mInstancesBuffer.mSize, 0, &data));
	return data;
}

void RenderInstancesVK::Unlock()
{
	vkUnmapMemory(mRenderer->GetDevice(), mInstancesBuffer.mMemory);
}

void RenderInstancesVK::Draw(RenderPrimitive *inPrimitive, int inStartInstance, int inNumInstances) const
{
	if (inNumInstances <= 0)
		return;

	VkCommandBuffer command_buffer = mRenderer->GetCommandBuffer();
	RenderPrimitiveVK *primitive = static_cast<RenderPrimitiveVK *>(inPrimitive);

	VkBuffer buffers[] = { primitive->mVertexBuffer.mBuffer, mInstancesBuffer.mBuffer };
	VkDeviceSize offsets[] = { 0, 0 };
	vkCmdBindVertexBuffers(command_buffer, 0, 2, buffers, offsets);

	if (primitive->mIndexBuffer.mBuffer == VK_NULL_HANDLE)
	{
		vkCmdDraw(command_buffer, primitive->mNumVtxToDraw, inNumInstances, 0, inStartInstance);
	}
	else
	{
		vkCmdBindIndexBuffer(command_buffer, primitive->mIndexBuffer.mBuffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(command_buffer, primitive->mNumIdxToDraw, inNumInstances, 0, 0, inStartInstance);
	}
}
