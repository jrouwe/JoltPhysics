// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Image/ZoomImage.h>
#include <Jolt/Core/Color.h>

/// Settings for blitting one surface to another with possibly different formats and dimensions. The blit
/// routine can use filtering or blurring on the fly. Also it can perform some other
/// basic operations like converting an image to grayscale or alpha only surfaces.
class BlitSettings
{
public:
	/// Constructor
							BlitSettings();

	/// Comparison operators
	bool					operator == (const BlitSettings &inRHS) const;

	/// Default settings
	static const BlitSettings	sDefault;

	/// Special operations that can be applied during the blit
	bool					mConvertRGBToAlpha;										///< Convert RGB values to alpha values (RGB values remain untouched)
	bool					mConvertAlphaToRGB;										///< Convert alpha values to grayscale RGB values (Alpha values remain untouched)
	bool					mConvertToGrayScale;									///< Convert RGB values to grayscale values (Alpha values remain untouched)
	bool					mInvertAlpha;											///< Invert alpha values
	bool					mColorKeyAlpha;											///< If true, colors in the range mColorKeyStart..mColorKeyEnd will get an alpha of 0, other colors will get an alpha of 255
	Color					mColorKeyStart;
	Color					mColorKeyEnd;
	ZoomSettings			mZoomSettings;											///< Settings for resizing the image
};

/// Copies an image from inSrc to inDst, converting it on the fly as defined by inBlitSettings
bool BlitSurface(RefConst<Surface> inSrc, Ref<Surface> ioDst, const BlitSettings &inBlitSettings = BlitSettings::sDefault);
