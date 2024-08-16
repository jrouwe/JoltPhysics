// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Collision/Shape/PlaneShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexShape.h>
#include <Jolt/Physics/Collision/Shape/ScaleHelpers.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/ShapeCast.h>
#include <Jolt/Physics/Collision/ShapeFilter.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/CollisionDispatch.h>
#include <Jolt/Physics/Collision/TransformedShape.h>
#include <Jolt/Physics/Collision/CollidePointResult.h>
#include <Jolt/Physics/SoftBody/SoftBodyVertex.h>
#include <Jolt/Core/Profiler.h>
#include <Jolt/Core/StreamIn.h>
#include <Jolt/Core/StreamOut.h>
#include <Jolt/Geometry/Plane.h>
#include <Jolt/Renderer/DebugRenderer.h>
#include <Jolt/ObjectStream/TypeDeclarations.h>

JPH_NAMESPACE_BEGIN

JPH_IMPLEMENT_SERIALIZABLE_VIRTUAL(PlaneShapeSettings)
{
	JPH_ADD_BASE_CLASS(PlaneShapeSettings, ShapeSettings)

	JPH_ADD_ATTRIBUTE(PlaneShapeSettings, mPlane)
	JPH_ADD_ATTRIBUTE(PlaneShapeSettings, mMaterial)
	JPH_ADD_ATTRIBUTE(PlaneShapeSettings, mSize)
}

ShapeSettings::ShapeResult PlaneShapeSettings::Create() const
{
	if (mCachedResult.IsEmpty())
		Ref<Shape> shape = new PlaneShape(*this, mCachedResult);
	return mCachedResult;
}

void PlaneShape::CalculateLocalBounds()
{
	// Project the corners of a bounding box of size [-mSize, mSize] onto the plane
	Vec3 corners[] =
	{
		Vec3(+mSize, +mSize, +mSize),
		Vec3(+mSize, +mSize, -mSize),
		Vec3(+mSize, -mSize, +mSize),
		Vec3(+mSize, -mSize, -mSize),
		Vec3(-mSize, +mSize, +mSize),
		Vec3(-mSize, +mSize, -mSize),
		Vec3(-mSize, -mSize, +mSize),
		Vec3(-mSize, -mSize, -mSize),
	};
	mLocalBounds = AABox();
	Vec3 normal = mPlane.GetNormal();
	for (Vec3 &c : corners)
	{
		Vec3 projected = mPlane.ProjectPointOnPlane(c);

		// Encapsulate these points
		mLocalBounds.Encapsulate(projected);

		// And also encapsulate a point mSize behind that point
		mLocalBounds.Encapsulate(projected - mSize * normal);
	}
}

PlaneShape::PlaneShape(const PlaneShapeSettings &inSettings, ShapeResult &outResult) :
	Shape(EShapeType::Plane, EShapeSubType::Plane, inSettings, outResult),
	mPlane(inSettings.mPlane),
	mMaterial(inSettings.mMaterial)
{
	if (!mPlane.GetNormal().IsNormalized())
	{
		outResult.SetError("Plane normal needs to be normalized!");
		return;
	}

	CalculateLocalBounds();

	outResult.Set(this);
}

MassProperties PlaneShape::GetMassProperties() const
{
	// Object should always be static, return default mass properties
	return MassProperties();
}

void PlaneShape::GetSupportingFace(const SubShapeID &inSubShapeID, Vec3Arg inDirection, Vec3Arg inScale, Mat44Arg inCenterOfMassTransform, SupportingFace &outVertices) const
{
	JPH_ASSERT(false, "Cannot provide sensible supporting face for a plane");
}

#ifdef JPH_DEBUG_RENDERER
void PlaneShape::Draw(DebugRenderer *inRenderer, RMat44Arg inCenterOfMassTransform, Vec3Arg inScale, ColorArg inColor, bool inUseMaterialColors, bool inDrawWireframe) const
{
	RMat44 com = inCenterOfMassTransform.PreScaled(inScale);
	
	RVec3 point = com * (-mPlane.GetNormal() * mPlane.GetConstant());
	Vec3 normal = com.GetDirectionPreservingMatrix().Multiply3x3(mPlane.GetNormal()).Normalized();
	inRenderer->DrawPlane(point, normal, inColor, mSize, DebugRenderer::ECastShadow::On, DebugRenderer::EDrawMode::Solid);
}
#endif // JPH_DEBUG_RENDERER

bool PlaneShape::CastRay(const RayCast &inRay, const SubShapeIDCreator &inSubShapeIDCreator, RayCastResult &ioHit) const
{
	JPH_PROFILE_FUNCTION();

	return false;
}

void PlaneShape::CastRay(const RayCast &inRay, const RayCastSettings &inRayCastSettings, const SubShapeIDCreator &inSubShapeIDCreator, CastRayCollector &ioCollector, const ShapeFilter &inShapeFilter) const
{
	JPH_PROFILE_FUNCTION();

	// Test shape filter
	if (!inShapeFilter.ShouldCollide(this, inSubShapeIDCreator.GetID()))
		return;
}

void PlaneShape::CollidePoint(Vec3Arg inPoint, const SubShapeIDCreator &inSubShapeIDCreator, CollidePointCollector &ioCollector, const ShapeFilter &inShapeFilter) const
{
	JPH_PROFILE_FUNCTION();

	// Test shape filter
	if (!inShapeFilter.ShouldCollide(this, inSubShapeIDCreator.GetID()))
		return;

	// Check if the point is inside the plane
	if (mPlane.SignedDistance(inPoint) < 0.0f)
		ioCollector.AddHit({ TransformedShape::sGetBodyID(ioCollector.GetContext()), inSubShapeIDCreator.GetID() });
}

void PlaneShape::CollideSoftBodyVertices(Mat44Arg inCenterOfMassTransform, Vec3Arg inScale, SoftBodyVertex *ioVertices, uint inNumVertices, [[maybe_unused]] float inDeltaTime, [[maybe_unused]] Vec3Arg inDisplacementDueToGravity, int inCollidingShapeIndex) const
{
	JPH_PROFILE_FUNCTION();

	// Convert plane to world space
	Plane plane = mPlane.GetTransformedWithScaling(inCenterOfMassTransform.PreScaled(inScale));

	for (SoftBodyVertex *v = ioVertices, *sbv_end = ioVertices + inNumVertices; v < sbv_end; ++v)
		if (v->mInvMass > 0.0f)
		{
			// Calculate penetration
			float penetration = -plane.SignedDistance(v->mPosition);
			if (penetration > v->mLargestPenetration)
			{
				v->mLargestPenetration = penetration;
				v->mCollisionPlane = plane;
				v->mCollidingShapeIndex = inCollidingShapeIndex;
			}
		}
}

void PlaneShape::sCastConvexVsPlane(const ShapeCast &inShapeCast, const ShapeCastSettings &inShapeCastSettings, const Shape *inShape, Vec3Arg inScale, [[maybe_unused]] const ShapeFilter &inShapeFilter, Mat44Arg inCenterOfMassTransform2, const SubShapeIDCreator &inSubShapeIDCreator1, const SubShapeIDCreator &inSubShapeIDCreator2, CastShapeCollector &ioCollector)
{
	JPH_PROFILE_FUNCTION();

	JPH_ASSERT(inShape->GetSubType() == EShapeSubType::Plane);
	//const PlaneShape *shape = static_cast<const PlaneShape *>(inShape);
}

struct PlaneShape::PSGetTrianglesContext
{
	JPH_INLINE					PSGetTrianglesContext(const PlaneShape *inShape, Mat44Arg inCenterOfMassTransform) :
		mShape(inShape),
		mCenterOfMassTransform(inCenterOfMassTransform)
	{
	}

	const PlaneShape *			mShape;
	Mat44						mCenterOfMassTransform;
};

void PlaneShape::GetTrianglesStart(GetTrianglesContext &ioContext, const AABox &inBox, Vec3Arg inPositionCOM, QuatArg inRotation, Vec3Arg inScale) const
{
	static_assert(sizeof(PSGetTrianglesContext) <= sizeof(GetTrianglesContext), "GetTrianglesContext too small");
	JPH_ASSERT(IsAligned(&ioContext, alignof(PSGetTrianglesContext)));

	new (&ioContext) PSGetTrianglesContext(this, Mat44::sRotationTranslation(inRotation, inPositionCOM).PreScaled(inScale));
}

int PlaneShape::GetTrianglesNext(GetTrianglesContext &ioContext, int inMaxTrianglesRequested, Float3 *outTriangleVertices, const PhysicsMaterial **outMaterials) const
{
	//PSGetTrianglesContext &context = (PSGetTrianglesContext &)ioContext;
	return 0;
}

void PlaneShape::sCollideConvexVsPlane(const Shape *inShape1, const Shape *inShape2, Vec3Arg inScale1, Vec3Arg inScale2, Mat44Arg inCenterOfMassTransform1, Mat44Arg inCenterOfMassTransform2, const SubShapeIDCreator &inSubShapeIDCreator1, const SubShapeIDCreator &inSubShapeIDCreator2, const CollideShapeSettings &inCollideShapeSettings, CollideShapeCollector &ioCollector, [[maybe_unused]] const ShapeFilter &inShapeFilter)
{
	JPH_PROFILE_FUNCTION();

	// Get the shapes
	JPH_ASSERT(inShape1->GetType() == EShapeType::Convex);
	JPH_ASSERT(inShape2->GetType() == EShapeType::Plane);
	const ConvexShape *shape1 = static_cast<const ConvexShape *>(inShape1);
	const PlaneShape *shape2 = static_cast<const PlaneShape *>(inShape2);

	// Get transforms
	Mat44 transform2 = inCenterOfMassTransform2.PreScaled(inScale2);
	Mat44 inverse_transform1 = inCenterOfMassTransform1.InversedRotationTranslation();
	Mat44 transform_2_to_1 = inverse_transform1 * transform2;

	// Transform the plane to the space of the convex shape
	Plane plane = shape2->mPlane.GetTransformedWithScaling(transform_2_to_1);
	Vec3 normal = plane.GetNormal();

	// Get support function
	ConvexShape::SupportBuffer shape1_support_buffer;
	const ConvexShape::Support *shape1_support = shape1->GetSupportFunction(ConvexShape::ESupportMode::Default, shape1_support_buffer, inScale1);

	// Get the support point of the convex shape in the opposite direction of the plane normal
	Vec3 support_point = shape1_support->GetSupport(-normal);
	float signed_distance = plane.SignedDistance(support_point);
	float convex_radius = shape1_support->GetConvexRadius();
	float penetration_depth = -signed_distance + convex_radius;
	if (penetration_depth > -inCollideShapeSettings.mMaxSeparationDistance)
	{
		// Get contact point
		Vec3 point1 = inCenterOfMassTransform1 * (support_point - normal * convex_radius);
		Vec3 point2 = inCenterOfMassTransform1 * (support_point - normal * signed_distance);
		Vec3 penetration_axis_world = inCenterOfMassTransform1.Multiply3x3(-normal);

		// Create collision result
		CollideShapeResult result(point1, point2, penetration_axis_world, penetration_depth, inSubShapeIDCreator1.GetID(), inSubShapeIDCreator2.GetID(), TransformedShape::sGetBodyID(ioCollector.GetContext()));

		// Gather faces
		if (inCollideShapeSettings.mCollectFacesMode == ECollectFacesMode::CollectFaces)
		{
			// Get supporting face of shape 1
			shape1->GetSupportingFace(SubShapeID(), normal, inScale1, inCenterOfMassTransform1, result.mShape1Face);

			// Project these points on the plane for shape 2 and reverse them
			if (!result.mShape1Face.empty())
			{
				Plane world_plane = plane.GetTransformed(inCenterOfMassTransform1);
				result.mShape2Face.resize(result.mShape1Face.size());
				for (uint i = 0; i < result.mShape1Face.size(); ++i)
					result.mShape2Face[i] = world_plane.ProjectPointOnPlane(result.mShape1Face[result.mShape1Face.size() - 1 - i]);
			}
		}

		// Notify the collector
		JPH_IF_TRACK_NARROWPHASE_STATS(TrackNarrowPhaseCollector track;)
		ioCollector.AddHit(result);
	}
}

void PlaneShape::SaveBinaryState(StreamOut &inStream) const
{
	Shape::SaveBinaryState(inStream);

	inStream.Write(mPlane);
	inStream.Write(mSize);
}

void PlaneShape::RestoreBinaryState(StreamIn &inStream)
{
	Shape::RestoreBinaryState(inStream);

	inStream.Read(mPlane);
	inStream.Read(mSize);

	CalculateLocalBounds();
}

void PlaneShape::SaveMaterialState(PhysicsMaterialList &outMaterials) const
{
	outMaterials = { mMaterial };
}

void PlaneShape::RestoreMaterialState(const PhysicsMaterialRefC *inMaterials, uint inNumMaterials)
{
	JPH_ASSERT(inNumMaterials == 1);
	mMaterial = inMaterials[0];
}

void PlaneShape::sRegister()
{
	ShapeFunctions &f = ShapeFunctions::sGet(EShapeSubType::Plane);
	f.mConstruct = []() -> Shape * { return new PlaneShape; };
	f.mColor = Color::sDarkRed;

	for (EShapeSubType s : sConvexSubShapeTypes)
	{
		CollisionDispatch::sRegisterCollideShape(s, EShapeSubType::Plane, sCollideConvexVsPlane);
		CollisionDispatch::sRegisterCastShape(s, EShapeSubType::Plane, sCastConvexVsPlane);

		CollisionDispatch::sRegisterCastShape(EShapeSubType::Plane, s, CollisionDispatch::sReversedCastShape);
		CollisionDispatch::sRegisterCollideShape(EShapeSubType::Plane, s, CollisionDispatch::sReversedCollideShape);
	}
}

JPH_NAMESPACE_END
