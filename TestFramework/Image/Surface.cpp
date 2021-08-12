// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Image/Surface.h>

//////////////////////////////////////////////////////////////////////////////////////////
// FormatDescription
//
// Description of a surface format
//////////////////////////////////////////////////////////////////////////////////////////

// Format descriptions
static FormatDescription sFormats[] =
{
	//				   Decription	BPP		#CMP	Closest 8 Bit				Closest Alpha				Red Mask	Green Mask	Blue Mask	Alpha Mask
	FormatDescription("A4L4",		8,		2,		ESurfaceFormat::A8L8,		ESurfaceFormat::A4L4,		0x0000000f, 0x0000000f, 0x0000000f, 0x000000f0),
	FormatDescription("L8",			8,		1,		ESurfaceFormat::L8,			ESurfaceFormat::A8L8,		0x000000ff,	0x000000ff,	0x000000ff, 0x00000000),
	FormatDescription("A8",			8,		1,		ESurfaceFormat::A8,			ESurfaceFormat::A8,			0x00000000,	0x00000000,	0x00000000, 0x000000ff),
	FormatDescription("A8L8",		16,		2,		ESurfaceFormat::A8L8,		ESurfaceFormat::A8L8,		0x000000ff, 0x000000ff, 0x000000ff, 0x0000ff00),
	FormatDescription("R5G6B5",		16,		3,		ESurfaceFormat::R8G8B8,		ESurfaceFormat::A1R5G5B5,	0x0000f800,	0x000007e0,	0x0000001f, 0x00000000),
	FormatDescription("X1R5G5B5",	16,		3,		ESurfaceFormat::R8G8B8,		ESurfaceFormat::A1R5G5B5,	0x00007c00,	0x000003e0,	0x0000001f, 0x00000000),
	FormatDescription("X4R4G4B4",	16,		3,		ESurfaceFormat::R8G8B8,		ESurfaceFormat::A4R4G4B4,	0x00000f00,	0x000000f0,	0x0000000f, 0x00000000),
	FormatDescription("A1R5G5B5",	16,		4,		ESurfaceFormat::A8R8G8B8,	ESurfaceFormat::A1R5G5B5,	0x00007c00,	0x000003e0,	0x0000001f, 0x00008000),
	FormatDescription("A4R4G4B4",	16,		4,		ESurfaceFormat::A8R8G8B8,	ESurfaceFormat::A4R4G4B4,	0x00000f00,	0x000000f0,	0x0000000f, 0x0000f000),
	FormatDescription("R8G8B8",		24,		3,		ESurfaceFormat::R8G8B8,		ESurfaceFormat::A8R8G8B8,	0x00ff0000,	0x0000ff00,	0x000000ff, 0x00000000),
	FormatDescription("B8G8R8",		24,		3,		ESurfaceFormat::B8G8R8,		ESurfaceFormat::A8B8G8R8,	0x000000ff,	0x0000ff00,	0x00ff0000, 0x00000000),
	FormatDescription("X8R8G8B8",	32,		3,		ESurfaceFormat::X8R8G8B8,	ESurfaceFormat::A8R8G8B8,	0x00ff0000,	0x0000ff00,	0x000000ff, 0x00000000),
	FormatDescription("X8B8G8R8",	32,		3,		ESurfaceFormat::X8B8G8R8,	ESurfaceFormat::A8B8G8R8,	0x000000ff,	0x0000ff00,	0x00ff0000, 0x00000000),
	FormatDescription("A8R8G8B8",	32,		4,		ESurfaceFormat::A8R8G8B8,	ESurfaceFormat::A8R8G8B8,	0x00ff0000,	0x0000ff00,	0x000000ff, 0xff000000),
	FormatDescription("A8B8G8R8",	32,		4,		ESurfaceFormat::A8B8G8R8,	ESurfaceFormat::A8B8G8R8,	0x000000ff,	0x0000ff00,	0x00ff0000, 0xff000000),
	FormatDescription("Invalid",	0,		0,		ESurfaceFormat::Invalid,	ESurfaceFormat::Invalid,	0x00000000,	0x00000000,	0x00000000, 0x00000000),
};

FormatDescription::FormatDescription(const char *inFormatName, int inBitsPerPixel, int inNumberOfComponents, ESurfaceFormat inClosest8BitFormat, ESurfaceFormat inClosestAlphaFormat, uint32 inRedMask, uint32 inGreenMask, uint32 inBlueMask, uint32 inAlphaMask) :
	mFormatName(inFormatName),
	mBitsPerPixel(inBitsPerPixel),
	mNumberOfComponents(inNumberOfComponents),
	mClosest8BitFormat(inClosest8BitFormat),
	mClosestAlphaFormat(inClosestAlphaFormat),
	mRedMask(inRedMask),
	mGreenMask(inGreenMask),
	mBlueMask(inBlueMask),
	mAlphaMask(inAlphaMask)
{
}

uint32 FormatDescription::Encode(ColorArg inColor) const
{
	uint32 col = 0;
	uint32 written_mask = 0;

	// Loop through all components
	for (int c = 0; c < 4; ++c)
	{
		// Check that we have not yet written this part of the color yet
		uint32 mask = GetComponentMask(c);
		if ((written_mask & mask) != 0) continue;
		written_mask |= mask;
			
		// Or in this component
		col |= int(round((1.0f / 255.0f) * mask * inColor(c))) & mask;
	}

	return col;
}

const Color FormatDescription::Decode(uint32 inColor) const
{
	Color col(0, 0, 0, 0);

	// Loop through all components
	for (int c = 0; c < 4; ++c)
	{
		uint32 mask = GetComponentMask(c);		
		if (mask != 0)
		{
			uint32 shift = CountTrailingZeros(mask);
			uint32 shifted_color = (inColor & mask) >> shift;
			uint32 shifted_mask = mask >> shift;
			col(c) = uint8((255 * shifted_color + 127) / shifted_mask);
		}
		else
			col(c) = 255;
	}

	return col;
}

const FormatDescription &GetFormatDescription(ESurfaceFormat inFormat)
{
	if (inFormat <= ESurfaceFormat::Invalid)
		return sFormats[uint(inFormat)];

	return sFormats[uint(ESurfaceFormat::Invalid)];
}

//////////////////////////////////////////////////////////////////////////////////////////
// Surface
//
// Class that contains an image in arbitrary format
//////////////////////////////////////////////////////////////////////////////////////////

Surface::Surface(int inWidth, int inHeight, ESurfaceFormat inFormat) :
	mFormat(inFormat),
	mWidth(inWidth),
	mHeight(inHeight),
	mLength(0),
	mLockMode(ESurfaceLockMode::None),
	mStride(0),
	mData(nullptr)
{
}

Surface::~Surface()
{
	JPH_ASSERT(!IsLocked());
	JPH_ASSERT(mData == nullptr);
	JPH_ASSERT(mStride == 0);
	JPH_ASSERT(mLength == 0);
}

void Surface::Lock(ESurfaceLockMode inMode) const
{
	// Check if this resource can be locked
	JPH_ASSERT(!IsLocked()); 
	JPH_ASSERT((uint(inMode) & uint(ESurfaceLockMode::ReadWrite)) != 0);

	// Store mode
	mLockMode = inMode;
		
	// Lock the buffer
	HardwareLock();	

	// Check that data and stride were filled in
	JPH_ASSERT(mData != nullptr);
	JPH_ASSERT(mStride > 0);
	JPH_ASSERT(mLength > 0);
}

void Surface::UnLock() const
{
	// Check if this resource was locked
	JPH_ASSERT(IsLocked());
	
	// Unlock the hardware resource
	HardwareUnLock();

	// Reset members, so we are sure they will be set next time
	mLockMode = ESurfaceLockMode::None; 	
	mStride = 0;
	mLength = 0;
	mData = nullptr;
}

void Surface::Clear(ColorArg inColor)
{
	Lock(ESurfaceLockMode::Write);

	// Get image properties
	int bpp = GetBytesPerPixel();
	int width = GetWidth();
	int height = GetHeight();

	// Determine clear color
	uint32 col = GetFormatDescription().Encode(inColor);

	// Clear the image
	for (int y = 0; y < height; ++y)
	{
		uint8 *d		= GetScanLine(y);
		uint8 *d_end	= GetScanLine(y) + width * bpp;

		while (d < d_end)
		{
			memcpy(d, &col, bpp);
			d += bpp;
		}		
	}

	UnLock();
}

//////////////////////////////////////////////////////////////////////////////////////////
// SoftwareSurface
//
// Class that contains an image in arbitrary format
//////////////////////////////////////////////////////////////////////////////////////////

SoftwareSurface::SoftwareSurface(int inWidth, int inHeight, ESurfaceFormat inFormat, int inStride) :
	Surface(inWidth, inHeight, inFormat)
{
	// Determine stride and length
	mPixelStride = inStride == 0? ((mWidth * GetBytesPerPixel() + 3) & ~3) : inStride;
	mPixelLength = mPixelStride * inHeight;

	// Allocate pixel data
	JPH_ASSERT(mPixelLength > 0);
	mPixelData = new uint8 [mPixelLength];	
}

SoftwareSurface::~SoftwareSurface()
{
	delete mPixelData;
}

void SoftwareSurface::HardwareLock() const
{
	// Get pointer to data
	mData = mPixelData;	
	mStride = mPixelStride;
	mLength = mPixelLength;
}

void SoftwareSurface::HardwareUnLock() const
{
}

