// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Geometry/OrientedBox.h>
#include <Jolt/Geometry/AABox.h>

JPH_NAMESPACE_BEGIN

/// Separating axis test between box A (axis aligned, centered at the origin) and box B (positioned by inRot in A's space).
/// Taken from: Real Time Collision Detection - Christer Ericson, Chapter 4.4.1, page 103-105.
/// The 15 axes are tested 3 at a time.
static inline bool sOverlaps(Mat44Arg inRot, Vec3Arg inHalfExtentsA, Vec3Arg inHalfExtentsB, float inEpsilon)
{
	// Add in an epsilon term to counteract arithmetic errors when two edges are parallel and their cross product is (near) null (see text for details)
	Vec3 epsilon = Vec3::sReplicate(inEpsilon);

	// Columns of the rotation (B's axes in A's space) and their absolute values
	Vec3 c0 = inRot.GetAxisX(), c1 = inRot.GetAxisY(), c2 = inRot.GetAxisZ();
	Vec3 abs_c0 = c0.Abs() + epsilon, abs_c1 = c1.Abs() + epsilon, abs_c2 = c2.Abs() + epsilon;

	Vec3 t = inRot.GetTranslation();
	Vec3 bx = inHalfExtentsB.Swizzle<SWIZZLE_X, SWIZZLE_X, SWIZZLE_X>(), by = inHalfExtentsB.Swizzle<SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_Y>(), bz = inHalfExtentsB.Swizzle<SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_Z>();

	// Test axes L = A0, L = A1, L = A2
	Vec3 rb = abs_c0 * bx + abs_c1 * by + abs_c2 * bz;
	UVec4 separated = Vec3::sLess(inHalfExtentsA + rb, t.Abs());
	if (separated.TestAnyXYZTrue())
		return false;

	// Rows of the rotation and their absolute values
	Mat44 rot_t = inRot.Transposed3x3();
	Vec3 r0 = rot_t.GetAxisX(), r1 = rot_t.GetAxisY(), r2 = rot_t.GetAxisZ();
	Vec3 abs_r0 = r0.Abs() + epsilon, abs_r1 = r1.Abs() + epsilon, abs_r2 = r2.Abs() + epsilon;

	Vec3 tx = t.Swizzle<SWIZZLE_X, SWIZZLE_X, SWIZZLE_X>(), ty = t.Swizzle<SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_Y>(), tz = t.Swizzle<SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_Z>();
	Vec3 ax = inHalfExtentsA.Swizzle<SWIZZLE_X, SWIZZLE_X, SWIZZLE_X>(), ay = inHalfExtentsA.Swizzle<SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_Y>(), az = inHalfExtentsA.Swizzle<SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_Z>();

	// Test axes L = B0, L = B1, L = B2
	Vec3 ra = abs_r0 * ax + abs_r1 * ay + abs_r2 * az;
	Vec3 dist = r0 * tx + r1 * ty + r2 * tz;
	separated = Vec3::sLess(ra + inHalfExtentsB, dist.Abs());
	if (separated.TestAnyXYZTrue())
		return false;

	// Radius of B projected on axis L = Ai x Bj, first calculated for all i per j and then transposed to get all j per i
	Mat44 rb_cross_t(Vec4(abs_c2 * by + abs_c1 * bz), Vec4(abs_c2 * bx + abs_c0 * bz), Vec4(abs_c1 * bx + abs_c0 * by), Vec4::sZero());
	Mat44 rb_cross = rb_cross_t.Transposed3x3();

	// Test axes L = A0 x B0, L = A0 x B1, L = A0 x B2
	ra = abs_r2 * ay + abs_r1 * az;
	separated = Vec3::sLess(ra + rb_cross.GetAxisX(), (r1 * tz - r2 * ty).Abs());

	// Test axes L = A1 x B0, L = A1 x B1, L = A1 x B2
	ra = abs_r2 * ax + abs_r0 * az;
	separated = UVec4::sOr(separated, Vec3::sLess(ra + rb_cross.GetAxisY(), (r2 * tx - r0 * tz).Abs()));

	// Test axes L = A2 x B0, L = A2 x B1, L = A2 x B2
	ra = abs_r1 * ax + abs_r0 * ay;
	separated = UVec4::sOr(separated, Vec3::sLess(ra + rb_cross.GetAxisZ(), (r0 * ty - r1 * tx).Abs()));

	// If no separating axis is found, the boxes must be intersecting
	return !separated.TestAnyXYZTrue();
}

bool OrientedBox::Overlaps(const AABox &inBox, float inEpsilon) const
{
	// Note that the code is swapped around: A is the aabox and B is the oriented box (this saves us from having to invert the orientation of the oriented box)
	Mat44 rot(mOrientation.GetColumn4(0), mOrientation.GetColumn4(1), mOrientation.GetColumn4(2), mOrientation.GetColumn4(3) - Vec4(inBox.GetCenter(), 0));
	return sOverlaps(rot, inBox.GetExtent(), mHalfExtents, inEpsilon);
}

bool OrientedBox::Overlaps(const OrientedBox &inBox, float inEpsilon) const
{
	// Note that A is this, B is inBox
	return sOverlaps(mOrientation.InversedRotationTranslation() * inBox.mOrientation, mHalfExtents, inBox.mHalfExtents, inEpsilon);
}

JPH_NAMESPACE_END
