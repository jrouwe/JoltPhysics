// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>
#include <Jolt/Physics/Character/CharacterBase.h>

// Base class for the character tests, initializes the test scene.
class CharacterBaseTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(CharacterBaseTest)

	// Number used to scale the terrain and camera movement to the scene
	virtual float			GetWorldScale() const override								{ return 0.2f; }

	// Initialize the test
	virtual void			Initialize() override;

	// Update the test, called before the physics update
	virtual void			PrePhysicsUpdate(const PreUpdateParams &inParams) override;

	// Override to specify the initial camera state (local to GetCameraPivot)
	virtual void			GetInitialCamera(CameraState &ioState) const override;

	// Override to specify a camera pivot point and orientation (world space)
	virtual Mat44			GetCameraPivot(float inCameraHeading, float inCameraPitch) const override;

	// Optional settings menu
	virtual bool			HasSettingsMenu() const override							{ return true; }
	virtual void			CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu) override;

	// Saving / restoring state for replay
	virtual void			SaveState(StateRecorder &inStream) const override;
	virtual void			RestoreState(StateRecorder &inStream) override;

protected:
	// Get position of the character
	virtual Vec3			GetCharacterPosition() const = 0;

	// Handle user input to the character
	virtual void			HandleInput(Vec3Arg inMovementDirection, bool inJump, bool inSwitchStance, float inDeltaTime) = 0;

	// Draw the character state
	void					DrawCharacterState(const CharacterBase *inCharacter, Mat44Arg inCharacterTransform, Vec3Arg inCharacterVelocity);

	// Character size
	static constexpr float	cCharacterHeightStanding = 1.35f;
	static constexpr float	cCharacterRadiusStanding = 0.3f;
	static constexpr float	cCharacterHeightCrouching = 0.8f;
	static constexpr float	cCharacterRadiusCrouching = 0.3f;
	static constexpr float	cCharacterSpeed = 6.0f;
	static constexpr float	cJumpSpeed = 4.0f;

	// The different stances for the character
	RefConst<Shape>			mStandingShape;
	RefConst<Shape>			mCrouchingShape;

	// List of boxes on ramp
	vector<BodyID>			mRampBlocks;
	float					mRampBlocksTimeLeft = 0.0f;

private:
	// List of possible scene names
	static const char *		sScenes[];

	// Filename of animation to load for this test
	static const char *		sSceneName;

	// Scene time (for moving bodies)
	float					mTime = 0.0f;

	// Moving bodies
	BodyID					mRotatingBody;
	BodyID					mVerticallyMovingBody;
	BodyID					mHorizontallyMovingBody;
};
