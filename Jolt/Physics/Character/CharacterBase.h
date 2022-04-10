// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Core/Reference.h>
#include <Jolt/Core/NonCopyable.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>
#include <Jolt/Physics/Collision/Shape/SubShapeID.h>
#include <Jolt/Physics/Collision/PhysicsMaterial.h>

JPH_NAMESPACE_BEGIN

class PhysicsSystem;
class StateRecorder;

/// Base class for configuration of a character
class CharacterBaseSettings : public RefTarget<CharacterBaseSettings>
{
public:
	/// Maximum angle of slope that character can still walk on (radians).
	float								mMaxSlopeAngle = DegreesToRadians(50.0f);

	/// Initial shape that represents the character's volume.
	/// Usually this is a capsule, make sure the shape is made so that the bottom of the shape is at (0, 0, 0).
	RefConst<Shape>						mShape;
};

/// Base class for character class
class CharacterBase : public RefTarget<CharacterBase>, public NonCopyable
{
public:
	/// Constructor
										CharacterBase(const CharacterBaseSettings *inSettings, PhysicsSystem *inSystem);

	/// Destructor
	virtual								~CharacterBase() = default;

	/// Set the maximum angle of slope that character can still walk on (radians)
	void								SetMaxSlopeAngle(float inMaxSlopeAngle)					{ mCosMaxSlopeAngle = cos(inMaxSlopeAngle); }

	/// Get the current shape that the character is using.
	const Shape *						GetShape() const										{ return mShape; }

	enum class EGroundState
	{
		OnGround,						///< Character is on the ground and can move freely
		Sliding,						///< Character is on a slope that is too steep and should start sliding
		InAir,							///< Character is in the air
	};

	///@name Properties of the ground this character is standing on

	/// Current ground state
	EGroundState						GetGroundState() const									{ return mGroundState; }

	/// Get the contact point with the ground
	Vec3 								GetGroundPosition() const								{ return mGroundPosition; }

	/// Get the contact normal with the ground
	Vec3	 							GetGroundNormal() const									{ return mGroundNormal; }

	/// Velocity in world space of ground
	Vec3								GetGroundVelocity() const								{ return mGroundVelocity; }
	
	/// Material that the character is standing on
	const PhysicsMaterial *				GetGroundMaterial() const								{ return mGroundMaterial; }

	/// BodyID of the object the character is standing on. Note may have been removed!
	BodyID								GetGroundBodyID() const									{ return mGroundBodyID; }

	/// Sub part of the body that we're standing on.
	SubShapeID							GetGroundSubShapeID() const								{ return mGroundBodySubShapeID; }

	/// User data value of the body that we're standing on
	uint64								GetGroundUserData() const								{ return mGroundUserData; }

	// Saving / restoring state for replay
	virtual void						SaveState(StateRecorder &inStream) const;
	virtual void						RestoreState(StateRecorder &inStream);

protected:
	// Cached physics system
	PhysicsSystem *						mSystem;

	// The shape that the body currently has
	RefConst<Shape>						mShape;

	// Cosine of the maximum angle of slope that character can still walk on
	float								mCosMaxSlopeAngle;

	// Ground properties
	EGroundState						mGroundState = EGroundState::InAir;
	BodyID								mGroundBodyID;
	SubShapeID							mGroundBodySubShapeID;
	Vec3								mGroundPosition = Vec3::sZero();
	Vec3								mGroundNormal = Vec3::sZero();
	Vec3								mGroundVelocity = Vec3::sZero();
	RefConst<PhysicsMaterial>			mGroundMaterial = PhysicsMaterial::sDefault;
	uint64								mGroundUserData = 0;
};

JPH_NAMESPACE_END
