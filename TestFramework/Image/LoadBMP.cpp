// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Image/LoadBMP.h>
#include <Image/BlitSurface.h>
#include <Image/Surface.h>

#pragma pack (1)

struct BitmapFileHeader
{
	char			mTypeB;
	char			mTypeM;
	uint32			mSize;
	uint16			mReserved1;
	uint16			mReserved2;
	uint32			mOffBits;
};

struct BitmapInfoHeader
{
	uint32			mSize;
	uint32			mWidth;
	uint32			mHeight;
	uint16			mPlanes;
	uint16			mBitCount;
	uint32			mCompression;
	uint32			mSizeImage;
	uint32			mXPelsPerMeter;
	uint32			mYPelsPerMeter;
	uint32			mClrUsed;
	uint32			mClrImportant;
};

#pragma pack ()

Ref<Surface> LoadBMP(istream &inStream)
{
	bool loaded = true;

	// Read bitmap info
	BitmapFileHeader bfh;
	BitmapInfoHeader bih;
	inStream.read((char *)&bfh, sizeof(bfh));
	if (inStream.fail())
		return nullptr;
	inStream.read((char *)&bih, sizeof(bih));
	if (inStream.fail())
		return nullptr;

	// Get properties
	int bpp = (bih.mBitCount + 7) >> 3;
	int scan_width = (bih.mWidth * bpp + 3) & (~3);

	// Check if it is a bitmap
	if (bfh.mTypeB != 'B' || bfh.mTypeM != 'M')
	{
		Trace("Not a BMP");
		return nullptr;
	}

	// Check if bitmap is bottom-up
	if (bih.mHeight <= 0)
	{
		Trace("Not bottom-up");
		return nullptr;
	}

	// Check if it is not compressed
	if (bih.mCompression != 0)
	{
		Trace("Is compressed");
		return nullptr;
	}

	Ref<Surface> surface;

	if (bih.mBitCount == 8)
	{
		// Load palette
		uint32 *palette = new uint32 [256];
		int pal_bytes = 4 * (bih.mClrUsed != 0? bih.mClrUsed : 256);
		inStream.read((char *)palette, pal_bytes);
		loaded = loaded && !inStream.fail();

		// Seek to image data
		inStream.seekg(bfh.mOffBits);

		// Convert pixel data to a surface
		surface = new SoftwareSurface(bih.mWidth, bih.mHeight, ESurfaceFormat::X8R8G8B8);
		surface->Lock(ESurfaceLockMode::Write);
		uint8 *scan_line = new uint8 [scan_width];
		for (int y = bih.mHeight - 1; y >= 0; --y)
		{
			// Load one scan line
			inStream.read((char *)scan_line, scan_width);
			loaded = loaded && !inStream.fail();

			// Copy one scan line
			uint8 *in_pixel = scan_line;
			uint32 *out_pixel = (uint32 *)surface->GetScanLine(y);
			for (uint x = 0; x < bih.mWidth; ++x, ++in_pixel, ++out_pixel)
				*out_pixel = palette[*in_pixel];
		}
		surface->UnLock();

		// Release temporaries
		delete [] palette;
		delete [] scan_line;
	}
	else
	{
		// Determine pixel format
		ESurfaceFormat format;
		switch (bih.mBitCount)
		{
		case 16:	format = ESurfaceFormat::X1R5G5B5;	break;
		case 24:	format = ESurfaceFormat::R8G8B8;	break;
		default:	Trace("Has invalid format");		return nullptr;
		}

		// Seek to image data
		inStream.seekg(bfh.mOffBits);

		// Convert pixel data to a surface
		surface = new SoftwareSurface(bih.mWidth, bih.mHeight, format, scan_width);
		surface->Lock(ESurfaceLockMode::Write);
		for (int y = bih.mHeight - 1; y >= 0; --y)
		{
			inStream.read((char *)surface->GetScanLine(y), scan_width);
			loaded = loaded && !inStream.fail();
		}
		surface->UnLock();
	}

	return loaded? surface : Ref<Surface>(nullptr);
}

bool SaveBMP(RefConst<Surface> inSurface, ostream &inStream)
{
	bool stored = true;

	// Convert surface if required
	const Surface *src = inSurface;
	Ref<Surface> tmp_src;
	if (inSurface->GetFormat() != ESurfaceFormat::R8G8B8)
	{
		tmp_src = new SoftwareSurface(inSurface->GetWidth(), inSurface->GetHeight(), ESurfaceFormat::R8G8B8);
		BlitSurface(inSurface, tmp_src);
		src = tmp_src.GetPtr();
	}

	// Lock the surface
	src->Lock(ESurfaceLockMode::Read);
	JPH_ASSERT(src->GetStride() % 4 == 0);

	BitmapFileHeader bfh;
	BitmapInfoHeader bih;

	// Fill in headers
	bfh.mTypeB				= 'B';
	bfh.mTypeM				= 'M';
	bfh.mSize				= sizeof(bfh) + sizeof(bih) + src->GetHeight() * src->GetStride();
	bfh.mReserved1			= 0;
	bfh.mReserved2			= 0;
	bfh.mOffBits			= sizeof(bfh) + sizeof(bih);

	bih.mSize				= sizeof(bih);
	bih.mWidth				= src->GetWidth();
	bih.mHeight				= src->GetHeight();
	bih.mPlanes				= 1;
	bih.mBitCount			= 24;
	bih.mCompression		= 0;
	bih.mSizeImage			= src->GetHeight() * src->GetStride();
	bih.mXPelsPerMeter		= 300;
	bih.mYPelsPerMeter		= 300;
	bih.mClrUsed			= 0;
	bih.mClrImportant		= 0;

	// Write headers
	inStream.write((char *)&bfh, sizeof(bfh));
	stored = stored && !inStream.fail();
	inStream.write((char *)&bih, sizeof(bih));
	stored = stored && !inStream.fail();

	// Write image data
	for (int y = src->GetHeight() - 1; y >= 0; --y)
	{
		inStream.write((const char *)src->GetScanLine(y), src->GetStride());
		stored = stored && !inStream.fail();
	}

	src->UnLock();

	return stored;
}
