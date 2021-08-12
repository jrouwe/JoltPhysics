// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt.h>

#include <Physics/Collision/CollideConvexVsTriangles.h>
#include <Physics/Collision/Shape/ScaleHelpers.h>
#include <Physics/Collision/CollideShape.h>
#include <Physics/Collision/TransformedShape.h>
#include <Physics/Collision/ActiveEdges.h>
#include <Core/StatCollector.h>
#include <Geometry/EPAPenetrationDepth.h>
#include <Geometry/Plane.h>

namespace JPH {

#ifdef JPH_STAT_COLLECTOR
alignas(JPH_CACHE_LINE_SIZE) atomic<int> CollideConvexVsTriangles::sNumCollideChecks { 0 };
alignas(JPH_CACHE_LINE_SIZE) atomic<int> CollideConvexVsTriangles::sNumGJKChecks { 0 };
alignas(JPH_CACHE_LINE_SIZE) atomic<int> CollideConvexVsTriangles::sNumEPAChecks { 0 };
alignas(JPH_CACHE_LINE_SIZE) atomic<int> CollideConvexVsTriangles::sNumCollisions { 0 };
#endif // JPH_STAT_COLLECTOR

CollideConvexVsTriangles::CollideConvexVsTriangles(const ConvexShape *inShape1, Vec3Arg inScale1, Vec3Arg inScale2, Mat44Arg inCenterOfMassTransform1, Mat44Arg inCenterOfMassTransform2, const SubShapeID &inSubShapeID1, const CollideShapeSettings &inCollideShapeSettings, CollideShapeCollector &ioCollector) :
	mCollideShapeSettings(inCollideShapeSettings),
	mCollector(ioCollector),
	mShape1(inShape1),
	mScale1(inScale1),
	mScale2(inScale2),
	mTransform1(inCenterOfMassTransform1),
	mSubShapeID1(inSubShapeID1)
{
	// Get transforms
	Mat44 inverse_transform2 = inCenterOfMassTransform2.InversedRotationTranslation();
	Mat44 transform1_to_2 = inverse_transform2 * inCenterOfMassTransform1;
	mTransform2To1 = transform1_to_2.InversedRotationTranslation();

	// Calculate bounds
	mBoundsOf1 = inShape1->GetLocalBounds().Scaled(inScale1);
	mBoundsOf1.ExpandBy(Vec3::sReplicate(inCollideShapeSettings.mMaxSeparationDistance));
	mBoundsOf1InSpaceOf2 = mBoundsOf1.Transformed(transform1_to_2);	// Convert bounding box of 1 into space of 2

	// Determine if shape 2 is inside out or not
	mScaleSign2 = ScaleHelpers::IsInsideOut(inScale2)? -1.0f : 1.0f;
}

void CollideConvexVsTriangles::Collide(Vec3Arg inV0, Vec3Arg inV1, Vec3Arg inV2, uint8 inActiveEdges, const SubShapeID &inSubShapeID2)
{
	JPH_PROFILE_FUNCTION();

	JPH_IF_STAT_COLLECTOR(sNumCollideChecks++;)

	// Scale triangle and transform it to the space of 1
	Vec3 v0 = mTransform2To1 * (mScale2 * inV0); 
	Vec3 v1 = mTransform2To1 * (mScale2 * inV1);
	Vec3 v2 = mTransform2To1 * (mScale2 * inV2);

	// Calculate triangle normal
	Vec3 triangle_normal = mScaleSign2 * (v1 - v0).Cross(v2 - v0);

	// Backface check
	bool back_facing = triangle_normal.Dot(v0) > 0.0f;
	if (mCollideShapeSettings.mBackFaceMode == EBackFaceMode::IgnoreBackFaces && back_facing)
		return;

	// Get bounding box for triangle
	AABox triangle_bbox = AABox::sFromTwoPoints(v0, v1);
	triangle_bbox.Encapsulate(v2);

	// Get intersection between triangle and shape box, if there is none, we're done
	if (!triangle_bbox.Overlaps(mBoundsOf1))
		return;

	// Create triangle support function
	TriangleConvexSupport triangle(v0, v1, v2);

	// Perform collision detection
	Vec3 penetration_axis = Vec3::sAxisX(), point1, point2;
	EPAPenetrationDepth pen_depth;
	EPAPenetrationDepth::EStatus status;

	JPH_IF_STAT_COLLECTOR(sNumGJKChecks++;)

	// Get the support function
	if (mShape1ExCvxRadius == nullptr)
		mShape1ExCvxRadius = mShape1->GetSupportFunction(ConvexShape::ESupportMode::ExcludeConvexRadius, mBufferExCvxRadius, mScale1);

	// Perform GJK step
	status = pen_depth.GetPenetrationDepthStepGJK(*mShape1ExCvxRadius, mShape1ExCvxRadius->GetConvexRadius() + mCollideShapeSettings.mMaxSeparationDistance, triangle, 0.0f, mCollideShapeSettings.mCollisionTolerance, penetration_axis, point1, point2);

	// Check result of collision detection
	if (status == EPAPenetrationDepth::EStatus::NotColliding)
		return;
	else if (status == EPAPenetrationDepth::EStatus::Indeterminate)
	{
		// Need to run expensive EPA algorithm
		JPH_IF_STAT_COLLECTOR(sNumEPAChecks++;)

		// Get the support function
		if (mShape1IncCvxRadius == nullptr)
			mShape1IncCvxRadius = mShape1->GetSupportFunction(ConvexShape::ESupportMode::IncludeConvexRadius, mBufferIncCvxRadius, mScale1);

		// Add convex radius
		AddConvexRadius<ConvexShape::Support> shape1_add_max_separation_distance(*mShape1IncCvxRadius, mCollideShapeSettings.mMaxSeparationDistance);

		// Perform EPA step
		if (!pen_depth.GetPenetrationDepthStepEPA(shape1_add_max_separation_distance, triangle, mCollideShapeSettings.mPenetrationTolerance, penetration_axis, point1, point2))
			return;
	}

	// Check if the penetration is bigger than the early out fraction
	float penetration_depth = (point2 - point1).Length() - mCollideShapeSettings.mMaxSeparationDistance;
	if (-penetration_depth >= mCollector.GetEarlyOutFraction())
		return;

	// Correct point1 for the added separation distance
	float penetration_axis_len = penetration_axis.Length();
	if (penetration_axis_len > 0.0f)
		point1 -= penetration_axis * (mCollideShapeSettings.mMaxSeparationDistance / penetration_axis_len);

	// Check if we have enabled active edge detection
	if (mCollideShapeSettings.mActiveEdgeMode == EActiveEdgeMode::CollideOnlyWithActive)
	{
		// Convert the active edge velocity hint to local space
		Vec3 active_edge_movement_direction = mTransform1.Multiply3x3Transposed(mCollideShapeSettings.mActiveEdgeMovementDirection);

		// Update the penetration axis to account for active edges
		// Note that we flip the triangle normal as the penetration axis is pointing towards the triangle instead of away
		penetration_axis = ActiveEdges::FixNormal(v0, v1, v2, back_facing? triangle_normal : -triangle_normal, inActiveEdges, point2, penetration_axis, active_edge_movement_direction);
	}

	// Convert to world space
	point1 = mTransform1 * point1;
	point2 = mTransform1 * point2;
	Vec3 penetration_axis_world = mTransform1.Multiply3x3(penetration_axis);

	// Create collision result
	CollideShapeResult result(point1, point2, penetration_axis_world, penetration_depth, mSubShapeID1, inSubShapeID2, TransformedShape::sGetBodyID(mCollector.GetContext()));

	// Gather faces
	if (mCollideShapeSettings.mCollectFacesMode == ECollectFacesMode::CollectFaces)
	{
		// Get supporting face of shape 1
		mShape1->GetSupportingFace(-penetration_axis, mScale1, result.mShape1Face);

		// Convert to world space
		for (Vec3 &p : result.mShape1Face)
			p = mTransform1 * p;

		// Get face of the triangle
		triangle.GetSupportingFace(penetration_axis, result.mShape2Face);

		// Convert to world space
		for (Vec3 &p : result.mShape2Face)
			p = mTransform1 * p;
	}
	
	JPH_IF_STAT_COLLECTOR(sNumCollisions++;)

	// Notify the collector
	mCollector.AddHit(result);
}

#ifdef JPH_STAT_COLLECTOR
void CollideConvexVsTriangles::sResetStats()
{
	sNumCollideChecks = 0;
	sNumGJKChecks = 0;
	sNumEPAChecks = 0;
	sNumCollisions = 0;
}

void CollideConvexVsTriangles::sCollectStats()
{
	JPH_PROFILE_FUNCTION();

	JPH_STAT_COLLECTOR_ADD("ConvexVsTriangles.NumChecks", int(sNumCollideChecks));
	JPH_STAT_COLLECTOR_ADD("ConvexVsTriangles.NumCollisions", int(sNumCollisions));
	if (sNumCollideChecks > 0)
	{
		JPH_STAT_COLLECTOR_ADD("ConvexVsTriangles.GJKCheckPercentage", 100.0f * sNumGJKChecks / sNumCollideChecks);
		JPH_STAT_COLLECTOR_ADD("ConvexVsTriangles.EPACheckPercentage", 100.0f * sNumEPAChecks / sNumCollideChecks);
	}
}
#endif // JPH_STAT_COLLECTOR

} // JPH