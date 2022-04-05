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

bool TransformedShape::CastRay(const RayCast &inRay, RayCastResult &ioHit) const
{
	if (mShape != nullptr)
	{
		// Transform the ray to local space
		RayCast ray = inRay.Transformed(GetInverseCenterOfMassTransform());

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

void TransformedShape::CastRay(const RayCast &inRay, const RayCastSettings &inRayCastSettings, CastRayCollector &ioCollector) const
{
	if (mShape != nullptr)
	{
		// Set the context on the collector
		ioCollector.SetContext(this);

		// Transform and scale the ray to local space
		RayCast ray = inRay.Transformed(GetInverseCenterOfMassTransform());

		// Scale the ray
		Vec3 inv_scale = GetShapeScale().Reciprocal();
		ray.mOrigin *= inv_scale;
		ray.mDirection *= inv_scale;

		// Cast the ray on the shape
		SubShapeIDCreator sub_shape_id(mSubShapeIDCreator);
		mShape->CastRay(ray, inRayCastSettings, sub_shape_id, ioCollector);
	}
}

void TransformedShape::CollidePoint(Vec3Arg inPoint, CollidePointCollector &ioCollector) const
{
	if (mShape != nullptr)
	{
		// Set the context on the collector
		ioCollector.SetContext(this);

		// Transform and scale the point to local space
		Vec3 point = (GetInverseCenterOfMassTransform() * inPoint) / GetShapeScale();

		// Do point collide on the shape
		SubShapeIDCreator sub_shape_id(mSubShapeIDCreator);
		mShape->CollidePoint(point, sub_shape_id, ioCollector);
	}
}

void TransformedShape::CollideShape(const Shape *inShape, Vec3Arg inShapeScale, Mat44Arg inCenterOfMassTransform, const CollideShapeSettings &inCollideShapeSettings, CollideShapeCollector &ioCollector) const
{
	if (mShape != nullptr)
	{
		// Set the context on the collector
		ioCollector.SetContext(this);

		SubShapeIDCreator sub_shape_id1, sub_shape_id2(mSubShapeIDCreator);
		CollisionDispatch::sCollideShapeVsShape(inShape, mShape, inShapeScale, GetShapeScale(), inCenterOfMassTransform, GetCenterOfMassTransform(), sub_shape_id1, sub_shape_id2, inCollideShapeSettings, ioCollector);
	}
}

void TransformedShape::CastShape(const ShapeCast &inShapeCast, const ShapeCastSettings &inShapeCastSettings, CastShapeCollector &ioCollector, const ShapeFilter &inShapeFilter) const
{
	if (mShape != nullptr)
	{
		// Set the context on the collector and filter
		ioCollector.SetContext(this);
		inShapeFilter.mBodyID2 = mBodyID;

		// Get center of mass of object we're casting against
		Mat44 center_of_mass_transform2 = GetCenterOfMassTransform();

		SubShapeIDCreator sub_shape_id1, sub_shape_id2(mSubShapeIDCreator);
		CollisionDispatch::sCastShapeVsShapeWorldSpace(inShapeCast, inShapeCastSettings, mShape, GetShapeScale(), inShapeFilter, center_of_mass_transform2, sub_shape_id1, sub_shape_id2, ioCollector);
	}
}

void TransformedShape::CollectTransformedShapes(const AABox &inBox, TransformedShapeCollector &ioCollector) const
{
	if (mShape != nullptr)
	{
		// Set the context on the collector
		ioCollector.SetContext(this);

		mShape->CollectTransformedShapes(inBox, mShapePositionCOM, mShapeRotation, GetShapeScale(), mSubShapeIDCreator, ioCollector);
	}
}

void TransformedShape::GetTrianglesStart(GetTrianglesContext &ioContext, const AABox &inBox) const
{
	if (mShape != nullptr)
		mShape->GetTrianglesStart(ioContext, inBox, mShapePositionCOM, mShapeRotation, GetShapeScale());
}

int TransformedShape::GetTrianglesNext(GetTrianglesContext &ioContext, int inMaxTrianglesRequested, Float3 *outTriangleVertices, const PhysicsMaterial **outMaterials) const
{
	if (mShape != nullptr)
		return mShape->GetTrianglesNext(ioContext, inMaxTrianglesRequested, outTriangleVertices, outMaterials);
	else
		return 0;
}

JPH_NAMESPACE_END
