// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Renderer/VK/ConstantBufferVK.h>
#include <Renderer/VK/RendererVK.h>
#include <Renderer/VK/FatalErrorIfFailedVK.h>

ConstantBufferVK::ConstantBufferVK(RendererVK *inRenderer, VkDeviceSize inBufferSize) :
	mRenderer(inRenderer)
{
	mRenderer->CreateBuffer(inBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, mBuffer);
}

ConstantBufferVK::~ConstantBufferVK()
{
	mRenderer->FreeBuffer(mBuffer);
}

void *ConstantBufferVK::MapInternal()
{
	void *data = nullptr;
	FatalErrorIfFailed(vkMapMemory(mRenderer->GetDevice(), mBuffer.mMemory, mBuffer.mOffset, mBuffer.mSize, 0, &data));
	return data;
}

void ConstantBufferVK::Unmap()
{
	vkUnmapMemory(mRenderer->GetDevice(), mBuffer.mMemory);
}
