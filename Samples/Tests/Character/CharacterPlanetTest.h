// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>
#include <Jolt/Physics/Character/CharacterVirtual.h>
#include <Jolt/Physics/PhysicsStepListener.h>

class CharacterPlanetTest : public Test, public PhysicsStepListener, public CharacterContactListener
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, CharacterPlanetTest)

	// Description of the test
	virtual const char *	GetDescription() const override
	{
		return "Demonstrates how to do custom gravity to simulate a character walking on a planet.";
	}

	// Initialize the test
	virtual void			Initialize() override;

	// Process input
	virtual void			ProcessInput(const ProcessInputParams &inParams) override;

	// Update the test, called before the physics update
	virtual void			PrePhysicsUpdate(const PreUpdateParams &inParams) override;

	// Override to specify the initial camera state (local to GetCameraPivot)
	virtual void			GetInitialCamera(CameraState &ioState) const override;

	// Override to specify a camera pivot point and orientation (world space)
	virtual RMat44			GetCameraPivot(float inCameraHeading, float inCameraPitch) const override;

	// Saving / restoring state for replay
	virtual void			SaveState(StateRecorder &inStream) const override;
	virtual void			RestoreState(StateRecorder &inStream) override;

	// Saving / restoring controller input state for replay
	virtual void			SaveInputState(StateRecorder &inStream) const override;
	virtual void			RestoreInputState(StateRecorder &inStream) override;

	// See: PhysicsStepListener
	virtual void			OnStep(const PhysicsStepListenerContext &inContext) override;

	// See: CharacterContactListener
	virtual void			OnContactAdded(const CharacterVirtual *inCharacter, const BodyID &inBodyID2, const SubShapeID &inSubShapeID2, RVec3Arg inContactPosition, Vec3Arg inContactNormal, CharacterContactSettings &ioSettings) override;

private:
	// Planet size
	static constexpr float	cPlanetRadius = 20.0f;

	// Character size
	static constexpr float	cCharacterHeightStanding = 1.35f;
	static constexpr float	cCharacterRadiusStanding = 0.3f;
	static constexpr float	cCharacterSpeed = 6.0f;
	static constexpr float	cJumpSpeed = 4.0f;

	// The 'player' character
	Ref<CharacterVirtual>	mCharacter;

	// Player input
	Vec3					mDesiredVelocity = Vec3::sZero();
	Vec3					mDesiredVelocityWS = Vec3::sZero();
	bool					mJump = false;
	bool					mWasJump = false;
};
