// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Physics/Body/BodyFilter.h>
#include <Physics/Body/BodyLock.h>
#include <Physics/Body/BodyLockInterface.h>
#include <Physics/Collision/ShapeFilter.h>
#include <Physics/Collision/BroadPhase/BroadPhase.h>
#include <Physics/Collision/BackFaceMode.h>

namespace JPH {

class Shape;
class CollideShapeSettings;
class RayCastResult;

/// Class that provides an interface for doing precise collision detection against the broad and then the narrow phase
class NarrowPhaseQuery : public NonCopyable
{
public:
	/// Constructor
								NarrowPhaseQuery() = default;
								NarrowPhaseQuery(BodyLockInterface &inBodyLockInterface, BroadPhase &inBroadPhase) : mBodyLockInterface(&inBodyLockInterface), mBroadPhase(&inBroadPhase) { }

	/// Cast a ray, returns true if it finds a hit closer than ioHit.mFraction and updates ioHit in that case.
	bool						CastRay(const RayCast &inRay, RayCastResult &ioHit, const BroadPhaseLayerFilter &inBroadPhaseLayerFilter = { }, const ObjectLayerFilter &inObjectLayerFilter = { }, const BodyFilter &inBodyFilter = { }) const;

	/// Cast a ray, allows collecting multiple hits
	void						CastRay(const RayCast &inRay, const RayCastSettings &inRayCastSettings, CastRayCollector &ioCollector, const BroadPhaseLayerFilter &inBroadPhaseLayerFilter = { }, const ObjectLayerFilter &inObjectLayerFilter = { }, const BodyFilter &inBodyFilter = { }) const;

	/// Check if inPoint is inside any shapes. For this tests all shapes are treated as if they were solid. 
	/// For a mesh shape, this test will only provide sensible information if the mesh is a closed manifold.
	/// For each shape that collides, ioCollector will receive a hit
	void						CollidePoint(Vec3Arg inPoint, CollidePointCollector &ioCollector, const BroadPhaseLayerFilter &inBroadPhaseLayerFilter = { }, const ObjectLayerFilter &inObjectLayerFilter = { }, const BodyFilter &inBodyFilter = { }) const;

	/// Collide a shape with the system
	void						CollideShape(const Shape *inShape, Vec3Arg inShapeScale, Mat44Arg inCenterOfMassTransform, const CollideShapeSettings &inCollideShapeSettings, CollideShapeCollector &ioCollector, const BroadPhaseLayerFilter &inBroadPhaseLayerFilter = { }, const ObjectLayerFilter &inObjectLayerFilter = { }, const BodyFilter &inBodyFilter = { }) const;

	/// Cast a shape and report any hits to ioCollector
	void						CastShape(const ShapeCast &inShapeCast, const ShapeCastSettings &inShapeCastSettings, CastShapeCollector &ioCollector, const BroadPhaseLayerFilter &inBroadPhaseLayerFilter = { }, const ObjectLayerFilter &inObjectLayerFilter = { }, const BodyFilter &inBodyFilter = { }, const ShapeFilter &inShapeFilter = { }) const;

	/// Collect all leaf transformed shapes that fall inside world space box inBox
	void						CollectTransformedShapes(const AABox &inBox, TransformedShapeCollector &ioCollector, const BroadPhaseLayerFilter &inBroadPhaseLayerFilter = { }, const ObjectLayerFilter &inObjectLayerFilter = { }, const BodyFilter &inBodyFilter = { }) const;

private:
	BodyLockInterface *			mBodyLockInterface = nullptr;
	BroadPhase *				mBroadPhase = nullptr;
};

} // JPH