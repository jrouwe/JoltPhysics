// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt.h>

#include <Physics/Body/BodyCreationSettings.h>
#include <Physics/Body/BodyLock.h>
#include <Physics/Collision/CollideShape.h>
#include <Physics/Character/Character.h>
#include <Physics/PhysicsSystem.h>
#include <ObjectStream/TypeDeclarations.h>

namespace JPH {

static inline BodyInterface &sGetBodyInterface(PhysicsSystem *inSystem, bool inLockBodies)
{
	return inLockBodies? inSystem->GetBodyInterface() : inSystem->GetBodyInterfaceNoLock();
}

static inline const NarrowPhaseQuery &sGetNarrowPhaseQuery(PhysicsSystem *inSystem, bool inLockBodies)
{
	return inLockBodies? inSystem->GetNarrowPhaseQuery() : inSystem->GetNarrowPhaseQueryNoLock();
}

Character::Character(CharacterSettings *inSettings, Vec3Arg inPosition, QuatArg inRotation, void *inUserData, PhysicsSystem *inSystem) :
	mLayer(inSettings->mLayer),
	mShape(inSettings->mShape),
	mSystem(inSystem),
	mGroundMaterial(PhysicsMaterial::sDefault)
{
	// Initialize max slope angle
	SetMaxSlopeAngle(inSettings->mMaxSlopeAngle);

	// Construct rigid body
	BodyCreationSettings settings(mShape, inPosition, inRotation, EMotionType::Dynamic, mLayer);
	settings.mFriction = inSettings->mFriction;
	settings.mGravityFactor = inSettings->mGravityFactor;
	Body *body = mSystem->GetBodyInterface().CreateBody(settings);
	if (body != nullptr)
	{
		body->SetUserData(inUserData);

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

void Character::CheckCollision(const Shape *inShape, float inMaxSeparationDistance, CollideShapeCollector &ioCollector, bool inLockBodies) const
{
	// Create query broadphase layer filter
	DefaultBroadPhaseLayerFilter broadphase_layer_filter = mSystem->GetDefaultBroadPhaseLayerFilter(mLayer);

	// Create query object layer filter
	DefaultObjectLayerFilter object_layer_filter = mSystem->GetDefaultLayerFilter(mLayer);

	// Ignore my own body
	IgnoreSingleBodyFilter body_filter(mBodyID);

	// Determine position to test
	Vec3 position;
	Quat rotation;
	BodyInterface &bi = sGetBodyInterface(mSystem, inLockBodies);
	bi.GetPositionAndRotation(mBodyID, position, rotation);
	Mat44 query_transform = Mat44::sRotationTranslation(rotation, position + rotation * inShape->GetCenterOfMass());

	// Settings for collide shape
	CollideShapeSettings settings;
	settings.mActiveEdgeMode = EActiveEdgeMode::CollideOnlyWithActive;
	settings.mActiveEdgeMovementDirection = bi.GetLinearVelocity(mBodyID);
	settings.mBackFaceMode = EBackFaceMode::IgnoreBackFaces;

	sGetNarrowPhaseQuery(mSystem, inLockBodies).CollideShape(inShape, Vec3::sReplicate(1.0f), query_transform, settings, ioCollector, broadphase_layer_filter, object_layer_filter, body_filter);
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
		float				mBestDot = 0.0f;
		Vec3				mGravity;
	};

	// Collide shape
	MyCollector collector(mSystem->GetGravity());
	CheckCollision(mShape, inMaxSeparationDistance, collector);

	// Copy results
	mGroundBodyID = collector.mGroundBodyID;
	mGroundPosition = collector.mGroundPosition;
	mGroundNormal = collector.mGroundNormal;
	mGroundMaterial = sGetBodyInterface(mSystem, inLockBodies).GetMaterial(collector.mGroundBodyID, collector.mGroundBodySubShapeID);
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
		CheckCollision(inShape, 0.0f, collector);
		if (collector.mHadCollision)
			return false;
	}

	// Switch the shape
	mShape = inShape;
	sGetBodyInterface(mSystem, inLockBodies).SetShape(mBodyID, mShape, false, EActivation::Activate);
	return true;
}

Character::EGroundState Character::GetGroundState() const
{ 
	if (mGroundBodyID.IsInvalid())
		return EGroundState::InAir;
	
	Vec3 up = -mSystem->GetGravity().Normalized();
	if (mGroundNormal.Dot(up) > mCosMaxSlopeAngle)
		return EGroundState::OnGround;
	else
		return EGroundState::Sliding;
}

void *Character::GetGroundUserData(bool inLockBodies) const
{
	return sGetBodyInterface(mSystem, inLockBodies).GetUserData(mGroundBodyID);
}

Vec3 Character::GetGroundVelocity(bool inLockBodies) const
{
	return sGetBodyInterface(mSystem, inLockBodies).GetPointVelocity(mGroundBodyID, mGroundPosition);
}

} // JPH