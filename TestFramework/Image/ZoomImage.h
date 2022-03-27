// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Core/Reference.h>

class Surface;

/// Filter function used to rescale the image
enum EFilter
{
	FilterBox,
	FilterTriangle,
	FilterBell,
	FilterBSpline,
	FilterLanczos3,
	FilterMitchell,
};

/// Zoom settings for ZoomImage
class ZoomSettings
{
public:
	/// Constructor									
								ZoomSettings();

	/// Comparison operators
	bool						operator == (const ZoomSettings &inRHS) const;
	
	/// Default settings
	static const ZoomSettings	sDefault;

	EFilter						mFilter;												///< Filter function for image scaling
	bool						mWrapFilter;											///< If true, the filter will be applied wrapping around the image, this provides better results for repeating textures
	float						mBlur;													///< If > 1 then the image will be blurred, if < 1 the image will be sharpened
};	

/// Function to resize an image
bool ZoomImage(RefConst<Surface> inSrc, Ref<Surface> ioDst, const ZoomSettings &inZoomSettings = ZoomSettings::sDefault);
