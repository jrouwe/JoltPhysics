// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Geometry/Triangle.h>
#include <Jolt/Geometry/IndexedTriangle.h>
#include <Jolt/Geometry/AABox.h>
#include <Jolt/Math/Mat44.h>

JPH_NAMESPACE_BEGIN

class AABox;

/// Oriented box
class [[nodiscard]] OrientedBox
{
public:
	/// Constructor
					OrientedBox() = default;
					OrientedBox(Mat44Arg inOrientation, Vec3Arg inHalfExtents)			: mOrientation(inOrientation), mHalfExtents(inHalfExtents) { }

	/// Construct from axis aligned box and transform. Only works for rotation/translation matrix (no scaling / shearing).
					OrientedBox(Mat44Arg inOrientation, const AABox &inBox)				: OrientedBox(inOrientation.PreTranslated(inBox.GetCenter()), inBox.GetExtent()) { }

	/// Test if oriented boxe overlaps with axis aligned box eachother
	bool			Overlaps(const AABox &inBox, float inEpsilon = 1.0e-6f) const;

	/// Test if two oriented boxes overlap eachother
	bool			Overlaps(const OrientedBox &inBox, float inEpsilon = 1.0e-6f) const;
						
	Mat44			mOrientation;														///< Transform that positions and rotates the local space axis aligned box into world space
	Vec3			mHalfExtents;														///< Half extents (half the size of the edge) of the local space axis aligned box
};

JPH_NAMESPACE_END
