// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Image/BlitSurface.h>
#include <Image/Surface.h>
#include <Jolt/Core/Color.h>
#include <Jolt/Core/Profiler.h>

//////////////////////////////////////////////////////////////////////////////////////////
// BlitSettings
//////////////////////////////////////////////////////////////////////////////////////////

const BlitSettings BlitSettings::sDefault;

BlitSettings::BlitSettings() : 
	mConvertRGBToAlpha(false), 
	mConvertAlphaToRGB(false), 
	mConvertToGrayScale(false), 
	mInvertAlpha(false), 
	mColorKeyAlpha(false),
	mColorKeyStart(240, 0, 240),
	mColorKeyEnd(255, 15, 255)
{ 
}

bool BlitSettings::operator == (const BlitSettings &inRHS) const
{ 
	return mConvertRGBToAlpha == inRHS.mConvertRGBToAlpha 
		&& mConvertAlphaToRGB == inRHS.mConvertAlphaToRGB
		&& mConvertToGrayScale == inRHS.mConvertToGrayScale 
		&& mInvertAlpha == inRHS.mInvertAlpha
		&& mColorKeyAlpha == inRHS.mColorKeyAlpha
		&& mColorKeyStart == inRHS.mColorKeyStart
		&& mColorKeyEnd == inRHS.mColorKeyEnd
		&& mZoomSettings == inRHS.mZoomSettings;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Converting from one format to another
//////////////////////////////////////////////////////////////////////////////////////////

// The macro COL(s) converts color s to another color given the mapping tables
#define CMP(s, c)	map[256 * c + ((s & src_mask[c]) >> src_shift[c])]
#define COL(s)		(CMP(s, 0) + CMP(s, 1) + CMP(s, 2) + CMP(s, 3))

static void sComputeTranslationTable(const FormatDescription & inSrcDesc, const FormatDescription & inDstDesc, uint32 *outMask, uint32 *outShift, uint32 *outMap)
{
	JPH_PROFILE("sComputeTranslationTable");
	
	// Compute translation tables for each color component
	uint32 written_mask = 0;
	for (int c = 0; c < 4; ++c)
	{
		outMask[c] = inSrcDesc.GetComponentMask(c);
		outShift[c] = CountTrailingZeros(outMask[c]);
		uint32 src_shifted_mask = outMask[c] >> outShift[c];

		uint32 dst_mask = inDstDesc.GetComponentMask(c);
		uint32 dst_shift = CountTrailingZeros(dst_mask);
		uint32 dst_shifted_mask = dst_mask >> dst_shift;

		if ((written_mask & dst_mask) != 0)
		{
			dst_mask = 0;
			dst_shift = 0;
			dst_shifted_mask = 0;
		}
		else
			written_mask |= dst_mask;
		
		float scale = float(dst_shifted_mask) / src_shifted_mask;

		uint32 entry = 0;

		if (src_shifted_mask != 0)
			for (; entry <= src_shifted_mask; ++entry)
				outMap[256 * c + entry] = uint32(round(scale * entry)) << dst_shift;

		for (; entry < 256; ++entry)
			outMap[256 * c + entry] = dst_mask;
	}
}

static bool sConvertImageDifferentTypes(RefConst<Surface> inSrc, Ref<Surface> ioDst)
{
	JPH_PROFILE("sConvertImageDifferentTypes");
	
	// Get image properties
	int sbpp = inSrc->GetBytesPerPixel();
	int dbpp = ioDst->GetBytesPerPixel();
	int width = inSrc->GetWidth();
	int height = inSrc->GetHeight();
	JPH_ASSERT(width == ioDst->GetWidth());
	JPH_ASSERT(height == ioDst->GetHeight());

	// Compute conversion map
	uint32 src_mask[4];
	uint32 src_shift[4];
	uint32 map[4 * 256];
	sComputeTranslationTable(inSrc->GetFormatDescription(), ioDst->GetFormatDescription(), src_mask, src_shift, map);

	inSrc->Lock(ESurfaceLockMode::Read);
	ioDst->Lock(ESurfaceLockMode::Write);

	// Convert the image
	for (int y = 0; y < height; ++y)
	{
		const uint8 *s		= inSrc->GetScanLine(y);
		const uint8 *s_end	= inSrc->GetScanLine(y) + width * sbpp;
		uint8 *d			= ioDst->GetScanLine(y);

		while (s < s_end)
		{
			uint32 src = 0;
			memcpy(&src, s, sbpp);
			uint32 dst = COL(src);
			memcpy(d, &dst, dbpp);
			s += sbpp;
			d += dbpp;
		}
	}

	inSrc->UnLock();
	ioDst->UnLock();

	return true;
}

static bool sConvertImageSameTypes(RefConst<Surface> inSrc, Ref<Surface> ioDst)
{
	JPH_PROFILE("sConvertImageSameTypes");

	// Get image properties
	int dbpp = ioDst->GetBytesPerPixel();
	int width = inSrc->GetWidth();
	int height = inSrc->GetHeight();
	JPH_ASSERT(inSrc->GetFormat() == ioDst->GetFormat());
	JPH_ASSERT(dbpp == inSrc->GetBytesPerPixel());
	JPH_ASSERT(width == ioDst->GetWidth());
	JPH_ASSERT(height == ioDst->GetHeight());

	inSrc->Lock(ESurfaceLockMode::Read);
	ioDst->Lock(ESurfaceLockMode::Write);

	// Copy the image line by line to compensate for stride
	for (int y = 0; y < height; ++y)
		memcpy(ioDst->GetScanLine(y), inSrc->GetScanLine(y), width * dbpp);			

	inSrc->UnLock();
	ioDst->UnLock();

	return true;
}

static bool sConvertImage(RefConst<Surface> inSrc, Ref<Surface> ioDst)
{	
	JPH_PROFILE("sConvertImage");

	if (inSrc->GetFormat() == ioDst->GetFormat())
		return sConvertImageSameTypes(inSrc, ioDst);
	else
		return sConvertImageDifferentTypes(inSrc, ioDst);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Special color conversions
//////////////////////////////////////////////////////////////////////////////////////////

static void sConvertRGBToAlpha(Ref<Surface> ioSurface)
{
	JPH_PROFILE("sConvertRGBToAlpha");

	// Check surface format
	JPH_ASSERT(ioSurface->GetFormat() == ESurfaceFormat::A8R8G8B8);

	// Get dimensions of image
	int width = ioSurface->GetWidth();
	int height = ioSurface->GetHeight();

	// Convert RGB values to alpha values
	for (int y = 0; y < height; ++y)
	{
		Color *c		= (Color *)ioSurface->GetScanLine(y);
		Color *c_end	= (Color *)(ioSurface->GetScanLine(y) + width * sizeof(Color));

		while (c < c_end)
		{
			c->a = c->GetIntensity();
			++c;
		}
	}
}

static void sConvertAlphaToRGB(Ref<Surface> ioSurface)
{
	JPH_PROFILE("sConvertAlphaToRGB");

	// Check surface format
	JPH_ASSERT(ioSurface->GetFormat() == ESurfaceFormat::A8R8G8B8);

	// Get dimensions of image
	int width = ioSurface->GetWidth();
	int height = ioSurface->GetHeight();

	// Convert alpha values to RGB values
	for (int y = 0; y < height; ++y)
	{
		Color *c		= (Color *)ioSurface->GetScanLine(y);
		Color *c_end	= (Color *)(ioSurface->GetScanLine(y) + width * sizeof(Color));

		while (c < c_end)
		{
			c->r = c->g = c->b = c->a;
			++c;
		}
	}
}

static void sConvertToGrayScale(Ref<Surface> ioSurface)
{
	JPH_PROFILE("sConvertToGrayScale");

	// Check surface format
	JPH_ASSERT(ioSurface->GetFormat() == ESurfaceFormat::A8R8G8B8);

	// Get dimensions of image
	int width = ioSurface->GetWidth();
	int height = ioSurface->GetHeight();

	// Convert RGB values to grayscale values
	for (int y = 0; y < height; ++y)
	{
		Color *c		= (Color *)ioSurface->GetScanLine(y);
		Color *c_end	= (Color *)(ioSurface->GetScanLine(y) + width * sizeof(Color));

		while (c < c_end)
		{
			uint8 intensity = c->GetIntensity();
			c->r = intensity;
			c->g = intensity;
			c->b = intensity;
			++c;
		}
	}
}

static void sInvertAlpha(Ref<Surface> ioSurface)
{
	JPH_PROFILE("sInvertAlpha");

	// Check surface format
	JPH_ASSERT(ioSurface->GetFormat() == ESurfaceFormat::A8R8G8B8);

	// Get dimensions of image
	int width = ioSurface->GetWidth();
	int height = ioSurface->GetHeight();

	// Invert all alpha values
	for (int y = 0; y < height; ++y)
	{
		Color *c		= (Color *)ioSurface->GetScanLine(y);
		Color *c_end	= (Color *)(ioSurface->GetScanLine(y) + width * sizeof(Color));

		while (c < c_end)
		{
			c->a = uint8(255 - c->a);
			++c;
		}
	}
}

static void sColorKeyAlpha(Ref<Surface> ioSurface, ColorArg inStart, ColorArg inEnd)
{
	JPH_PROFILE("sColorKeyAlpha");

	// Check surface format
	JPH_ASSERT(ioSurface->GetFormat() == ESurfaceFormat::A8R8G8B8);

	// Get dimensions of image
	int width = ioSurface->GetWidth();
	int height = ioSurface->GetHeight();

	// Set alpha values
	for (int y = 0; y < height; ++y)
	{
		Color *c		= (Color *)ioSurface->GetScanLine(y);
		Color *c_end	= (Color *)(ioSurface->GetScanLine(y) + width * sizeof(Color));

		while (c < c_end)
		{
			if (c->r >= inStart.r && c->r <= inEnd.r && c->g >= inStart.g && c->g <= inEnd.g && c->b >= inStart.b && c->b <= inEnd.b)
				c->a = 0;
			else
				c->a = 255;
			++c;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
// BlitSurface
//////////////////////////////////////////////////////////////////////////////////////////

bool BlitSurface(RefConst<Surface> inSrc, Ref<Surface> ioDst, const BlitSettings &inBlitSettings)
{
	JPH_PROFILE("BlitSurface");

	// Do extra conversion options
	RefConst<Surface> src = inSrc;
	if (inBlitSettings.mConvertRGBToAlpha || inBlitSettings.mConvertAlphaToRGB || inBlitSettings.mConvertToGrayScale || inBlitSettings.mInvertAlpha || inBlitSettings.mColorKeyAlpha)
	{
		// Do them on A8R8G8B8 format so the conversion routines are simple
		Ref<Surface> tmp = new SoftwareSurface(inSrc->GetWidth(), inSrc->GetHeight(), ESurfaceFormat::A8R8G8B8);
		sConvertImage(inSrc, tmp);
		src = tmp;		

		// Perform all optional conversions
		tmp->Lock(ESurfaceLockMode::ReadWrite);

		if (inBlitSettings.mConvertRGBToAlpha)
			sConvertRGBToAlpha(tmp);

		if (inBlitSettings.mConvertAlphaToRGB)
			sConvertAlphaToRGB(tmp);

		if (inBlitSettings.mConvertToGrayScale)
			sConvertToGrayScale(tmp);

		if (inBlitSettings.mInvertAlpha)
			sInvertAlpha(tmp);

		if (inBlitSettings.mColorKeyAlpha)
			sColorKeyAlpha(tmp, inBlitSettings.mColorKeyStart, inBlitSettings.mColorKeyEnd);

		tmp->UnLock();
	}

	if (src->GetWidth() != ioDst->GetWidth() || src->GetHeight() != ioDst->GetHeight())
	{
		// Zoom the image if the destination size is not equal to the source size
		if (!ZoomImage(src, ioDst, inBlitSettings.mZoomSettings))
			return false;
	}
	else
	{
		// Convert the image if the sizes are equal
		if (!sConvertImage(src, ioDst))
			return false;
	}

	return true;
}

