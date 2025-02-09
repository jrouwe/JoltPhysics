// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Renderer/DX12/RenderInstancesDX12.h>
#include <Renderer/DX12/RenderPrimitiveDX12.h>
#include <Renderer/DX12/FatalErrorIfFailedDX12.h>

void RenderInstancesDX12::Clear()
{
	if (mInstanceBuffer != nullptr)
		mRenderer->RecycleD3DResourceOnUploadHeap(mInstanceBuffer.Get(), mInstanceBufferSize);

	mInstanceBuffer = nullptr;
	mInstanceBufferSize = 0;
	mInstanceSize = 0;
}

void RenderInstancesDX12::CreateBuffer(int inNumInstances, int inInstanceSize)
{
	uint new_size = uint(inNumInstances) * inInstanceSize;
	if (mInstanceBuffer == nullptr || mInstanceBufferSize < new_size)
	{
		// Delete the old buffer
		Clear();

		// Calculate size
		mInstanceBufferSize = new_size;

		// Create buffer
		mInstanceBuffer = mRenderer->CreateD3DResourceOnUploadHeap(mInstanceBufferSize);
		JPH_IF_DEBUG(mInstanceBuffer->SetName(L"Instance Buffer");)
	}

	// Update parameters
	mInstanceSize = inInstanceSize;
}

void *RenderInstancesDX12::Lock()
{
	uint32 *mapped_resource;
	D3D12_RANGE range = { 0, 0 };
	mInstanceBuffer->Map(0, &range, (void **)&mapped_resource);
	return mapped_resource;
}

void RenderInstancesDX12::Unlock()
{
	mInstanceBuffer->Unmap(0, nullptr);
}

void RenderInstancesDX12::Draw(RenderPrimitive *inPrimitive, int inStartInstance, int inNumInstances) const
{
	if (inNumInstances <= 0)
		return;

	RenderPrimitiveDX12 *primitive = static_cast<RenderPrimitiveDX12 *>(inPrimitive);

	ID3D12GraphicsCommandList *command_list = mRenderer->GetCommandList();

	// Set topology
	command_list->IASetPrimitiveTopology(primitive->mType == PipelineState::ETopology::Triangle? D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST : D3D_PRIMITIVE_TOPOLOGY_LINELIST);

	D3D12_VERTEX_BUFFER_VIEW vb_view[2];

	// Vertex buffer
	vb_view[0].BufferLocation = primitive->mVtxBuffer->GetGPUVirtualAddress();
	vb_view[0].StrideInBytes = primitive->mVtxSize;
	vb_view[0].SizeInBytes = primitive->mNumVtxToDraw * primitive->mVtxSize;

	// Instances buffer
	vb_view[1].BufferLocation = mInstanceBuffer->GetGPUVirtualAddress();
	vb_view[1].StrideInBytes = mInstanceSize;
	vb_view[1].SizeInBytes = mInstanceBufferSize;

	command_list->IASetVertexBuffers(0, 2, &vb_view[0]);

	if (primitive->mIdxBuffer == nullptr)
	{
		// Draw instanced primitive
		command_list->DrawInstanced(primitive->mNumVtxToDraw, inNumInstances, 0, inStartInstance);
	}
	else
	{
		// Set index buffer
		D3D12_INDEX_BUFFER_VIEW ib_view;
		ib_view.BufferLocation = primitive->mIdxBuffer->GetGPUVirtualAddress();
		ib_view.SizeInBytes = primitive->mNumIdxToDraw * sizeof(uint32);
		ib_view.Format = DXGI_FORMAT_R32_UINT;
		command_list->IASetIndexBuffer(&ib_view);

		// Draw instanced primitive
		command_list->DrawIndexedInstanced(primitive->mNumIdxToDraw, inNumInstances, 0, 0, inStartInstance);
	}
}
