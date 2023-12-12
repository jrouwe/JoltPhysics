// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Character/CharacterBaseTest.h>
#include <Jolt/Physics/Character/CharacterVirtual.h>
#include <Jolt/Physics/StateRecorderImpl.h>

// Simple test that test the CharacterVirtual class. Allows the user to move around with the arrow keys and jump with the J button.
class CharacterVirtualTest : public CharacterBaseTest, public CharacterContactListener
{
public:
	// Stores the frame information captured. See `mIsRecordingInput`.
	struct FrameSnapshot
	{
		string mInitialState;

		Vec3 mMovementDirection = Vec3::sZero();
		bool mJump = false;
		bool mSwitchStance = false;
	};

public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, CharacterVirtualTest)

	// Initialize the test
	virtual void			Initialize() override;

	// Fetches the character input to use on the next frame.
	virtual void			FetchNewInput(const PreUpdateParams &inParams, Vec3Arg& outMovementDirection, bool &outJump, bool &outSwitchStance);

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

	virtual const char*		GetAdditionalCharacterStateInfo() const override;

protected:
	// Get position of the character
	virtual RVec3			GetCharacterPosition() const override				{ return mCharacter->GetPosition(); }

	// Handle user input to the character
	virtual void			HandleInput(Vec3Arg inMovementDirection, bool inJump, bool inSwitchStance, float inDeltaTime) override;

	// Add character movement settings
	virtual void			AddCharacterMovementSettings(DebugUI* inUI, UIElement* inSubMenu) override;

	// Add test configuration settings
	virtual void			AddConfigurationSettings(DebugUI *inUI, UIElement *inSubMenu) override;

private:
	// Character movement settings
	static inline bool		sEnableCharacterInertia = true;

	// Test configuration settings
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

	// The player can toggle this by pressing M and record the next inputs until Y is pressed again.
	bool					mIsRecordingInput = false;

	// Used to track the current replaying frame. `-1` is used when the replay is off. The player can toggle this by pressing `U`.
	int						mReplayingFrame = -1;

	// The recorded frames, used to re-play.
	Array<FrameSnapshot>	mRecordedFrames;

	// Contains all the non deterministic frames, and it's used to print it on UI.
	Array<int>				mNonDeterministicFrames;
};
