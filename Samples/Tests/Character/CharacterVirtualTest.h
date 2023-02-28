// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Character/CharacterBaseTest.h>
#include <Jolt/Physics/Character/CharacterVirtual.h>

// Simple test that test the CharacterVirtual class. Allows the user to move around with the arrow keys and jump with the J button.
class CharacterVirtualTest : public CharacterBaseTest, public CharacterContactListener
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(CharacterVirtualTest)

	// Initialize the test
	virtual void			Initialize() override;

	// Update the test, called before the physics update
	virtual void			PrePhysicsUpdate(const PreUpdateParams &inParams) override;

	// Saving / restoring state for replay
	virtual void			SaveState(StateRecorder &inStream) const override;
	virtual void			RestoreState(StateRecorder &inStream) override;

	/// Callback to adjust the velocity of a body as seen by the character. Can be adjusted to e.g. implement a conveyor belt or an inertial dampener system of a sci-fi space ship.
	virtual void			OnAdjustBodyVelocity(const CharacterVirtual *inCharacter, const Body &inBody2, Vec3 &ioLinearVelocity, Vec3 &ioAngularVelocity) override;

	// Called whenever the character collides with a body. Returns true if the contact can push the character.
	virtual void			OnContactAdded(const CharacterVirtual *inCharacter, const BodyID &inBodyID2, const SubShapeID &inSubShapeID2, RVec3Arg inContactPosition, Vec3Arg inContactNormal, CharacterContactSettings &ioSettings) override;

	// Called whenever the character movement is solved and a constraint is hit. Allows the listener to override the resulting character velocity (e.g. by preventing sliding along certain surfaces).
	virtual void			OnContactSolve(const CharacterVirtual *inCharacter, const BodyID &inBodyID2, const SubShapeID &inSubShapeID2, RVec3Arg inContactPosition, Vec3Arg inContactNormal, Vec3Arg inContactVelocity, const PhysicsMaterial *inContactMaterial, Vec3Arg inCharacterVelocity, Vec3 &ioNewCharacterVelocity) override;

protected:
	// Get position of the character
	virtual RVec3			GetCharacterPosition() const override				{ return mCharacter->GetPosition(); }

	// Handle user input to the character
	virtual void			HandleInput(Vec3Arg inMovementDirection, bool inJump, bool inSwitchStance, float inDeltaTime) override;

	// Add test configuration settings
	virtual void			AddConfigurationSettings(DebugUI *inUI, UIElement *inSubMenu) override;

private:
	// Test settings
	static inline EBackFaceMode sBackFaceMode = EBackFaceMode::CollideWithBackFaces;
	static inline float		sUpRotationX = 0;
	static inline float		sUpRotationZ = 0;	
	static inline float		sMaxSlopeAngle = DegreesToRadians(45.0f);
	static inline float		sMaxStrength = 100.0f;
	static inline float		sCharacterPadding = 0.02f;
	static inline float		sPenetrationRecoverySpeed = 1.0f;
	static inline float		sPredictiveContactDistance = 0.1f;
	static inline bool		sEnableWalkStairs = true;
	static inline bool		sEnableStickToFloor = true;

	// The 'player' character
	Ref<CharacterVirtual>	mCharacter;

	// Smoothed value of the player input
	Vec3					mDesiredVelocity = Vec3::sZero();

	// True when the player is pressing movement controls
	bool					mAllowSliding = false;
};
