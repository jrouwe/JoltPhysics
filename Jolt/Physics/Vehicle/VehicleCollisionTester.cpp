// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Vehicle/VehicleCollisionTester.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/ShapeCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Jolt/Physics/PhysicsSystem.h>

JPH_NAMESPACE_BEGIN

bool VehicleCollisionTesterRay::Collide(PhysicsSystem &inPhysicsSystem, uint inWheelIndex, Vec3Arg inOrigin, Vec3Arg inDirection, float inSuspensionMaxLength, const BodyID &inVehicleBodyID, Body *&outBody, SubShapeID &outSubShapeID, Vec3 &outContactPosition, Vec3 &outContactNormal, float &outSuspensionLength) const
{
	DefaultBroadPhaseLayerFilter broadphase_layer_filter = inPhysicsSystem.GetDefaultBroadPhaseLayerFilter(mObjectLayer);
	DefaultObjectLayerFilter object_layer_filter = inPhysicsSystem.GetDefaultLayerFilter(mObjectLayer);
	IgnoreSingleBodyFilter body_filter(inVehicleBodyID);

	RayCast ray { inOrigin, inSuspensionMaxLength * inDirection };

	class MyCollector : public CastRayCollector
	{
	public:
							MyCollector(PhysicsSystem &inPhysicsSystem, const RayCast &inRay, Vec3Arg inUpDirection, float inCosMaxSlopeAngle) : 
			mPhysicsSystem(inPhysicsSystem),
			mRay(inRay),
			mUpDirection(inUpDirection),
			mCosMaxSlopeAngle(inCosMaxSlopeAngle)
		{
		}

		virtual void		AddHit(const RayCastResult &inResult) override
		{
			// Test if this collision is closer than the previous one
			if (inResult.mFraction < GetEarlyOutFraction())
			{
				// Lock the body
				BodyLockRead lock(mPhysicsSystem.GetBodyLockInterfaceNoLock(), inResult.mBodyID);
				JPH_ASSERT(lock.Succeeded()); // When this runs all bodies are locked so this should not fail
				const Body *body = &lock.GetBody();

				if (body->IsSensor())
					return;

				// Test that we're not hitting a vertical wall
				Vec3 contact_pos = mRay.GetPointOnRay(inResult.mFraction);
				Vec3 normal = body->GetWorldSpaceSurfaceNormal(inResult.mSubShapeID2, contact_pos);
				if (normal.Dot(mUpDirection) > mCosMaxSlopeAngle)
				{
					// Update early out fraction to this hit
					UpdateEarlyOutFraction(inResult.mFraction);

					// Get the contact properties
					mBody = body;
					mSubShapeID2 = inResult.mSubShapeID2;
					mContactPosition = contact_pos;
					mContactNormal = normal;
				}
			}
		}

		// Configuration
		PhysicsSystem &		mPhysicsSystem;
		RayCast				mRay;
		Vec3				mUpDirection;
		float				mCosMaxSlopeAngle;

		// Resulting closest collision
		const Body *		mBody = nullptr;
		SubShapeID			mSubShapeID2;
		Vec3				mContactPosition;
		Vec3				mContactNormal;
	};

	RayCastSettings settings;

	MyCollector collector(inPhysicsSystem, ray, mUp, mCosMaxSlopeAngle);
	inPhysicsSystem.GetNarrowPhaseQueryNoLock().CastRay(ray, settings, collector, broadphase_layer_filter, object_layer_filter, body_filter);
	if (collector.mBody == nullptr)
		return false;

	outBody = const_cast<Body *>(collector.mBody);
	outSubShapeID = collector.mSubShapeID2;
	outContactPosition = collector.mContactPosition;
	outContactNormal = collector.mContactNormal;
	outSuspensionLength = inSuspensionMaxLength * collector.GetEarlyOutFraction();

	return true;
}

bool VehicleCollisionTesterCastSphere::Collide(PhysicsSystem &inPhysicsSystem, uint inWheelIndex, Vec3Arg inOrigin, Vec3Arg inDirection, float inSuspensionMaxLength, const BodyID &inVehicleBodyID, Body *&outBody, SubShapeID &outSubShapeID, Vec3 &outContactPosition, Vec3 &outContactNormal, float &outSuspensionLength) const
{
	DefaultBroadPhaseLayerFilter broadphase_layer_filter = inPhysicsSystem.GetDefaultBroadPhaseLayerFilter(mObjectLayer);
	DefaultObjectLayerFilter object_layer_filter = inPhysicsSystem.GetDefaultLayerFilter(mObjectLayer);
	IgnoreSingleBodyFilter body_filter(inVehicleBodyID);

	SphereShape sphere(mRadius);
	sphere.SetEmbedded();

	float cast_length = max(0.0f, inSuspensionMaxLength - mRadius);
	ShapeCast shape_cast(&sphere, Vec3::sReplicate(1.0f), Mat44::sTranslation(inOrigin), inDirection * cast_length);

	ShapeCastSettings settings;
	settings.mUseShrunkenShapeAndConvexRadius = true;
	settings.mReturnDeepestPoint = true;

	class MyCollector : public CastShapeCollector
	{
	public:
							MyCollector(PhysicsSystem &inPhysicsSystem, const ShapeCast &inShapeCast, Vec3Arg inUpDirection, float inCosMaxSlopeAngle) : 
			mPhysicsSystem(inPhysicsSystem),
			mShapeCast(inShapeCast),
			mUpDirection(inUpDirection),
			mCosMaxSlopeAngle(inCosMaxSlopeAngle)
		{
		}

		virtual void		AddHit(const ShapeCastResult &inResult) override
		{
			// Test if this collision is closer than the previous one
			if (inResult.mFraction < GetEarlyOutFraction())
			{
				// Lock the body
				BodyLockRead lock(mPhysicsSystem.GetBodyLockInterfaceNoLock(), inResult.mBodyID2);
				JPH_ASSERT(lock.Succeeded()); // When this runs all bodies are locked so this should not fail
				const Body *body = &lock.GetBody();

				if (body->IsSensor())
					return;

				// Test that we're not hitting a vertical wall
				Vec3 normal = -inResult.mPenetrationAxis.Normalized();
				if (normal.Dot(mUpDirection) > mCosMaxSlopeAngle)
				{
					// Update early out fraction to this hit
					UpdateEarlyOutFraction(inResult.mFraction);

					// Get the contact properties
					mBody = body;
					mSubShapeID2 = inResult.mSubShapeID2;
					mContactPosition = inResult.mContactPointOn2;
					mContactNormal = normal;
				}
			}
		}

		// Configuration
		PhysicsSystem &		mPhysicsSystem;
		const ShapeCast &	mShapeCast;
		Vec3				mUpDirection;
		float				mCosMaxSlopeAngle;

		// Resulting closest collision
		const Body *		mBody = nullptr;
		SubShapeID			mSubShapeID2;
		Vec3				mContactPosition;
		Vec3				mContactNormal;
	};

	MyCollector collector(inPhysicsSystem, shape_cast, mUp, mCosMaxSlopeAngle);
	inPhysicsSystem.GetNarrowPhaseQueryNoLock().CastShape(shape_cast, settings, collector, broadphase_layer_filter, object_layer_filter, body_filter);
	if (collector.mBody == nullptr)
		return false;

	outBody = const_cast<Body *>(collector.mBody);
	outSubShapeID = collector.mSubShapeID2;
	outContactPosition = collector.mContactPosition;
	outContactNormal = collector.mContactNormal;
	outSuspensionLength = min(inSuspensionMaxLength, cast_length * collector.GetEarlyOutFraction() + mRadius);

	return true;
}

JPH_NAMESPACE_END
