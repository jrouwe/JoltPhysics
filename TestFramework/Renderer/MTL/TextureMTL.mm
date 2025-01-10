// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Renderer/MTL/TextureMTL.h>
#include <Renderer/MTL/RendererMTL.h>
#include <Renderer/MTL/FatalErrorIfFailedMTL.h>
#include <Image/BlitSurface.h>

TextureMTL::TextureMTL(RendererMTL *inRenderer, const Surface *inSurface) :
	Texture(inSurface->GetWidth(), inSurface->GetHeight()),
	mRenderer(inRenderer)
{
	ESurfaceFormat format = inSurface->GetFormat();
	MTLPixelFormat mt_format = MTLPixelFormatBGRA8Unorm;
	switch (format)
	{
	case ESurfaceFormat::A4L4:
	case ESurfaceFormat::A8L8:
	case ESurfaceFormat::A4R4G4B4:
	case ESurfaceFormat::R8G8B8:
	case ESurfaceFormat::B8G8R8:
	case ESurfaceFormat::X8R8G8B8:
	case ESurfaceFormat::X8B8G8R8:
	case ESurfaceFormat::A8R8G8B8:
	case ESurfaceFormat::A8B8G8R8:		mt_format = MTLPixelFormatBGRA8Unorm;			format = ESurfaceFormat::A8R8G8B8; break;
	case ESurfaceFormat::L8:			mt_format = MTLPixelFormatR8Unorm;				break;
	case ESurfaceFormat::A8:			mt_format = MTLPixelFormatA8Unorm;				break;
	case ESurfaceFormat::R5G6B5:
	case ESurfaceFormat::X1R5G5B5:
	case ESurfaceFormat::X4R4G4B4:		mt_format = MTLPixelFormatB5G6R5Unorm;			format = ESurfaceFormat::R5G6B5; break;
	case ESurfaceFormat::A1R5G5B5:		mt_format = MTLPixelFormatA1BGR5Unorm;			break;
	case ESurfaceFormat::Invalid:
	default:							JPH_ASSERT(false);								break;
	}

	// Blit the surface to another temporary surface if the format changed
	const Surface *surface = inSurface;
	Ref<Surface> tmp;
	if (format != inSurface->GetFormat())
	{
		tmp = new SoftwareSurface(mWidth, mHeight, format);
		BlitSurface(inSurface, tmp);
		surface = tmp;
	}

	// Create descriptor
	MTLTextureDescriptor *descriptor = [[MTLTextureDescriptor alloc] init];
	descriptor.textureType = MTLTextureType2D;
	descriptor.usage = MTLTextureUsageShaderRead;
	descriptor.pixelFormat = mt_format;
	descriptor.width = mWidth;
	descriptor.height = mHeight;
	descriptor.storageMode = MTLStorageModeManaged;

	MTLRegion region =
	{
		{ 0, 0, 0 },
		{ NSUInteger(mWidth), NSUInteger(mHeight), 1}
	};

	// Create texture
	mTexture = [inRenderer->GetDevice() newTextureWithDescriptor: descriptor];
	surface->Lock(ESurfaceLockMode::Read);
	[mTexture replaceRegion: region mipmapLevel:0 withBytes: surface->GetData() bytesPerRow: surface->GetStride()];
	surface->UnLock();

	[descriptor release];
}

TextureMTL::TextureMTL(RendererMTL *inRenderer, int inWidth, int inHeight) :
	Texture(inWidth, inHeight),
	mRenderer(inRenderer)
{
	MTLTextureDescriptor *descriptor = [[MTLTextureDescriptor alloc] init];
	descriptor.textureType = MTLTextureType2D;
	descriptor.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
	descriptor.pixelFormat = MTLPixelFormatDepth32Float;
	descriptor.width = mWidth;
	descriptor.height = mHeight;
	descriptor.storageMode = MTLStorageModePrivate;

	mTexture = [inRenderer->GetDevice() newTextureWithDescriptor: descriptor];
	
	[descriptor release];
}

TextureMTL::~TextureMTL()
{
	[mTexture release];
}

void TextureMTL::Bind() const
{
	[mRenderer->GetRenderEncoder() setFragmentTexture: mTexture atIndex: 0];
}
