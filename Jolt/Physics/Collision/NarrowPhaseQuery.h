// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Physics/Body/BodyFilter.h>
#include <Jolt/Physics/Body/BodyLock.h>
#include <Jolt/Physics/Body/BodyLockInterface.h>
#include <Jolt/Physics/Collision/ShapeFilter.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhase.h>
#include <Jolt/Physics/Collision/BackFaceMode.h>

JPH_NAMESPACE_BEGIN

class Shape;
class CollideShapeSettings;
class RayCastResult;

/// Class that provides an interface for doing precise collision detection against the broad and then the narrow phase
class NarrowPhaseQuery : public NonCopyable
{
public:
	/// Initialize the interface (should only be called by PhysicsSystem)
	void						Init(BodyLockInterface &inBodyLockInterface, BroadPhase &inBroadPhase) { mBodyLockInterface = &inBodyLockInterface; mBroadPhase = &inBroadPhase; }

	/// Cast a ray and find the closest hit. Returns true if it finds a hit. Hits further than ioHit.mFraction will not be considered and in this case ioHit will remain unmodified (and the function will return false).
	/// Convex objects will be treated as solid (meaning if the ray starts inside, you'll get a hit fraction of 0) and back face hits against triangles are returned.
	/// If you want the surface normal of the hit use Body::GetWorldSpaceSurfaceNormal(ioHit.mSubShapeID2, inRay.GetPointOnRay(ioHit.mFraction)) on body with ID ioHit.mBodyID.
	bool						CastRay(const RayCast &inRay, RayCastResult &ioHit, const BroadPhaseLayerFilter &inBroadPhaseLayerFilter = { }, const ObjectLayerFilter &inObjectLayerFilter = { }, const BodyFilter &inBodyFilter = { }) const;

	/// Cast a ray, allows collecting multiple hits. Note that this version is more flexible but also slightly slower than the CastRay function that returns only a single hit.
	/// If you want the surface normal of the hit use Body::GetWorldSpaceSurfaceNormal(collected sub shape ID, inRay.GetPointOnRay(collected fraction)) on body with collected body ID.
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

JPH_NAMESPACE_END
