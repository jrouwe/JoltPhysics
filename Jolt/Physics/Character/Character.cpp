// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyLock.h>
#include <Jolt/Physics/Collision/CollideShape.h>
#include <Jolt/Physics/Character/Character.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/ObjectStream/TypeDeclarations.h>

JPH_NAMESPACE_BEGIN

static inline const BodyLockInterface &sGetBodyLockInterface(const PhysicsSystem *inSystem, bool inLockBodies)
{
	return inLockBodies? static_cast<const BodyLockInterface &>(inSystem->GetBodyLockInterface()) : static_cast<const BodyLockInterface &>(inSystem->GetBodyLockInterfaceNoLock());
}

static inline BodyInterface &sGetBodyInterface(PhysicsSystem *inSystem, bool inLockBodies)
{
	return inLockBodies? inSystem->GetBodyInterface() : inSystem->GetBodyInterfaceNoLock();
}

static inline const NarrowPhaseQuery &sGetNarrowPhaseQuery(const PhysicsSystem *inSystem, bool inLockBodies)
{
	return inLockBodies? inSystem->GetNarrowPhaseQuery() : inSystem->GetNarrowPhaseQueryNoLock();
}

Character::Character(const CharacterSettings *inSettings, Vec3Arg inPosition, QuatArg inRotation, uint64 inUserData, PhysicsSystem *inSystem) :
	CharacterBase(inSettings, inSystem),
	mLayer(inSettings->mLayer)
{
	// Construct rigid body
	BodyCreationSettings settings(mShape, inPosition, inRotation, EMotionType::Dynamic, mLayer);
	settings.mFriction = inSettings->mFriction;
	settings.mGravityFactor = inSettings->mGravityFactor;
	settings.mUserData = inUserData;
	Body *body = mSystem->GetBodyInterface().CreateBody(settings);
	if (body != nullptr)
	{
		// Update the mass properties of the shape so that we set the correct mass and don't allow any rotation
		body->GetMotionProperties()->SetInverseMass(1.0f / inSettings->mMass);
		body->GetMotionProperties()->SetInverseInertia(Vec3::sZero(), Quat::sIdentity());

		mBodyID = body->GetID();
	}
}

Character::~Character()
{
	// Destroy the body
	mSystem->GetBodyInterface().DestroyBody(mBodyID);
}

void Character::AddToPhysicsSystem(EActivation inActivationMode, bool inLockBodies)
{
	sGetBodyInterface(mSystem, inLockBodies).AddBody(mBodyID, inActivationMode);
}

void Character::RemoveFromPhysicsSystem(bool inLockBodies)
{
	sGetBodyInterface(mSystem, inLockBodies).RemoveBody(mBodyID);
}

void Character::Activate(bool inLockBodies)
{
	sGetBodyInterface(mSystem, inLockBodies).ActivateBody(mBodyID);
}

void Character::CheckCollision(Mat44Arg inCenterOfMassTransform, Vec3Arg inMovementDirection, float inMaxSeparationDistance, const Shape *inShape, CollideShapeCollector &ioCollector, bool inLockBodies) const
{
	// Create query broadphase layer filter
	DefaultBroadPhaseLayerFilter broadphase_layer_filter = mSystem->GetDefaultBroadPhaseLayerFilter(mLayer);

	// Create query object layer filter
	DefaultObjectLayerFilter object_layer_filter = mSystem->GetDefaultLayerFilter(mLayer);

	// Ignore my own body
	IgnoreSingleBodyFilter body_filter(mBodyID);

	// Settings for collide shape
	CollideShapeSettings settings;
	settings.mMaxSeparationDistance = inMaxSeparationDistance;
	settings.mActiveEdgeMode = EActiveEdgeMode::CollideOnlyWithActive;
	settings.mActiveEdgeMovementDirection = inMovementDirection;
	settings.mBackFaceMode = EBackFaceMode::IgnoreBackFaces;

	sGetNarrowPhaseQuery(mSystem, inLockBodies).CollideShape(inShape, Vec3::sReplicate(1.0f), inCenterOfMassTransform, settings, ioCollector, broadphase_layer_filter, object_layer_filter, body_filter);
}

void Character::CheckCollision(Vec3Arg inPosition, QuatArg inRotation, Vec3Arg inMovementDirection, float inMaxSeparationDistance, const Shape *inShape, CollideShapeCollector &ioCollector, bool inLockBodies) const
{
	// Calculate center of mass transform
	Mat44 center_of_mass = Mat44::sRotationTranslation(inRotation, inPosition).PreTranslated(inShape->GetCenterOfMass());

	CheckCollision(center_of_mass, inMovementDirection, inMaxSeparationDistance, inShape, ioCollector, inLockBodies);
}

void Character::CheckCollision(const Shape *inShape, float inMaxSeparationDistance, CollideShapeCollector &ioCollector, bool inLockBodies) const
{
	// Determine position and velocity of body
	Mat44 query_transform;
	Vec3 velocity;
	{
		BodyLockRead lock(sGetBodyLockInterface(mSystem, inLockBodies), mBodyID);
		if (!lock.Succeeded())
			return;

		const Body &body = lock.GetBody();

		// Correct the center of mass transform for the difference between the old and new center of mass shape
		query_transform = body.GetCenterOfMassTransform().PreTranslated(inShape->GetCenterOfMass() - mShape->GetCenterOfMass());
		velocity = body.GetLinearVelocity();
	}

	CheckCollision(query_transform, velocity, inMaxSeparationDistance, inShape, ioCollector, inLockBodies);
}

void Character::PostSimulation(float inMaxSeparationDistance, bool inLockBodies)
{
	// Collector that finds the hit with the normal that is the most 'up'
	class MyCollector : public CollideShapeCollector
	{
	public:
		// Constructor
		explicit			MyCollector(Vec3Arg inGravity) : mGravity(inGravity) { }

		// See: CollectorType::AddHit
		virtual void		AddHit(const CollideShapeResult &inResult) override
		{
			Vec3 normal = -inResult.mPenetrationAxis.Normalized();
			float dot = normal.Dot(mGravity);
			if (dot < mBestDot) // Find the hit that is most opposite to the gravity
			{
				mGroundBodyID = inResult.mBodyID2;
				mGroundBodySubShapeID = inResult.mSubShapeID2;
				mGroundPosition = inResult.mContactPointOn2;
				mGroundNormal = normal;
				mBestDot = dot;
			}
		}

		BodyID				mGroundBodyID;
		SubShapeID			mGroundBodySubShapeID;
		Vec3				mGroundPosition = Vec3::sZero();
		Vec3				mGroundNormal = Vec3::sZero();

	private:
		float				mBestDot = FLT_MAX;
		Vec3				mGravity;
	};

	// Collide shape
	MyCollector collector(mSystem->GetGravity());
	CheckCollision(mShape, inMaxSeparationDistance, collector, inLockBodies);

	// Copy results
	mGroundBodyID = collector.mGroundBodyID;
	mGroundBodySubShapeID = collector.mGroundBodySubShapeID;
	mGroundPosition = collector.mGroundPosition;
	mGroundNormal = collector.mGroundNormal;

	// Get additional data from body
	BodyLockRead lock(sGetBodyLockInterface(mSystem, inLockBodies), mGroundBodyID);
	if (lock.Succeeded())
	{
		const Body &body = lock.GetBody();

		// Update ground state
		Vec3 up = -mSystem->GetGravity().Normalized();
		if (mGroundNormal.Dot(up) > mCosMaxSlopeAngle)
			mGroundState = EGroundState::OnGround;
		else
			mGroundState = EGroundState::Sliding;

		// Copy other body properties
		mGroundMaterial = body.GetShape()->GetMaterial(mGroundBodySubShapeID);
		mGroundVelocity = body.GetPointVelocity(mGroundPosition);
		mGroundUserData = body.GetUserData();
	}
	else
	{
		mGroundState = EGroundState::InAir;
		mGroundMaterial = PhysicsMaterial::sDefault;
		mGroundVelocity = Vec3::sZero();
		mGroundUserData = 0;
	}
}

void Character::SetLinearAndAngularVelocity(Vec3Arg inLinearVelocity, Vec3Arg inAngularVelocity, bool inLockBodies)
{
	sGetBodyInterface(mSystem, inLockBodies).SetLinearAndAngularVelocity(mBodyID, inLinearVelocity, inAngularVelocity);
}

Vec3 Character::GetLinearVelocity(bool inLockBodies) const
{
	return sGetBodyInterface(mSystem, inLockBodies).GetLinearVelocity(mBodyID);
}

void Character::SetLinearVelocity(Vec3Arg inLinearVelocity, bool inLockBodies)
{
	sGetBodyInterface(mSystem, inLockBodies).SetLinearVelocity(mBodyID, inLinearVelocity);
}

void Character::AddLinearVelocity(Vec3Arg inLinearVelocity, bool inLockBodies)
{
	sGetBodyInterface(mSystem, inLockBodies).AddLinearVelocity(mBodyID, inLinearVelocity);
}

void Character::AddImpulse(Vec3Arg inImpulse, bool inLockBodies)
{
	sGetBodyInterface(mSystem, inLockBodies).AddImpulse(mBodyID, inImpulse);
}

void Character::GetPositionAndRotation(Vec3 &outPosition, Quat &outRotation, bool inLockBodies) const
{
	sGetBodyInterface(mSystem, inLockBodies).GetPositionAndRotation(mBodyID, outPosition, outRotation);
}

void Character::SetPositionAndRotation(Vec3Arg inPosition, QuatArg inRotation, EActivation inActivationMode, bool inLockBodies) const
{
	sGetBodyInterface(mSystem, inLockBodies).SetPositionAndRotation(mBodyID, inPosition, inRotation, inActivationMode);
}

Vec3 Character::GetPosition(bool inLockBodies) const
{
	return sGetBodyInterface(mSystem, inLockBodies).GetPosition(mBodyID);
}

void Character::SetPosition(Vec3Arg inPosition, EActivation inActivationMode, bool inLockBodies) 
{
	sGetBodyInterface(mSystem, inLockBodies).SetPosition(mBodyID, inPosition, inActivationMode);
}

Quat Character::GetRotation(bool inLockBodies) const
{
	return sGetBodyInterface(mSystem, inLockBodies).GetRotation(mBodyID);
}

void Character::SetRotation(QuatArg inRotation, EActivation inActivationMode, bool inLockBodies) 
{
	sGetBodyInterface(mSystem, inLockBodies).SetRotation(mBodyID, inRotation, inActivationMode);
}

Vec3 Character::GetCenterOfMassPosition(bool inLockBodies) const
{
	return sGetBodyInterface(mSystem, inLockBodies).GetCenterOfMassPosition(mBodyID);
}

Mat44 Character::GetWorldTransform(bool inLockBodies) const
{
	return sGetBodyInterface(mSystem, inLockBodies).GetWorldTransform(mBodyID);
}

void Character::SetLayer(ObjectLayer inLayer, bool inLockBodies)
{
	mLayer = inLayer;

	sGetBodyInterface(mSystem, inLockBodies).SetObjectLayer(mBodyID, inLayer);
}

bool Character::SetShape(const Shape *inShape, float inMaxPenetrationDepth, bool inLockBodies)
{
	if (inMaxPenetrationDepth < FLT_MAX)
	{
		// Collector that checks if there is anything in the way while switching to inShape
		class MyCollector : public CollideShapeCollector
		{
		public:
			// Constructor
			explicit			MyCollector(float inMaxPenetrationDepth) : mMaxPenetrationDepth(inMaxPenetrationDepth) { }

			// See: CollectorType::AddHit
			virtual void		AddHit(const CollideShapeResult &inResult) override
			{
				if (inResult.mPenetrationDepth > mMaxPenetrationDepth)
				{
					mHadCollision = true;
					ForceEarlyOut();
				}
			}

			float				mMaxPenetrationDepth;
			bool				mHadCollision = false;
		};

		// Test if anything is in the way of switching
		MyCollector collector(inMaxPenetrationDepth);
		CheckCollision(inShape, 0.0f, collector, inLockBodies);
		if (collector.mHadCollision)
			return false;
	}

	// Switch the shape
	mShape = inShape;
	sGetBodyInterface(mSystem, inLockBodies).SetShape(mBodyID, mShape, false, EActivation::Activate);
	return true;
}

JPH_NAMESPACE_END
