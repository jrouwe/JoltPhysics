// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Geometry/Plane.h>
#include <Jolt/Geometry/AABox.h>

/// A camera frustum containing of 6 planes (left, right, top, bottom, near, far) pointing inwards
class [[nodiscard]] Frustum
{
public:
	/// Empty constructor
					Frustum() = default;

	/// Construct frustom from position, forward, up, field of view x and y and near and far plane. 
	/// Note that inUp does not need to be perpendicular to inForward but cannot be collinear.
	inline			Frustum(Vec3Arg inPosition, Vec3Arg inForward, Vec3Arg inUp, float inFOVX, float inFOVY, float inNear, float inFar)
	{
		Vec3 right = inForward.Cross(inUp).Normalized();
		Vec3 up = right.Cross(inForward).Normalized(); // Calculate the real up vector (inUp does not need to be perpendicular to inForward)

		// Near and far plane
		mPlanes[0] = Plane::sFromPointAndNormal(inPosition + inNear * inForward, inForward);
		mPlanes[1] = Plane::sFromPointAndNormal(inPosition + inFar * inForward, -inForward);

		// Top and bottom planes
		mPlanes[2] = Plane::sFromPointAndNormal(inPosition, Mat44::sRotation(right, 0.5f * inFOVY) * -up);
		mPlanes[3] = Plane::sFromPointAndNormal(inPosition, Mat44::sRotation(right, -0.5f * inFOVY) * up);

		// Left and right planes
		mPlanes[4] = Plane::sFromPointAndNormal(inPosition, Mat44::sRotation(up, 0.5f * inFOVX) * right);
		mPlanes[5] = Plane::sFromPointAndNormal(inPosition, Mat44::sRotation(up, -0.5f * inFOVX) * -right);
	}

	/// Test if frustum overlaps with axis aligned box. Note that this is a conservative estimate and can return true if the
	/// frustum doesn't actually overlap with the box. This is because we only test the plane axis as separating axis
	/// and skip checking the cross products of the edges of the frustum
	inline bool		Overlaps(const AABox &inBox) const
	{
		// Loop over all frustum planes
		for (const Plane &p : mPlanes)
		{
			// Get support point (the maximum extent) in the direction of our normal
			Vec3 support = inBox.GetSupport(p.GetNormal());

			// If this is behind our plane, the box is not inside the frustum
			if (p.SignedDistance(support) < 0.0f)
				return false;
		}

		return true;
	}

private:
	Plane			mPlanes[6];																	///< Planes forming the frustum
};
