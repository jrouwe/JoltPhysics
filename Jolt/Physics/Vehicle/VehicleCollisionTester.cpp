// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Vehicle/VehicleCollisionTester.h>
#include <Jolt/Physics/Vehicle/VehicleConstraint.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/ShapeCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CylinderShape.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Jolt/Physics/PhysicsSystem.h>

JPH_NAMESPACE_BEGIN

bool VehicleCollisionTesterRay::Collide(PhysicsSystem &inPhysicsSystem, const VehicleConstraint &inVehicleConstraint, uint inWheelIndex, RVec3Arg inOrigin, Vec3Arg inDirection, const BodyID &inVehicleBodyID, Body *&outBody, SubShapeID &outSubShapeID, RVec3 &outContactPosition, Vec3 &outContactNormal, float &outSuspensionLength) const
{
	const DefaultBroadPhaseLayerFilter default_broadphase_layer_filter = inPhysicsSystem.GetDefaultBroadPhaseLayerFilter(mObjectLayer);
	const BroadPhaseLayerFilter &broadphase_layer_filter = mBroadPhaseLayerFilter != nullptr? *mBroadPhaseLayerFilter : default_broadphase_layer_filter;

	const DefaultObjectLayerFilter default_object_layer_filter = inPhysicsSystem.GetDefaultLayerFilter(mObjectLayer);
	const ObjectLayerFilter &object_layer_filter = mObjectLayerFilter != nullptr? *mObjectLayerFilter : default_object_layer_filter;

	const IgnoreSingleBodyFilter default_body_filter(inVehicleBodyID);
	const BodyFilter &body_filter = mBodyFilter != nullptr? *mBodyFilter : default_body_filter;

	const WheelSettings *wheel_settings = inVehicleConstraint.GetWheel(inWheelIndex)->GetSettings();
	float wheel_radius = wheel_settings->mRadius;
	float ray_length = wheel_settings->mSuspensionMaxLength + wheel_radius;
	RRayCast ray { inOrigin, ray_length * inDirection };

	class MyCollector : public CastRayCollector
	{
	public:
							MyCollector(PhysicsSystem &inPhysicsSystem, const RRayCast &inRay, Vec3Arg inUpDirection, float inCosMaxSlopeAngle) :
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
				RVec3 contact_pos = mRay.GetPointOnRay(inResult.mFraction);
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
		RRayCast			mRay;
		Vec3				mUpDirection;
		float				mCosMaxSlopeAngle;

		// Resulting closest collision
		const Body *		mBody = nullptr;
		SubShapeID			mSubShapeID2;
		RVec3				mContactPosition;
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
	outSuspensionLength = max(0.0f, ray_length * collector.GetEarlyOutFraction() - wheel_radius);

	return true;
}

bool VehicleCollisionTesterCastSphere::Collide(PhysicsSystem &inPhysicsSystem, const VehicleConstraint &inVehicleConstraint, uint inWheelIndex, RVec3Arg inOrigin, Vec3Arg inDirection, const BodyID &inVehicleBodyID, Body *&outBody, SubShapeID &outSubShapeID, RVec3 &outContactPosition, Vec3 &outContactNormal, float &outSuspensionLength) const
{
	const DefaultBroadPhaseLayerFilter default_broadphase_layer_filter = inPhysicsSystem.GetDefaultBroadPhaseLayerFilter(mObjectLayer);
	const BroadPhaseLayerFilter &broadphase_layer_filter = mBroadPhaseLayerFilter != nullptr? *mBroadPhaseLayerFilter : default_broadphase_layer_filter;

	const DefaultObjectLayerFilter default_object_layer_filter = inPhysicsSystem.GetDefaultLayerFilter(mObjectLayer);
	const ObjectLayerFilter &object_layer_filter = mObjectLayerFilter != nullptr? *mObjectLayerFilter : default_object_layer_filter;

	const IgnoreSingleBodyFilter default_body_filter(inVehicleBodyID);
	const BodyFilter &body_filter = mBodyFilter != nullptr? *mBodyFilter : default_body_filter;

	SphereShape sphere(mRadius);
	sphere.SetEmbedded();

	const WheelSettings *wheel_settings = inVehicleConstraint.GetWheel(inWheelIndex)->GetSettings();
	float wheel_radius = wheel_settings->mRadius;
	float shape_cast_length = wheel_settings->mSuspensionMaxLength + wheel_radius - mRadius;
	RShapeCast shape_cast(&sphere, Vec3::sReplicate(1.0f), RMat44::sTranslation(inOrigin), inDirection * shape_cast_length);

	ShapeCastSettings settings;
	settings.mUseShrunkenShapeAndConvexRadius = true;
	settings.mReturnDeepestPoint = true;

	class MyCollector : public CastShapeCollector
	{
	public:
							MyCollector(PhysicsSystem &inPhysicsSystem, const RShapeCast &inShapeCast, Vec3Arg inUpDirection, float inCosMaxSlopeAngle) :
			mPhysicsSystem(inPhysicsSystem),
			mShapeCast(inShapeCast),
			mUpDirection(inUpDirection),
			mCosMaxSlopeAngle(inCosMaxSlopeAngle)
		{
		}

		virtual void		AddHit(const ShapeCastResult &inResult) override
		{
			// Test if this collision is closer/deeper than the previous one
			float early_out = inResult.GetEarlyOutFraction();
			if (early_out < GetEarlyOutFraction())
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
					UpdateEarlyOutFraction(early_out);

					// Get the contact properties
					mBody = body;
					mSubShapeID2 = inResult.mSubShapeID2;
					mContactPosition = mShapeCast.mCenterOfMassStart.GetTranslation() + inResult.mContactPointOn2;
					mContactNormal = normal;
					mFraction = inResult.mFraction;
				}
			}
		}

		// Configuration
		PhysicsSystem &		mPhysicsSystem;
		const RShapeCast &	mShapeCast;
		Vec3				mUpDirection;
		float				mCosMaxSlopeAngle;

		// Resulting closest collision
		const Body *		mBody = nullptr;
		SubShapeID			mSubShapeID2;
		RVec3				mContactPosition;
		Vec3				mContactNormal;
		float				mFraction;
	};

	MyCollector collector(inPhysicsSystem, shape_cast, mUp, mCosMaxSlopeAngle);
	inPhysicsSystem.GetNarrowPhaseQueryNoLock().CastShape(shape_cast, settings, shape_cast.mCenterOfMassStart.GetTranslation(), collector, broadphase_layer_filter, object_layer_filter, body_filter);
	if (collector.mBody == nullptr)
		return false;

	outBody = const_cast<Body *>(collector.mBody);
	outSubShapeID = collector.mSubShapeID2;
	outContactPosition = collector.mContactPosition;
	outContactNormal = collector.mContactNormal;
	outSuspensionLength = max(0.0f, shape_cast_length * collector.mFraction + mRadius - wheel_radius);

	return true;
}

bool VehicleCollisionTesterCastCylinder::Collide(PhysicsSystem &inPhysicsSystem, const VehicleConstraint &inVehicleConstraint, uint inWheelIndex, RVec3Arg inOrigin, Vec3Arg inDirection, const BodyID &inVehicleBodyID, Body *&outBody, SubShapeID &outSubShapeID, RVec3 &outContactPosition, Vec3 &outContactNormal, float &outSuspensionLength) const
{
	const DefaultBroadPhaseLayerFilter default_broadphase_layer_filter = inPhysicsSystem.GetDefaultBroadPhaseLayerFilter(mObjectLayer);
	const BroadPhaseLayerFilter &broadphase_layer_filter = mBroadPhaseLayerFilter != nullptr? *mBroadPhaseLayerFilter : default_broadphase_layer_filter;

	const DefaultObjectLayerFilter default_object_layer_filter = inPhysicsSystem.GetDefaultLayerFilter(mObjectLayer);
	const ObjectLayerFilter &object_layer_filter = mObjectLayerFilter != nullptr? *mObjectLayerFilter : default_object_layer_filter;

	const IgnoreSingleBodyFilter default_body_filter(inVehicleBodyID);
	const BodyFilter &body_filter = mBodyFilter != nullptr? *mBodyFilter : default_body_filter;

	const WheelSettings *wheel_settings = inVehicleConstraint.GetWheel(inWheelIndex)->GetSettings();
	float max_suspension_length = wheel_settings->mSuspensionMaxLength;

	// Get the wheel transform given that the cylinder rotates around the Y axis
	RMat44 shape_cast_start = inVehicleConstraint.GetWheelWorldTransform(inWheelIndex, Vec3::sAxisY(), Vec3::sAxisX());
	shape_cast_start.SetTranslation(inOrigin);

	// Construct a cylinder with the dimensions of the wheel
	float wheel_half_width = 0.5f * wheel_settings->mWidth;
	CylinderShape cylinder(wheel_half_width, wheel_settings->mRadius, min(wheel_half_width, wheel_settings->mRadius) * mConvexRadiusFraction);
	cylinder.SetEmbedded();

	RShapeCast shape_cast(&cylinder, Vec3::sReplicate(1.0f), shape_cast_start, inDirection * max_suspension_length);

	ShapeCastSettings settings;
	settings.mUseShrunkenShapeAndConvexRadius = true;
	settings.mReturnDeepestPoint = true;

	class MyCollector : public CastShapeCollector
	{
	public:
							MyCollector(PhysicsSystem &inPhysicsSystem, const RShapeCast &inShapeCast) :
			mPhysicsSystem(inPhysicsSystem),
			mShapeCast(inShapeCast)
		{
		}

		virtual void		AddHit(const ShapeCastResult &inResult) override
		{
			// Test if this collision is closer/deeper than the previous one
			float early_out = inResult.GetEarlyOutFraction();
			if (early_out < GetEarlyOutFraction())
			{
				// Lock the body
				BodyLockRead lock(mPhysicsSystem.GetBodyLockInterfaceNoLock(), inResult.mBodyID2);
				JPH_ASSERT(lock.Succeeded()); // When this runs all bodies are locked so this should not fail
				const Body *body = &lock.GetBody();

				if (body->IsSensor())
					return;

				// Update early out fraction to this hit
				UpdateEarlyOutFraction(early_out);

				// Get the contact properties
				mBody = body;
				mSubShapeID2 = inResult.mSubShapeID2;
				mContactPosition = mShapeCast.mCenterOfMassStart.GetTranslation() + inResult.mContactPointOn2;
				mContactNormal = -inResult.mPenetrationAxis.Normalized();
				mFraction = inResult.mFraction;
			}
		}

		// Configuration
		PhysicsSystem &		mPhysicsSystem;
		const RShapeCast &	mShapeCast;

		// Resulting closest collision
		const Body *		mBody = nullptr;
		SubShapeID			mSubShapeID2;
		RVec3				mContactPosition;
		Vec3				mContactNormal;
		float				mFraction;
	};

	MyCollector collector(inPhysicsSystem, shape_cast);
	inPhysicsSystem.GetNarrowPhaseQueryNoLock().CastShape(shape_cast, settings, shape_cast.mCenterOfMassStart.GetTranslation(), collector, broadphase_layer_filter, object_layer_filter, body_filter);
	if (collector.mBody == nullptr)
		return false;

	outBody = const_cast<Body *>(collector.mBody);
	outSubShapeID = collector.mSubShapeID2;
	outContactPosition = collector.mContactPosition;
	outContactNormal = collector.mContactNormal;
	outSuspensionLength = max_suspension_length * collector.mFraction;

	return true;
}

JPH_NAMESPACE_END
