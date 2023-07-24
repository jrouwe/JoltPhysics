// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Physics/SoftBody/SoftBodyParticleSettings.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Collision/CollisionGroup.h>

JPH_NAMESPACE_BEGIN

/// This class contains the information needed to create a soft body object
class JPH_EXPORT SoftBodyCreationSettings
{
public:
	/// Constructor
						SoftBodyCreationSettings() = default;
						SoftBodyCreationSettings(const SoftBodyParticleSettings *inSettings, RVec3Arg inPosition = RVec3::sZero(), QuatArg inRotation = Quat::sIdentity()) : mSettings(inSettings), mPosition(inPosition), mRotation(inRotation) { }

	RefConst<SoftBodyParticleSettings> mSettings;			///< Defines the configuration of this soft body

	RVec3				mPosition { RVec3::sZero() };		///< Initial position of the soft body
	Quat				mRotation { Quat::sIdentity() };	///< Initial rotation of the soft body

	/// User data value (can be used by application)
	uint64				mUserData = 0;

	///@name Collision settings
	ObjectLayer			mObjectLayer = 0;					///< The collision layer this body belongs to (determines if two objects can collide)
	CollisionGroup		mCollisionGroup;					///< The collision group this body belongs to (determines if two objects can collide)

	uint32				mNumIterations = 5;					///< Number of solver iterations
	float				mLinearDamping = 0.05f;				///< Linear damping: dv/dt = -mLinearDamping * v
	float				mRestitution = 0.0f;				///< Restitution when colliding
	float				mFriction = 0.2f;					///< Friction coefficient when colliding
	float				mPressure = 0.0f;					///< n * R * T, amount of substance * ideal gass constant * absolute temperature, see https://en.wikipedia.org/wiki/Pressure
	float				mGravityFactor = 1.0f;				///< Value to multiply gravity with for this body
	bool				mUpdatePosition = true;				///< Update the position of the body while simulating (set to false for something that is attached to the static world)
};

JPH_NAMESPACE_END
