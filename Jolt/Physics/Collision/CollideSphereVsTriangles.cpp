// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt.h>

#include <Physics/Collision/CollideSphereVsTriangles.h>
#include <Physics/Collision/Shape/ScaleHelpers.h>
#include <Physics/Collision/CollideShape.h>
#include <Physics/Collision/TransformedShape.h>
#include <Physics/Collision/ActiveEdges.h>
#include <Physics/Collision/NarrowPhaseStats.h>
#include <Core/Profiler.h>

namespace JPH {

CollideSphereVsTriangles::CollideSphereVsTriangles(const SphereShape *inShape1, Vec3Arg inScale1, Vec3Arg inScale2, Mat44Arg inCenterOfMassTransform1, Mat44Arg inCenterOfMassTransform2, const SubShapeID &inSubShapeID1, const CollideShapeSettings &inCollideShapeSettings, CollideShapeCollector &ioCollector) :
	mCollideShapeSettings(inCollideShapeSettings),
	mCollector(ioCollector),
	mShape1(inShape1),
	mScale2(inScale2),
	mTransform2(inCenterOfMassTransform2),
	mSubShapeID1(inSubShapeID1)
{
	// Calculate the center of the sphere in the space of 2
	mSphereCenterIn2 = inCenterOfMassTransform2.Multiply3x3Transposed(inCenterOfMassTransform1.GetTranslation() - inCenterOfMassTransform2.GetTranslation());

	// Determine if shape 2 is inside out or not
	mScaleSign2 = ScaleHelpers::IsInsideOut(inScale2)? -1.0f : 1.0f;

	// Check that the sphere is uniformly scaled
	JPH_ASSERT(ScaleHelpers::IsUniformScale(inScale1.Abs()));
	mRadius = abs(inScale1.GetX()) * inShape1->GetRadius();
	mRadiusPlusMaxSeparationSq = Square(mRadius + inCollideShapeSettings.mMaxSeparationDistance);
}

void CollideSphereVsTriangles::Collide(Vec3Arg inV0, Vec3Arg inV1, Vec3Arg inV2, uint8 inActiveEdges, const SubShapeID &inSubShapeID2)
{
	JPH_PROFILE_FUNCTION();

	// Scale triangle and make it relative to the center of the sphere
	Vec3 v0 = mScale2 * inV0 - mSphereCenterIn2; 
	Vec3 v1 = mScale2 * inV1 - mSphereCenterIn2;
	Vec3 v2 = mScale2 * inV2 - mSphereCenterIn2;

	// Calculate triangle normal
	Vec3 triangle_normal = mScaleSign2 * (v1 - v0).Cross(v2 - v0);

	// Backface check
	bool back_facing = triangle_normal.Dot(v0) > 0.0f;
	if (mCollideShapeSettings.mBackFaceMode == EBackFaceMode::IgnoreBackFaces && back_facing)
		return;

	// Check if we collide with the sphere
	uint32 closest_feature;
	Vec3 point2 = ClosestPoint::GetClosestPointOnTriangle(v0, v1, v2, closest_feature);
	float point2_len_sq = point2.LengthSq();
	if (point2_len_sq > mRadiusPlusMaxSeparationSq)
		return;

	// Calculate penetration depth
	float penetration_depth = mRadius - sqrt(point2_len_sq);
	if (-penetration_depth >= mCollector.GetEarlyOutFraction())
		return;

	// Calculate penetration axis, direction along which to push 2 to move it out of collision (this is always away from the sphere center)
	Vec3 penetration_axis = point2.NormalizedOr(Vec3::sAxisY());

	// Calculate the point on the sphere
	Vec3 point1 = mRadius * penetration_axis;

	// Check if we have enabled active edge detection
	if (mCollideShapeSettings.mActiveEdgeMode == EActiveEdgeMode::CollideOnlyWithActive && inActiveEdges != 0b111)
	{
		// Convert the active edge velocity hint to local space
		Vec3 active_edge_movement_direction = mTransform2.Multiply3x3Transposed(mCollideShapeSettings.mActiveEdgeMovementDirection);

		// Update the penetration axis to account for active edges
		// Note that we flip the triangle normal as the penetration axis is pointing towards the triangle instead of away
		penetration_axis = ActiveEdges::FixNormal(v0, v1, v2, back_facing? triangle_normal : -triangle_normal, inActiveEdges, point2, penetration_axis, active_edge_movement_direction);
	}

	// Convert to world space
	point1 = mTransform2 * (mSphereCenterIn2 + point1);
	point2 = mTransform2 * (mSphereCenterIn2 + point2);
	Vec3 penetration_axis_world = mTransform2.Multiply3x3(penetration_axis);

	// Create collision result
	CollideShapeResult result(point1, point2, penetration_axis_world, penetration_depth, mSubShapeID1, inSubShapeID2, TransformedShape::sGetBodyID(mCollector.GetContext()));

	// Note: We don't gather faces here because that's only useful if both shapes have a face. Since the sphere always has only 1 contact point, the manifold is always a point.

	// Notify the collector
	JPH_IF_TRACK_NARROWPHASE_STATS(TrackNarrowPhaseCollector track;)
	mCollector.AddHit(result);
}

} // JPH