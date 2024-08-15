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

AABox PlaneShape::GetLocalBounds() const
{
	return AABox(Vec3::sReplicate(-mSize), Vec3::sReplicate(mSize));
}

#ifdef JPH_DEBUG_RENDERER
void PlaneShape::Draw(DebugRenderer *inRenderer, RMat44Arg inCenterOfMassTransform, Vec3Arg inScale, ColorArg inColor, bool inUseMaterialColors, bool inDrawWireframe) const
{
	RMat44 com = inCenterOfMassTransform * Mat44::sScale(inScale);
	
	RVec3 point = com * (-mPlane.GetNormal() * mPlane.GetConstant());
	Vec3 normal = (com.GetDirectionPreservingMatrix() * mPlane.GetNormal()).Normalized();
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

	JPH_ASSERT(ScaleHelpers::IsNotScaled(inScale));

	Mat44 inverse_transform = inCenterOfMassTransform.InversedRotationTranslation();

	for (SoftBodyVertex *v = ioVertices, *sbv_end = ioVertices + inNumVertices; v < sbv_end; ++v)
		if (v->mInvMass > 0.0f)
		{
			// Convert to local space
			Vec3 local_pos = inverse_transform * v->mPosition;

			// Calculate penetration
			float penetration = -mPlane.SignedDistance(local_pos);
			if (penetration > v->mLargestPenetration)
			{
				v->mLargestPenetration = penetration;
				v->mCollisionPlane = mPlane.GetTransformed(inCenterOfMassTransform);
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

	new (&ioContext) PSGetTrianglesContext(this, Mat44::sRotationTranslation(inRotation, inPositionCOM) * Mat44::sScale(inScale));
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
	//const ConvexShape *shape1 = static_cast<const ConvexShape *>(inShape1);
	//const PlaneShape *shape2 = static_cast<const PlaneShape *>(inShape2);
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
