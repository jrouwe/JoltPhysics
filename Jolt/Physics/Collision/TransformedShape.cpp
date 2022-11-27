// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Collision/TransformedShape.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/ShapeCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/Shape/SubShapeID.h>
#include <Jolt/Physics/Collision/CollisionDispatch.h>
#include <Jolt/Geometry/OrientedBox.h>

JPH_NAMESPACE_BEGIN

bool TransformedShape::CastRay(const RRayCast &inRay, RayCastResult &ioHit) const
{
	if (mShape != nullptr)
	{
		// Transform the ray to local space, note that this drops precision which is possible because we're in local space now
		RayCast ray(inRay.Transformed(GetInverseCenterOfMassTransform()));

		// Scale the ray
		Vec3 inv_scale = GetShapeScale().Reciprocal();
		ray.mOrigin *= inv_scale;
		ray.mDirection *= inv_scale;

		// Cast the ray on the shape
		SubShapeIDCreator sub_shape_id(mSubShapeIDCreator);
		if (mShape->CastRay(ray, sub_shape_id, ioHit))
		{
			// Set body ID on the hit result
			ioHit.mBodyID = mBodyID;

			return true;
		}
	}

	return false;
}

void TransformedShape::CastRay(const RRayCast &inRay, const RayCastSettings &inRayCastSettings, CastRayCollector &ioCollector, const ShapeFilter &inShapeFilter) const
{
	if (mShape != nullptr)
	{
		// Set the context on the collector and filter
		ioCollector.SetContext(this);
		inShapeFilter.mBodyID2 = mBodyID;

		// Transform the ray to local space, note that this drops precision which is possible because we're in local space now
		RayCast ray(inRay.Transformed(GetInverseCenterOfMassTransform()));

		// Scale the ray
		Vec3 inv_scale = GetShapeScale().Reciprocal();
		ray.mOrigin *= inv_scale;
		ray.mDirection *= inv_scale;

		// Cast the ray on the shape
		SubShapeIDCreator sub_shape_id(mSubShapeIDCreator);
		mShape->CastRay(ray, inRayCastSettings, sub_shape_id, ioCollector, inShapeFilter);
	}
}

void TransformedShape::CollidePoint(RVec3Arg inPoint, CollidePointCollector &ioCollector, const ShapeFilter &inShapeFilter) const
{
	if (mShape != nullptr)
	{
		// Set the context on the collector and filter
		ioCollector.SetContext(this);
		inShapeFilter.mBodyID2 = mBodyID;

		// Transform and scale the point to local space
		Vec3 point = Vec3(GetInverseCenterOfMassTransform() * inPoint) / GetShapeScale();

		// Do point collide on the shape
		SubShapeIDCreator sub_shape_id(mSubShapeIDCreator);
		mShape->CollidePoint(point, sub_shape_id, ioCollector, inShapeFilter);
	}
}

void TransformedShape::CollideShape(const Shape *inShape, Vec3Arg inShapeScale, RMat44Arg inCenterOfMassTransform, const CollideShapeSettings &inCollideShapeSettings, RVec3Arg inBaseOffset, CollideShapeCollector &ioCollector, const ShapeFilter &inShapeFilter) const
{
	if (mShape != nullptr)
	{
		// Set the context on the collector and filter
		ioCollector.SetContext(this);
		inShapeFilter.mBodyID2 = mBodyID;

		SubShapeIDCreator sub_shape_id1, sub_shape_id2(mSubShapeIDCreator);
		Mat44 transform1 = inCenterOfMassTransform.PostTranslated(-inBaseOffset).ToMat44();
		Mat44 transform2 = GetCenterOfMassTransform().PostTranslated(-inBaseOffset).ToMat44();
		CollisionDispatch::sCollideShapeVsShape(inShape, mShape, inShapeScale, GetShapeScale(), transform1, transform2, sub_shape_id1, sub_shape_id2, inCollideShapeSettings, ioCollector, inShapeFilter);
	}
}

void TransformedShape::CastShape(const RShapeCast &inShapeCast, const ShapeCastSettings &inShapeCastSettings, RVec3Arg inBaseOffset, CastShapeCollector &ioCollector, const ShapeFilter &inShapeFilter) const
{
	if (mShape != nullptr)
	{
		// Set the context on the collector and filter
		ioCollector.SetContext(this);
		inShapeFilter.mBodyID2 = mBodyID;

		// Get the shape cast relative to the base offset and convert it to floats
		ShapeCast shape_cast(inShapeCast.PostTranslated(-inBaseOffset));

		// Get center of mass of object we're casting against relative to the base offset and convert it to floats
		Mat44 center_of_mass_transform2 = GetCenterOfMassTransform().PostTranslated(-inBaseOffset).ToMat44();

		SubShapeIDCreator sub_shape_id1, sub_shape_id2(mSubShapeIDCreator);
		CollisionDispatch::sCastShapeVsShapeWorldSpace(shape_cast, inShapeCastSettings, mShape, GetShapeScale(), inShapeFilter, center_of_mass_transform2, sub_shape_id1, sub_shape_id2, ioCollector);
	}
}

void TransformedShape::CollectTransformedShapes(const AABox &inBox, TransformedShapeCollector &ioCollector, const ShapeFilter &inShapeFilter) const
{
	if (mShape != nullptr)
	{
		// Set the context on the collector
		ioCollector.SetContext(this);

		mShape->CollectTransformedShapes(inBox, mShapePositionCOM, mShapeRotation, GetShapeScale(), mSubShapeIDCreator, ioCollector, inShapeFilter);
	}
}

void TransformedShape::GetTrianglesStart(GetTrianglesContext &ioContext, const AABox &inBox) const
{
	if (mShape != nullptr)
		mShape->GetTrianglesStart(ioContext, inBox, Vec3(mShapePositionCOM), mShapeRotation, GetShapeScale()); // TODO_DP
}

int TransformedShape::GetTrianglesNext(GetTrianglesContext &ioContext, int inMaxTrianglesRequested, Float3 *outTriangleVertices, const PhysicsMaterial **outMaterials) const
{
	if (mShape != nullptr)
		return mShape->GetTrianglesNext(ioContext, inMaxTrianglesRequested, outTriangleVertices, outMaterials);
	else
		return 0;
}

JPH_NAMESPACE_END
