// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Renderer/Texture.h>
#include <Image/BlitSurface.h>
#include <Renderer/Renderer.h>
#include <Renderer/FatalErrorIfFailed.h>

Texture::Texture(Renderer *inRenderer, const Surface *inSurface) :
	mRenderer(inRenderer)
{
	// Store dimensions
	mWidth = inSurface->GetWidth();
	mHeight = inSurface->GetHeight();

	// Create description
	D3D12_RESOURCE_DESC desc = {};
	desc.MipLevels = 1;
	ESurfaceFormat format = inSurface->GetFormat();
	switch (format)
	{
	case ESurfaceFormat::A4L4:			desc.Format = DXGI_FORMAT_R8G8_UNORM;		break;
	case ESurfaceFormat::L8:			desc.Format = DXGI_FORMAT_R8_UNORM;			break;
	case ESurfaceFormat::A8:			desc.Format = DXGI_FORMAT_A8_UNORM;			break;
	case ESurfaceFormat::A8L8:			desc.Format = DXGI_FORMAT_R8G8_UNORM;		break;
	case ESurfaceFormat::R5G6B5:		desc.Format = DXGI_FORMAT_B5G6R5_UNORM;		break;
	case ESurfaceFormat::X1R5G5B5:		desc.Format = DXGI_FORMAT_B5G5R5A1_UNORM;	format = ESurfaceFormat::A1R5G5B5;	break;
	case ESurfaceFormat::X4R4G4B4:		desc.Format = DXGI_FORMAT_B4G4R4A4_UNORM;	format = ESurfaceFormat::A4R4G4B4;	break;
	case ESurfaceFormat::A1R5G5B5:		desc.Format = DXGI_FORMAT_B5G5R5A1_UNORM;	break;
	case ESurfaceFormat::A4R4G4B4:		desc.Format = DXGI_FORMAT_B4G4R4A4_UNORM;	break;
	case ESurfaceFormat::R8G8B8:		desc.Format = DXGI_FORMAT_B8G8R8X8_UNORM;	format = ESurfaceFormat::X8R8G8B8;	break;
	case ESurfaceFormat::B8G8R8:		desc.Format = DXGI_FORMAT_B8G8R8X8_UNORM;	format = ESurfaceFormat::X8R8G8B8;	break;
	case ESurfaceFormat::X8R8G8B8:		desc.Format = DXGI_FORMAT_B8G8R8X8_UNORM;	break;
	case ESurfaceFormat::X8B8G8R8:		desc.Format = DXGI_FORMAT_B8G8R8X8_UNORM;	format = ESurfaceFormat::X8R8G8B8;	break;
	case ESurfaceFormat::A8R8G8B8:		desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;	break;
	case ESurfaceFormat::A8B8G8R8:		desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;	format = ESurfaceFormat::A8R8G8B8;	break;
	case ESurfaceFormat::Invalid:
	default:							JPH_ASSERT(false);							break;
	}
	desc.Width = mWidth;
	desc.Height = mHeight;
	desc.Flags = D3D12_RESOURCE_FLAG_NONE;
	desc.DepthOrArraySize = 1;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

	// Blit the surface to another temporary surface if the format changed
	const Surface *surface = inSurface;
	Ref<Surface> tmp;
	if (format != inSurface->GetFormat())
	{
		tmp = new SoftwareSurface(mWidth, mHeight, format);
		BlitSurface(inSurface, tmp);
		surface = tmp;
	}

	// Create texture in default heap
	D3D12_HEAP_PROPERTIES heap_properties = {};
	heap_properties.Type = D3D12_HEAP_TYPE_DEFAULT;
	heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heap_properties.CreationNodeMask = 1;
	heap_properties.VisibleNodeMask = 1;
	FatalErrorIfFailed(inRenderer->GetDevice()->CreateCommittedResource(&heap_properties, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&mTexture)));
	JPH_IF_DEBUG(mTexture->SetName(L"Texture");)

	// Determine required size of data to copy
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
	UINT64 row_size_in_bytes;
	UINT64 required_size = 0;
	inRenderer->GetDevice()->GetCopyableFootprints(&desc, 0, 1, 0, &footprint, nullptr, &row_size_in_bytes, &required_size);

	// Create the GPU upload buffer
	ComPtr<ID3D12Resource> upload_resource = mRenderer->CreateD3DResourceOnUploadHeap(required_size);
	JPH_IF_DEBUG(upload_resource->SetName(L"Texture Upload");)

	// Copy data to upload texture
	surface->Lock(ESurfaceLockMode::Read);
	uint8 *upload_data;
	D3D12_RANGE range = { 0, 0 }; // We're not going to read
	FatalErrorIfFailed(upload_resource->Map(0, &range, (void **)&upload_data));
	for (int y = 0; y < mHeight; ++y)
		memcpy(upload_data + y * row_size_in_bytes, surface->GetData() + y * surface->GetStride(), surface->GetBytesPerPixel() * mWidth);
	upload_resource->Unmap(0, nullptr);
	surface->UnLock();

	// Start a commandlist for the upload
	ID3D12GraphicsCommandList *list = inRenderer->GetUploadQueue().Start();

	// Copy the texture from our upload buffer to our final texture
	D3D12_TEXTURE_COPY_LOCATION copy_dst;
	copy_dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	copy_dst.pResource = mTexture.Get();
	copy_dst.SubresourceIndex = 0;
	D3D12_TEXTURE_COPY_LOCATION copy_src;
	copy_src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	copy_src.pResource = upload_resource.Get();
	copy_src.PlacedFootprint = footprint;
	list->CopyTextureRegion(&copy_dst, 0, 0, 0, &copy_src, nullptr);

	// Indicate that the texture is now ready to be used by a pixel shader
	D3D12_RESOURCE_BARRIER barrier;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = mTexture.Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	list->ResourceBarrier(1, &barrier);

	// Create a SRV for the texture
	mSRV = inRenderer->GetSRVHeap().Allocate();
	D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
	srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srv_desc.Format = desc.Format;
	srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srv_desc.Texture2D.MipLevels = 1;
	inRenderer->GetDevice()->CreateShaderResourceView(mTexture.Get(), &srv_desc, mSRV);

	// Wait for copying to finish so we can destroy the upload texture
	inRenderer->GetUploadQueue().ExecuteAndWait();

	// Recycle the upload buffer
	inRenderer->RecycleD3DResourceOnUploadHeap(upload_resource.Get(), required_size);
}

Texture::Texture(Renderer *inRenderer, int inWidth, int inHeight) :
	mRenderer(inRenderer)
{
	// Store dimensions
	mWidth = inWidth;
	mHeight = inHeight;

	// Allocate depth stencil buffer
	D3D12_CLEAR_VALUE clear_value = {};
	clear_value.Format = DXGI_FORMAT_D32_FLOAT;
	clear_value.DepthStencil.Depth = 1.0f;
	clear_value.DepthStencil.Stencil = 0;

	D3D12_HEAP_PROPERTIES heap_properties = {};
	heap_properties.Type = D3D12_HEAP_TYPE_DEFAULT;
	heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heap_properties.CreationNodeMask = 1;
	heap_properties.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC depth_stencil_desc = {};
	depth_stencil_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depth_stencil_desc.Alignment = 0;
	depth_stencil_desc.Width = mWidth;
	depth_stencil_desc.Height = mHeight;
	depth_stencil_desc.DepthOrArraySize = 1;
	depth_stencil_desc.MipLevels = 1;
	depth_stencil_desc.Format = DXGI_FORMAT_D32_FLOAT;
	depth_stencil_desc.SampleDesc.Count = 1;
	depth_stencil_desc.SampleDesc.Quality = 0;
	depth_stencil_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depth_stencil_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	FatalErrorIfFailed(inRenderer->GetDevice()->CreateCommittedResource(&heap_properties, D3D12_HEAP_FLAG_NONE, &depth_stencil_desc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, &clear_value, IID_PPV_ARGS(&mTexture)));
	JPH_IF_DEBUG(mTexture->SetName(L"Render Target Texture");)

	// Create DSV for the texture
	mDSV = inRenderer->GetDSVHeap().Allocate();
	D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc = {};
	dsv_desc.Format = DXGI_FORMAT_D32_FLOAT;
	dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsv_desc.Flags = D3D12_DSV_FLAG_NONE;
	inRenderer->GetDevice()->CreateDepthStencilView(mTexture.Get(), &dsv_desc, mDSV);

	// Create a SRV for the texture
	mSRV = inRenderer->GetSRVHeap().Allocate();
	D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
	srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srv_desc.Format = DXGI_FORMAT_R32_FLOAT;
	srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srv_desc.Texture2D.MipLevels = 1;
	inRenderer->GetDevice()->CreateShaderResourceView(mTexture.Get(), &srv_desc, mSRV);
}

Texture::~Texture()
{
	if (mSRV.ptr != 0)
		mRenderer->GetSRVHeap().Free(mSRV);

	if (mDSV.ptr != 0)
		mRenderer->GetDSVHeap().Free(mDSV);

	if (mTexture != nullptr)
		mRenderer->RecycleD3DObject(mTexture.Get());
}

void Texture::Bind(int inSlot) const
{
	mRenderer->GetCommandList()->SetGraphicsRootDescriptorTable(inSlot, mRenderer->GetSRVHeap().ConvertToGPUHandle(mSRV));
}

void Texture::ClearRenderTarget()
{
	mRenderer->GetCommandList()->ClearDepthStencilView(mDSV, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

void Texture::SetAsRenderTarget(bool inSet) const
{
	if (inSet)
	{
		// Indicate make the texture ready for rendering to
		D3D12_RESOURCE_BARRIER barrier;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = mTexture.Get();
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_WRITE;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		mRenderer->GetCommandList()->ResourceBarrier(1, &barrier);

		// Set as render target
		mRenderer->GetCommandList()->OMSetRenderTargets(0, nullptr, FALSE, &mDSV);

		// Set view port
		D3D12_VIEWPORT viewport = { 0.0f, 0.0f, static_cast<float>(mWidth), static_cast<float>(mHeight), 0.0f, 1.0f };
		mRenderer->GetCommandList()->RSSetViewports(1, &viewport);

		// Set scissor rect
		D3D12_RECT scissor_rect = { 0, 0, static_cast<LONG>(mWidth), static_cast<LONG>(mHeight) };
		mRenderer->GetCommandList()->RSSetScissorRects(1, &scissor_rect);
	}
	else
	{
		// Indicate that the texture is now ready to be used by a pixel shader
		D3D12_RESOURCE_BARRIER barrier;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = mTexture.Get();
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_DEPTH_WRITE;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		mRenderer->GetCommandList()->ResourceBarrier(1, &barrier);
	}
}
