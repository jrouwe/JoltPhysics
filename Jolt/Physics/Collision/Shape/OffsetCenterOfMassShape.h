// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Physics/Collision/Shape/DecoratedShape.h>

namespace JPH {

class CollideShapeSettings;

/// Class that constructs an OffsetCenterOfMassShape
class OffsetCenterOfMassShapeSettings final : public DecoratedShapeSettings
{
public:
	JPH_DECLARE_SERIALIZABLE_VIRTUAL(OffsetCenterOfMassShapeSettings)

	/// Constructor
									OffsetCenterOfMassShapeSettings() = default;

	/// Construct with shape settings, can be serialized.
									OffsetCenterOfMassShapeSettings(Vec3Arg inOffset, const ShapeSettings *inShape) : DecoratedShapeSettings(inShape), mOffset(inOffset) { }

	/// Variant that uses a concrete shape, which means this object cannot be serialized.
									OffsetCenterOfMassShapeSettings(Vec3Arg inOffset, const Shape *inShape): DecoratedShapeSettings(inShape), mOffset(inOffset) { }

	// See: ShapeSettings
	virtual ShapeResult				Create() const override;

	Vec3							mOffset;												///< Offset to be applied to the center of mass of the child shape
};

/// This shape will shift the center of mass of a child shape, it can e.g. be used to lower the center of mass of an unstable object like a boat to make it stable
class OffsetCenterOfMassShape final : public DecoratedShape
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(OffsetCenterOfMassShape)

	/// Constructor
									OffsetCenterOfMassShape() = default;
									OffsetCenterOfMassShape(const OffsetCenterOfMassShapeSettings &inSettings, ShapeResult &outResult);

	/// Get type
	virtual EShapeType				GetType() const override								{ return EShapeType::OffsetCenterOfMass; }

	/// Access the offset that is applied to the center of mass
	const Vec3						GetOffset() const										{ return mOffset; }

	// See Shape::GetCenterOfMass
	virtual Vec3					GetCenterOfMass() const override						{ return mInnerShape->GetCenterOfMass() + mOffset; }

	// See Shape::GetLocalBounds
	virtual AABox					GetLocalBounds() const override;
		
	// See Shape::GetWorldSpaceBounds
	virtual AABox					GetWorldSpaceBounds(Mat44Arg inCenterOfMassTransform, Vec3Arg inScale) const override;

	// See Shape::GetInnerRadius
	virtual float					GetInnerRadius() const override							{ return mInnerShape->GetInnerRadius(); }

	// See Shape::GetMassProperties
	virtual MassProperties			GetMassProperties() const override						{ return mInnerShape->GetMassProperties(); }

	// See Shape::GetSubShapeTransformedShape
	virtual TransformedShape		GetSubShapeTransformedShape(const SubShapeID &inSubShapeID, Vec3Arg inPositionCOM, QuatArg inRotation, Vec3Arg inScale, SubShapeID &outRemainder) const override;

	// See Shape::GetSurfaceNormal
	virtual Vec3					GetSurfaceNormal(const SubShapeID &inSubShapeID, Vec3Arg inLocalSurfacePosition) const override;

	// See Shape::GetSubmergedVolume
	virtual void					GetSubmergedVolume(Mat44Arg inCenterOfMassTransform, Vec3Arg inScale, const Plane &inSurface, float &outTotalVolume, float &outSubmergedVolume, Vec3 &outCenterOfBuoyancy) const override;

#ifdef JPH_DEBUG_RENDERER
	// See Shape::Draw
	virtual void					Draw(DebugRenderer *inRenderer, Mat44Arg inCenterOfMassTransform, Vec3Arg inScale, ColorArg inColor, bool inUseMaterialColors, bool inDrawWireframe) const override;

	// See Shape::DrawGetSupportFunction
	virtual void					DrawGetSupportFunction(DebugRenderer *inRenderer, Mat44Arg inCenterOfMassTransform, Vec3Arg inScale, ColorArg inColor, bool inDrawSupportDirection) const override;

	// See Shape::DrawGetSupportingFace
	virtual void					DrawGetSupportingFace(DebugRenderer *inRenderer, Mat44Arg inCenterOfMassTransform, Vec3Arg inScale) const override;
#endif // JPH_DEBUG_RENDERER

	// See Shape::CastRay
	virtual bool					CastRay(const RayCast &inRay, const SubShapeIDCreator &inSubShapeIDCreator, RayCastResult &ioHit) const override;
	virtual void					CastRay(const RayCast &inRay, const RayCastSettings &inRayCastSettings, const SubShapeIDCreator &inSubShapeIDCreator, CastRayCollector &ioCollector) const override;

	// See: Shape::CollidePoint
	virtual void					CollidePoint(Vec3Arg inPoint, const SubShapeIDCreator &inSubShapeIDCreator, CollidePointCollector &ioCollector) const override;

	// See Shape::CastShape
	virtual void					CastShape(const ShapeCast &inShapeCast, const ShapeCastSettings &inShapeCastSettings, Vec3Arg inScale, const ShapeFilter &inShapeFilter, Mat44Arg inCenterOfMassTransform2, const SubShapeIDCreator &inSubShapeIDCreator1, const SubShapeIDCreator &inSubShapeIDCreator2, CastShapeCollector &ioCollector) const override;

	// See Shape::CollectTransformedShapes
	virtual void					CollectTransformedShapes(const AABox &inBox, Vec3Arg inPositionCOM, QuatArg inRotation, Vec3Arg inScale, const SubShapeIDCreator &inSubShapeIDCreator, TransformedShapeCollector &ioCollector) const override;

	// See Shape::TransformShape
	virtual void					TransformShape(Mat44Arg inCenterOfMassTransform, TransformedShapeCollector &ioCollector) const override;

	// See Shape::GetTrianglesStart
	virtual void					GetTrianglesStart(GetTrianglesContext &ioContext, const AABox &inBox, Vec3Arg inPositionCOM, QuatArg inRotation, Vec3Arg inScale) const override { JPH_ASSERT(false, "Cannot call on non-leaf shapes, use CollectTransformedShapes to collect the leaves first!"); }

	// See Shape::GetTrianglesNext
	virtual int						GetTrianglesNext(GetTrianglesContext &ioContext, int inMaxTrianglesRequested, Float3 *outTriangleVertices, const PhysicsMaterial **outMaterials = nullptr) const override { JPH_ASSERT(false, "Cannot call on non-leaf shapes, use CollectTransformedShapes to collect the leaves first!"); return 0; }

	/// Collide 2 shapes and pass any hits on to ioCollector
	static void						sCollideOffsetCenterOfMassShapeVsShape(const OffsetCenterOfMassShape *inShape1, const Shape *inShape2, Vec3Arg inScale1, Vec3Arg inScale2, Mat44Arg inCenterOfMassTransform1, Mat44Arg inCenterOfMassTransform2, const SubShapeIDCreator &inSubShapeIDCreator1, const SubShapeIDCreator &inSubShapeIDCreator2, const CollideShapeSettings &inCollideShapeSettings, CollideShapeCollector &ioCollector);
	static void						sCollideShapeVsOffsetCenterOfMassShape(const Shape *inShape1, const OffsetCenterOfMassShape *inShape2, Vec3Arg inScale1, Vec3Arg inScale2, Mat44Arg inCenterOfMassTransform1, Mat44Arg inCenterOfMassTransform2, const SubShapeIDCreator &inSubShapeIDCreator1, const SubShapeIDCreator &inSubShapeIDCreator2, const CollideShapeSettings &inCollideShapeSettings, CollideShapeCollector &ioCollector);

	/// Cast an offset center of mass shape againt a shape, reports hits to ioCollector
	/// @param inShapeCast The shape to cast against the other shape and its start and direction
	/// @param inShapeCastSettings Settings for performing the cast
	/// @param inShape The shape to cast against.
	/// @param inScale Local space scale for the shape to cast against.
	/// @param inShapeFilter Determines if sub shapes of the shape can collide
	/// @param inCenterOfMassTransform2 Is the center of mass transform of shape 2 (excluding scale), this is used to provide a transform to the shape cast result so that local quantities can be transformed into world space.
	/// @param inSubShapeIDCreator1 Class that tracks the current sub shape ID for the casting shape
	/// @param inSubShapeIDCreator2 Class that tracks the current sub shape ID for the shape we're casting against
	/// @param ioCollector The collector that receives the results.
	static void						sCastOffsetCenterOfMassShapeVsShape(const ShapeCast &inShapeCast, const ShapeCastSettings &inShapeCastSettings, const Shape *inShape, Vec3Arg inScale, const ShapeFilter &inShapeFilter, Mat44Arg inCenterOfMassTransform2, const SubShapeIDCreator &inSubShapeIDCreator1, const SubShapeIDCreator &inSubShapeIDCreator2, CastShapeCollector &ioCollector);

	// See Shape
	virtual void					SaveBinaryState(StreamOut &inStream) const override;

	// See Shape::GetStats
	virtual Stats					GetStats() const override								{ return Stats(sizeof(*this), 0); }

	// See Shape::GetVolume
	virtual float					GetVolume() const override								{ return mInnerShape->GetVolume(); }

	// See Shape::IsValidScale
	virtual bool					IsValidScale(Vec3Arg inScale) const override			{ return mInnerShape->IsValidScale(inScale); }

protected:
	// See: Shape::RestoreBinaryState
	virtual void					RestoreBinaryState(StreamIn &inStream) override;
		
	Vec3							mCenterOfMass;											///< Position of the center of mass
	Vec3							mOffset;												///< Offset of the center of mass
};

} // JPH