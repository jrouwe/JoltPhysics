// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Physics/SoftBody/SoftBodySharedSettings.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Collision/CollisionGroup.h>
#include <Jolt/ObjectStream/SerializableObject.h>

JPH_NAMESPACE_BEGIN

class StreamIn;
class StreamOut;

/// This class contains the information needed to create a soft body object
class JPH_EXPORT SoftBodyCreationSettings
{
public:
	JPH_DECLARE_SERIALIZABLE_NON_VIRTUAL(JPH_EXPORT, SoftBodyCreationSettings)

	/// Constructor
						SoftBodyCreationSettings() = default;
						SoftBodyCreationSettings(const SoftBodySharedSettings *inSettings, RVec3Arg inPosition = RVec3::sZero(), QuatArg inRotation = Quat::sIdentity()) : mSettings(inSettings), mPosition(inPosition), mRotation(inRotation) { }

	/// Saves the state of this object in binary form to inStream. Doesn't store the shared settings nor the group filter.
	void				SaveBinaryState(StreamOut &inStream) const;

	/// Restore the state of this object from inStream. Doesn't restore the shared settings nor the group filter.
	void				RestoreBinaryState(StreamIn &inStream);

	RefConst<SoftBodySharedSettings> mSettings;				///< Defines the configuration of this soft body

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
