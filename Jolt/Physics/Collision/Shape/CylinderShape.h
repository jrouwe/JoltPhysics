// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Physics/Collision/Shape/ConvexShape.h>
#include <Jolt/Physics/PhysicsSettings.h>

JPH_NAMESPACE_BEGIN

/// Class that constructs a CylinderShape
class CylinderShapeSettings final : public ConvexShapeSettings
{
public:
	JPH_DECLARE_SERIALIZABLE_VIRTUAL(CylinderShapeSettings)

	/// Default constructor for deserialization
							CylinderShapeSettings() = default;

	/// Create a shape centered around the origin with one top at (0, -inHalfHeight, 0) and the other at (0, inHalfHeight, 0) and radius inRadius.
	/// (internally the convex radius will be subtracted from the cylinder the total cylinder will not grow with the convex radius, but the edges of the cylinder will be rounded a bit).
							CylinderShapeSettings(float inHalfHeight, float inRadius, float inConvexRadius = cDefaultConvexRadius, const PhysicsMaterial *inMaterial = nullptr) : ConvexShapeSettings(inMaterial), mHalfHeight(inHalfHeight), mRadius(inRadius), mConvexRadius(inConvexRadius) { }

	// See: ShapeSettings
	virtual ShapeResult		Create() const override;

	float					mHalfHeight = 0.0f;
	float					mRadius = 0.0f;
	float					mConvexRadius = 0.0f;
};

/// A cylinder
class CylinderShape final : public ConvexShape
{
public:
	/// Constructor
							CylinderShape() : ConvexShape(EShapeSubType::Cylinder) { }
							CylinderShape(const CylinderShapeSettings &inSettings, ShapeResult &outResult);

	/// Create a shape centered around the origin with one top at (0, -inHalfHeight, 0) and the other at (0, inHalfHeight, 0) and radius inRadius.
	/// (internally the convex radius will be subtracted from the cylinder the total cylinder will not grow with the convex radius, but the edges of the cylinder will be rounded a bit).
							CylinderShape(float inHalfHeight, float inRadius, float inConvexRadius = cDefaultConvexRadius, const PhysicsMaterial *inMaterial = nullptr);

	/// Get half height of cylinder
	float					GetHalfHeight() const														{ return mHalfHeight; }

	/// Get radius of cylinder
	float					GetRadius() const															{ return mRadius; }

	// See Shape::GetLocalBounds
	virtual AABox			GetLocalBounds() const override;

	// See Shape::GetInnerRadius
	virtual float			GetInnerRadius() const override												{ return min(mHalfHeight, mRadius); }

	// See Shape::GetMassProperties
	virtual MassProperties	GetMassProperties() const override;

	// See Shape::GetSurfaceNormal
	virtual Vec3			GetSurfaceNormal(const SubShapeID &inSubShapeID, Vec3Arg inLocalSurfacePosition) const override;

	// See ConvexShape::GetSupportFunction
	virtual const Support *	GetSupportFunction(ESupportMode inMode, SupportBuffer &inBuffer, Vec3Arg inScale) const override;

	// See ConvexShape::GetSupportingFace
	virtual void			GetSupportingFace(Vec3Arg inDirection, Vec3Arg inScale, SupportingFace &outVertices) const override;

#ifdef JPH_DEBUG_RENDERER
	// See Shape::Draw
	virtual void			Draw(DebugRenderer *inRenderer, Mat44Arg inCenterOfMassTransform, Vec3Arg inScale, ColorArg inColor, bool inUseMaterialColors, bool inDrawWireframe) const override;
#endif // JPH_DEBUG_RENDERER

	// See Shape::CastRay
	using ConvexShape::CastRay;
	virtual bool			CastRay(const RayCast &inRay, const SubShapeIDCreator &inSubShapeIDCreator, RayCastResult &ioHit) const override;

	// See: Shape::CollidePoint
	virtual void			CollidePoint(Vec3Arg inPoint, const SubShapeIDCreator &inSubShapeIDCreator, CollidePointCollector &ioCollector) const override;

	// See Shape::TransformShape
	virtual void			TransformShape(Mat44Arg inCenterOfMassTransform, TransformedShapeCollector &ioCollector) const override;

	// See Shape::GetTrianglesStart
	virtual void			GetTrianglesStart(GetTrianglesContext &ioContext, const AABox &inBox, Vec3Arg inPositionCOM, QuatArg inRotation, Vec3Arg inScale) const override;

	// See Shape::GetTrianglesNext
	virtual int				GetTrianglesNext(GetTrianglesContext &ioContext, int inMaxTrianglesRequested, Float3 *outTriangleVertices, const PhysicsMaterial **outMaterials = nullptr) const override;

	// See Shape
	virtual void			SaveBinaryState(StreamOut &inStream) const override;

	// See Shape::GetStats
	virtual Stats			GetStats() const override												{ return Stats(sizeof(*this), 0); }

	// See Shape::GetVolume
	virtual float			GetVolume() const override												{ return 2.0f * JPH_PI * mHalfHeight * Square(mRadius); }

	// See Shape::IsValidScale
	virtual bool			IsValidScale(Vec3Arg inScale) const override;

	// Register shape functions with the registry
	static void				sRegister();

protected:
	// See: Shape::RestoreBinaryState
	virtual void			RestoreBinaryState(StreamIn &inStream) override;

private:
	// Class for GetSupportFunction
	class					Cylinder;

	float					mHalfHeight = 0.0f;
	float					mRadius = 0.0f;
	float					mConvexRadius = 0.0f;
};

JPH_NAMESPACE_END
