// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Geometry/AABox.h>

namespace JPH {

/// Structure that holds AABox moving linearly through 3d space
struct AABoxCast
{
	AABox						mBox;						///< Axis aligned box at starting location
	Vec3						mDirection;					///< Direction and length of the cast (anything beyond this length will not be reported as a hit)
};

} // JPH