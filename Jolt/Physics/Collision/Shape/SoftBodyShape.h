// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Physics/Collision/Shape/Shape.h>

JPH_NAMESPACE_BEGIN

class SoftBodyMotionProperties;

/// Shape used exclusively for soft bodies.
/// It adds collision detection to the soft body.
/// Used internally by the engine!
class JPH_EXPORT SoftBodyShape final : public Shape
{
public:
	JPH_OVERRIDE_NEW_DELETE

	/// Constructor
									SoftBodyShape() : Shape(EShapeType::SoftBody, EShapeSubType::SoftBody) { }

	/// Determine amount of bits needed to encode sub shape id
	uint							GetSubShapeIDBits() const;

	// See Shape
	virtual bool					MustBeStatic() const override							{ return false; }
	virtual Vec3					GetCenterOfMass() const override						{ return Vec3::sZero(); }
	virtual AABox					GetLocalBounds() const override;
	virtual uint					GetSubShapeIDBitsRecursive() const override				{ return GetSubShapeIDBits(); }
	virtual float					GetInnerRadius() const override							{ return 0.0f; /* TODO */ }
	virtual MassProperties			GetMassProperties() const								{ return MassProperties(); /* TODO */ }
	virtual const PhysicsMaterial *	GetMaterial(const SubShapeID &inSubShapeID) const override;
	virtual Vec3					GetSurfaceNormal(const SubShapeID &inSubShapeID, Vec3Arg inLocalSurfacePosition) const;
	virtual void					GetSupportingFace(const SubShapeID &inSubShapeID, Vec3Arg inDirection, Vec3Arg inScale, Mat44Arg inCenterOfMassTransform, SupportingFace &outVertices) const override { /* TODO */ }
	virtual void					GetSubmergedVolume(Mat44Arg inCenterOfMassTransform, Vec3Arg inScale, const Plane &inSurface, float &outTotalVolume, float &outSubmergedVolume, Vec3 &outCenterOfBuoyancy
#ifdef JPH_DEBUG_RENDERER // Not using JPH_IF_DEBUG_RENDERER for Doxygen
		, RVec3Arg inBaseOffset
#endif
		) const override { outSubmergedVolume = 0; outTotalVolume = 1.0f; outCenterOfBuoyancy = Vec3::sZero(); /* TODO */ }
#ifdef JPH_DEBUG_RENDERER
	virtual void					Draw(DebugRenderer *inRenderer, RMat44Arg inCenterOfMassTransform, Vec3Arg inScale, ColorArg inColor, bool inUseMaterialColors, bool inDrawWireframe) const override;
#endif // JPH_DEBUG_RENDERER
	virtual bool					CastRay(const RayCast &inRay, const SubShapeIDCreator &inSubShapeIDCreator, RayCastResult &ioHit) const override;
	virtual void					CastRay(const RayCast &inRay, const RayCastSettings &inRayCastSettings, const SubShapeIDCreator &inSubShapeIDCreator, CastRayCollector &ioCollector, const ShapeFilter &inShapeFilter = { }) const override;
	virtual void					CollidePoint(Vec3Arg inPoint, const SubShapeIDCreator &inSubShapeIDCreator, CollidePointCollector &ioCollector, const ShapeFilter &inShapeFilter = { }) const override { /* TODO */ }
	virtual void					GetTrianglesStart(GetTrianglesContext &ioContext, const AABox &inBox, Vec3Arg inPositionCOM, QuatArg inRotation, Vec3Arg inScale) const override { /* TODO */ }
	virtual int						GetTrianglesNext(GetTrianglesContext &ioContext, int inMaxTrianglesRequested, Float3 *outTriangleVertices, const PhysicsMaterial **outMaterials = nullptr) const override { /* TODO */ return 0; }
	virtual Stats					GetStats() const override { return Stats(sizeof(this), 1); /* TODO */ }
	virtual float					GetVolume() const override { /* TODO */ return 0.0f; }

	const SoftBodyMotionProperties *mSoftBodyMotionProperties;
};

JPH_NAMESPACE_END
