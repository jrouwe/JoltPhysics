// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Image/LoadTGA.h>
#include <Image/Surface.h>

#pragma pack (1)

struct TGAHeader
{
	uint8			mIDLength;
	uint8			mColorMapType;
	uint8			mImageType;
	uint16			mColorMapFirstEntryIndex;
	uint16			mColorMapLength;
	uint8			mColorMapEntrySize;
	uint16			mXOrigin;
	uint16			mYOrigin;
	uint16			mWidth;
	uint16			mHeight;
	uint8			mPixelDepth;
	uint8			mImageDescriptor;
};

#pragma pack ()

Ref<Surface> LoadTGA(istream &inStream)
{
	bool loaded = true;

	// Read header
	TGAHeader header;	
	inStream.read((char *)&header, sizeof(header));
	if (inStream.fail())
		return nullptr;

	// Get properties
	int bytes_per_pixel = (header.mPixelDepth + 7) >> 3;
	int scan_width = bytes_per_pixel * header.mWidth;
	
	// Check type
	if (header.mImageType < 1 || header.mImageType > 2)
	{
		Trace("Not a readable TGA");
		return nullptr;
	}

	// Check compression
	if ((header.mImageType == 1 && header.mColorMapType != 1) || (header.mImageType == 2 && header.mColorMapType != 0))
	{
		Trace("Not an uncompressed TGA");
		return nullptr;
	}

	Ref<Surface> surface;
	
	if (header.mPixelDepth == 8)
	{
		// Determine pixel format
		ESurfaceFormat format;
		int pixel_size;
		switch (header.mColorMapEntrySize)
		{
		case 15:	format = ESurfaceFormat::X1R5G5B5;	pixel_size = 2;		break;
		case 16:	format = ESurfaceFormat::X1R5G5B5;	pixel_size = 2;		break;
		case 24:	format = ESurfaceFormat::R8G8B8;	pixel_size = 3;		break;
		case 32:	format = ESurfaceFormat::A8R8G8B8;	pixel_size = 4;		break;
		default:	Trace("Has invalid format");		return nullptr;
		}

		// Seek to beginning of palette
		inStream.seekg(sizeof(TGAHeader) + header.mIDLength);
		
		// Load palette
		int pal_bytes = pixel_size * header.mColorMapLength;
		uint8 *palette = new uint8 [pal_bytes];
		inStream.read((char *)palette, pal_bytes);
		loaded = loaded && !inStream.fail();

		// Convert pixel data to a surface
		surface = new SoftwareSurface(header.mWidth, header.mHeight, format);
		surface->Lock(ESurfaceLockMode::Write);
		uint8 *scan_line = new uint8 [scan_width];
		for (int y = header.mHeight - 1; y >= 0; --y)
		{
			// Load one scan line
			inStream.read((char *)scan_line, scan_width);
			loaded = loaded && !inStream.fail();

			// Copy one scan line
			uint8 *in_pixel = scan_line;
			uint8 *out_pixel = (uint8 *)surface->GetScanLine(y);
			for (int x = 0; x < header.mWidth; ++x, ++in_pixel, out_pixel += pixel_size)
				memcpy(out_pixel, palette + (*in_pixel - header.mColorMapFirstEntryIndex) * pixel_size, pixel_size);
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
		switch (header.mPixelDepth)
		{
		case 15:	format = ESurfaceFormat::X1R5G5B5;	break;
		case 16:	format = ESurfaceFormat::X1R5G5B5;	break;
		case 24:	format = ESurfaceFormat::R8G8B8;		break;
		case 32:	format = ESurfaceFormat::A8R8G8B8;	break;
		default:	Trace("Invalid format");	return nullptr;
		}

		// Convert pixel data to a surface
		surface = new SoftwareSurface(header.mWidth, header.mHeight, format, scan_width);
		surface->Lock(ESurfaceLockMode::Write);
		for (int y = header.mHeight - 1; y >= 0; --y)
		{
			inStream.read((char *)surface->GetScanLine(y), scan_width);
			loaded = loaded && !inStream.fail();
		}
		surface->UnLock();
	}

	return loaded? surface : Ref<Surface>(nullptr);
}
