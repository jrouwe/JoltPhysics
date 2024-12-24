// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Renderer/DX12/ConstantBufferDX12.h>
#include <Renderer/DX12/RendererDX12.h>
#include <Renderer/DX12/FatalErrorIfFailedDX12.h>

ConstantBufferDX12::ConstantBufferDX12(RendererDX12 *inRenderer, uint64 inBufferSize) :
	mRenderer(inRenderer)
{
	mBuffer = mRenderer->CreateD3DResourceOnUploadHeap(inBufferSize);
	mBufferSize = inBufferSize;
}

ConstantBufferDX12::~ConstantBufferDX12()
{
	if (mBuffer != nullptr)
		mRenderer->RecycleD3DResourceOnUploadHeap(mBuffer.Get(), mBufferSize);
}

void *ConstantBufferDX12::MapInternal()
{
	void *mapped_resource;
	D3D12_RANGE range = { 0, 0 }; // We're not going to read
	FatalErrorIfFailed(mBuffer->Map(0, &range, &mapped_resource));
	return mapped_resource;
}

void ConstantBufferDX12::Unmap()
{
	mBuffer->Unmap(0, nullptr);
}

void ConstantBufferDX12::Bind(int inSlot)
{
	mRenderer->GetCommandList()->SetGraphicsRootConstantBufferView(inSlot, mBuffer->GetGPUVirtualAddress());
}
