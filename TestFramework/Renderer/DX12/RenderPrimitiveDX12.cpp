// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Renderer/DX12/RenderPrimitiveDX12.h>
#include <Renderer/DX12/FatalErrorIfFailedDX12.h>

void RenderPrimitiveDX12::ReleaseVertexBuffer()
{
	if (mVtxBuffer != nullptr)
	{
		if (mVtxBufferInUploadHeap)
			mRenderer->RecycleD3DResourceOnUploadHeap(mVtxBuffer.Get(), mNumVtx * mVtxSize);
		else
			mRenderer->RecycleD3DObject(mVtxBuffer.Get());
		mVtxBuffer = nullptr;
	}

	mVtxBufferInUploadHeap = false;
	RenderPrimitive::ReleaseVertexBuffer();
}

void RenderPrimitiveDX12::ReleaseIndexBuffer()
{
	if (mIdxBuffer != nullptr)
	{
		if (mIdxBufferInUploadHeap)
			mRenderer->RecycleD3DResourceOnUploadHeap(mIdxBuffer.Get(), mNumIdx * sizeof(uint32));
		else
			mRenderer->RecycleD3DObject(mIdxBuffer.Get());
		mIdxBuffer = nullptr;
	}

	mIdxBufferInUploadHeap = false;
	RenderPrimitive::ReleaseIndexBuffer();
}

void RenderPrimitiveDX12::CreateVertexBuffer(int inNumVtx, int inVtxSize, const void *inData)
{
	RenderPrimitive::CreateVertexBuffer(inNumVtx, inVtxSize, inData);

	uint64 size = uint64(inNumVtx) * inVtxSize;

	if (inData != nullptr)
	{
		// Data provided, assume the buffer is static so allocate it on the GPU
		mVtxBuffer = mRenderer->CreateD3DResourceOnDefaultHeap(inData, size);
		mVtxBufferInUploadHeap = false;
	}
	else
	{
		// No data provided, create a buffer that will be uploaded to the GPU every time it is used
		mVtxBuffer = mRenderer->CreateD3DResourceOnUploadHeap(size);
		mVtxBufferInUploadHeap = true;
	}

	JPH_IF_DEBUG(mVtxBuffer->SetName(L"Vertex Buffer");)
}

void *RenderPrimitiveDX12::LockVertexBuffer()
{
	void *mapped_resource;
	D3D12_RANGE range = { 0, 0 };
	FatalErrorIfFailed(mVtxBuffer->Map(0, &range, &mapped_resource));
	return mapped_resource;
}

void RenderPrimitiveDX12::UnlockVertexBuffer()
{
	mVtxBuffer->Unmap(0, nullptr);
}

void RenderPrimitiveDX12::CreateIndexBuffer(int inNumIdx, const uint32 *inData)
{
	RenderPrimitive::CreateIndexBuffer(inNumIdx, inData);

	uint64 size = uint64(inNumIdx) * sizeof(uint32);

	if (inData != nullptr)
	{
		// Data provided, assume the buffer is static so allocate it on the GPU
		mIdxBuffer = mRenderer->CreateD3DResourceOnDefaultHeap(inData, size);
		mIdxBufferInUploadHeap = false;
	}
	else
	{
		// No data provided, create a buffer that will be uploaded to the GPU every time it is used
		mIdxBuffer = mRenderer->CreateD3DResourceOnUploadHeap(size);
		mIdxBufferInUploadHeap = true;
	}

	JPH_IF_DEBUG(mIdxBuffer->SetName(L"Index Buffer");)
}

uint32 *RenderPrimitiveDX12::LockIndexBuffer()
{
	uint32 *mapped_resource;
	D3D12_RANGE range = { 0, 0 };
	FatalErrorIfFailed(mIdxBuffer->Map(0, &range, (void **)&mapped_resource));
	return mapped_resource;
}

void RenderPrimitiveDX12::UnlockIndexBuffer()
{
	mIdxBuffer->Unmap(0, nullptr);
}

void RenderPrimitiveDX12::Draw() const
{
	ID3D12GraphicsCommandList *command_list = mRenderer->GetCommandList();

	// Set topology
	command_list->IASetPrimitiveTopology(mType == PipelineState::ETopology::Triangle? D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST : D3D_PRIMITIVE_TOPOLOGY_LINELIST);

	if (mIdxBuffer == nullptr)
	{
		// Set vertex buffer
		D3D12_VERTEX_BUFFER_VIEW vb_view;
		vb_view.BufferLocation = mVtxBuffer->GetGPUVirtualAddress();
		vb_view.StrideInBytes = mVtxSize;
		vb_view.SizeInBytes = mNumVtxToDraw * mVtxSize;
		command_list->IASetVertexBuffers(0, 1, &vb_view);

		// Draw the non indexed primitive
		command_list->DrawInstanced(mNumVtxToDraw, 1, 0, 0);
	}
	else
	{
		// Set vertex buffer
		D3D12_VERTEX_BUFFER_VIEW vb_view;
		vb_view.BufferLocation = mVtxBuffer->GetGPUVirtualAddress();
		vb_view.StrideInBytes = mVtxSize;
		vb_view.SizeInBytes = mNumVtx * mVtxSize;
		command_list->IASetVertexBuffers(0, 1, &vb_view);

		// Set index buffer
		D3D12_INDEX_BUFFER_VIEW ib_view;
		ib_view.BufferLocation = mIdxBuffer->GetGPUVirtualAddress();
		ib_view.SizeInBytes = mNumIdxToDraw * sizeof(uint32);
		ib_view.Format = DXGI_FORMAT_R32_UINT;
		command_list->IASetIndexBuffer(&ib_view);

		// Draw indexed primitive
		command_list->DrawIndexedInstanced(mNumIdxToDraw, 1, 0, 0, 0);
	}
}
