// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Physics/Collision/Shape/ConvexShape.h>

JPH_NAMESPACE_BEGIN

/// Class that constructs a TriangleShape
class TriangleShapeSettings final : public ConvexShapeSettings
{
public:
	JPH_DECLARE_SERIALIZABLE_VIRTUAL(TriangleShapeSettings)

	/// Default constructor for deserialization
							TriangleShapeSettings() = default;

	/// Create a triangle with points (inV1, inV2, inV3) (counter clockwise) and convex radius inConvexRadius.
	/// Note that the convex radius is currently only used for shape vs shape collision, for all other purposes the triangle is infinitely thin.
							TriangleShapeSettings(Vec3Arg inV1, Vec3Arg inV2, Vec3Arg inV3, float inConvexRadius = 0.0f, const PhysicsMaterial *inMaterial = nullptr) : ConvexShapeSettings(inMaterial), mV1(inV1), mV2(inV2), mV3(inV3), mConvexRadius(inConvexRadius) { }

	// See: ShapeSettings
	virtual ShapeResult		Create() const override;

	Vec3					mV1;
	Vec3					mV2;
	Vec3					mV3;
	float					mConvexRadius = 0.0f;
};

/// A single triangle, not the most efficient way of creating a world filled with triangles but can be used as a query shape for example.
class TriangleShape final : public ConvexShape
{
public:
	/// Constructor
							TriangleShape() : ConvexShape(EShapeSubType::Triangle) { }
							TriangleShape(const TriangleShapeSettings &inSettings, ShapeResult &outResult);

	/// Create a triangle with points (inV1, inV2, inV3) (counter clockwise) and convex radius inConvexRadius.
	/// Note that the convex radius is currently only used for shape vs shape collision, for all other purposes the triangle is infinitely thin.
							TriangleShape(Vec3Arg inV1, Vec3Arg inV2, Vec3Arg inV3, float inConvexRadius = 0.0f, const PhysicsMaterial *inMaterial = nullptr) : ConvexShape(EShapeSubType::Triangle, inMaterial), mV1(inV1), mV2(inV2), mV3(inV3), mConvexRadius(inConvexRadius) { JPH_ASSERT(inConvexRadius >= 0.0f); }

	/// Convex radius
	float					GetConvexRadius() const																{ return mConvexRadius; }

	// See Shape::GetLocalBounds
	virtual AABox			GetLocalBounds() const override;
		
	// See Shape::GetWorldSpaceBounds
	virtual AABox			GetWorldSpaceBounds(Mat44Arg inCenterOfMassTransform, Vec3Arg inScale) const override;

	// See Shape::GetInnerRadius
	virtual float			GetInnerRadius() const override														{ return mConvexRadius; }

	// See Shape::GetMassProperties
	virtual MassProperties	GetMassProperties() const override;

	// See Shape::GetSurfaceNormal
	virtual Vec3			GetSurfaceNormal(const SubShapeID &inSubShapeID, Vec3Arg inLocalSurfacePosition) const override;

	// See ConvexShape::GetSupportFunction
	virtual const Support *	GetSupportFunction(ESupportMode inMode, SupportBuffer &inBuffer, Vec3Arg inScale) const override;

	// See ConvexShape::GetSupportingFace
	virtual void			GetSupportingFace(Vec3Arg inDirection, Vec3Arg inScale, SupportingFace &outVertices) const override;

	// See Shape::GetSubmergedVolume
	virtual void			GetSubmergedVolume(Mat44Arg inCenterOfMassTransform, Vec3Arg inScale, const Plane &inSurface, float &outTotalVolume, float &outSubmergedVolume, Vec3 &outCenterOfBuoyancy) const override;

#ifdef JPH_DEBUG_RENDERER
	// See Shape::Draw
	virtual void			Draw(DebugRenderer *inRenderer, Mat44Arg inCenterOfMassTransform, Vec3Arg inScale, ColorArg inColor, bool inUseMaterialColors, bool inDrawWireframe) const override;
#endif // JPH_DEBUG_RENDERER

	// See Shape::CastRay
	virtual bool			CastRay(const RayCast &inRay, const SubShapeIDCreator &inSubShapeIDCreator, RayCastResult &ioHit) const override;
	virtual void			CastRay(const RayCast &inRay, const RayCastSettings &inRayCastSettings, const SubShapeIDCreator &inSubShapeIDCreator, CastRayCollector &ioCollector) const override;

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
	virtual Stats			GetStats() const override															{ return Stats(sizeof(*this), 1); }

	// See Shape::GetVolume
	virtual float			GetVolume() const override															{ return 0; }

	// See Shape::IsValidScale
	virtual bool			IsValidScale(Vec3Arg inScale) const override;

	// Register shape functions with the registry
	static void				sRegister();

protected:
	// See: Shape::RestoreBinaryState
	virtual void			RestoreBinaryState(StreamIn &inStream) override;

private:
	// Helper functions called by CollisionDispatch
	static void				sCollideConvexVsTriangle(const Shape *inShape1, const Shape *inShape2, Vec3Arg inScale1, Vec3Arg inScale2, Mat44Arg inCenterOfMassTransform1, Mat44Arg inCenterOfMassTransform2, const SubShapeIDCreator &inSubShapeIDCreator1, const SubShapeIDCreator &inSubShapeIDCreator2, const CollideShapeSettings &inCollideShapeSettings, CollideShapeCollector &ioCollector);
	static void				sCollideSphereVsTriangle(const Shape *inShape1, const Shape *inShape2, Vec3Arg inScale1, Vec3Arg inScale2, Mat44Arg inCenterOfMassTransform1, Mat44Arg inCenterOfMassTransform2, const SubShapeIDCreator &inSubShapeIDCreator1, const SubShapeIDCreator &inSubShapeIDCreator2, const CollideShapeSettings &inCollideShapeSettings, CollideShapeCollector &ioCollector);
	static void				sCastConvexVsTriangle(const ShapeCast &inShapeCast, const ShapeCastSettings &inShapeCastSettings, const Shape *inShape, Vec3Arg inScale, const ShapeFilter &inShapeFilter, Mat44Arg inCenterOfMassTransform2, const SubShapeIDCreator &inSubShapeIDCreator1, const SubShapeIDCreator &inSubShapeIDCreator2, CastShapeCollector &ioCollector);
	static void				sCastSphereVsTriangle(const ShapeCast &inShapeCast, const ShapeCastSettings &inShapeCastSettings, const Shape *inShape, Vec3Arg inScale, const ShapeFilter &inShapeFilter, Mat44Arg inCenterOfMassTransform2, const SubShapeIDCreator &inSubShapeIDCreator1, const SubShapeIDCreator &inSubShapeIDCreator2, CastShapeCollector &ioCollector);

	// Context for GetTrianglesStart/Next
	class					TSGetTrianglesContext;

	// Classes for GetSupportFunction
	class 					TriangleNoConvex;
	class					TriangleWithConvex;

	Vec3					mV1;
	Vec3					mV2;
	Vec3					mV3;
	float					mConvexRadius = 0.0f;
};

JPH_NAMESPACE_END
