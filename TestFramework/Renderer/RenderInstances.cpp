// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Renderer/RenderInstances.h>
#include <Renderer/RenderPrimitive.h>
#include <Renderer/FatalErrorIfFailed.h>

void RenderInstances::Clear()
{
	if (mInstanceBuffer != nullptr)
		mRenderer->RecycleD3DResourceOnUploadHeap(mInstanceBuffer.Get(), mInstanceBufferSize);

	mInstanceBuffer = nullptr;
	mInstanceBufferSize = 0;
	mInstanceSize = 0;
}

void RenderInstances::CreateBuffer(int inNumInstances, int inInstanceSize)
{
	if (mInstanceBuffer == nullptr || mInstanceBufferSize < inNumInstances * inInstanceSize)
	{
		// Delete the old buffer
		Clear();

		// Calculate size
		mInstanceBufferSize = inNumInstances * inInstanceSize;

		// Create buffer
		mInstanceBuffer = mRenderer->CreateD3DResourceOnUploadHeap(mInstanceBufferSize);
		JPH_IF_DEBUG(mInstanceBuffer->SetName(L"Instance Buffer");)
	}

	// Update parameters
	mInstanceSize = inInstanceSize;
}

void *RenderInstances::Lock()
{
	uint32 *mapped_resource;
	D3D12_RANGE range = { 0, 0 };
	mInstanceBuffer->Map(0, &range, (void **)&mapped_resource);
	return mapped_resource;
}

void RenderInstances::Unlock()
{
	mInstanceBuffer->Unmap(0, nullptr);
}
	
void RenderInstances::Draw(RenderPrimitive *inPrimitive, int inStartInstance, int inNumInstances) const
{
	if (inNumInstances <= 0)
		return;

	ID3D12GraphicsCommandList *command_list = mRenderer->GetCommandList();

	// Set topology
	command_list->IASetPrimitiveTopology(inPrimitive->mType);

	D3D12_VERTEX_BUFFER_VIEW vb_view[2];

	// Vertex buffer
	vb_view[0].BufferLocation = inPrimitive->mVtxBuffer->GetGPUVirtualAddress();
	vb_view[0].StrideInBytes = inPrimitive->mVtxSize;
	vb_view[0].SizeInBytes = inPrimitive->mNumVtxToDraw * inPrimitive->mVtxSize;

	// Instances buffer
	vb_view[1].BufferLocation = mInstanceBuffer->GetGPUVirtualAddress();
	vb_view[1].StrideInBytes = mInstanceSize;
	vb_view[1].SizeInBytes = mInstanceBufferSize;

	command_list->IASetVertexBuffers(0, 2, &vb_view[0]);

	if (inPrimitive->mIdxBuffer == nullptr)
	{
		// Draw instanced primitive
		command_list->DrawInstanced(inPrimitive->mNumVtxToDraw, inNumInstances, 0, inStartInstance);
	}
	else
	{
		// Set index buffer
		D3D12_INDEX_BUFFER_VIEW ib_view;
		ib_view.BufferLocation = inPrimitive->mIdxBuffer->GetGPUVirtualAddress();
		ib_view.SizeInBytes = inPrimitive->mNumIdxToDraw * sizeof(uint32);
		ib_view.Format = DXGI_FORMAT_R32_UINT;
		command_list->IASetIndexBuffer(&ib_view);

		// Draw instanced primitive
		command_list->DrawIndexedInstanced(inPrimitive->mNumIdxToDraw, inNumInstances, 0, 0, inStartInstance);
	}
}
