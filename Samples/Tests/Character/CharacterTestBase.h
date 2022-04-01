// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

// Base class for the character tests, initializes the test scene.
class CharacterTestBase : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(CharacterTestBase)

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

protected:
	// Get position of the character
	virtual Vec3			GetCharacterPosition() const = 0;

	// Handle user input to the character
	virtual void			HandleInput(Vec3Arg inMovementDirection, bool inJump, bool inSwitchStance, float inDeltaTime) = 0;

	// Character size
	inline static constexpr float cCharacterHeightStanding = 1.35f;
	inline static constexpr float cCharacterRadiusStanding = 0.3f;
	inline static constexpr float cCharacterHeightCrouching = 0.8f;
	inline static constexpr float cCharacterRadiusCrouching = 0.3f;
	inline static constexpr float cCharacterSpeed = 6.0f;
	inline static constexpr float cJumpSpeed = 4.0f;

	// The different stances for the character
	RefConst<Shape>			mStandingShape;
	RefConst<Shape>			mCrouchingShape;

private:
	// List of possible scene names
	static const char *		sScenes[];

	// Filename of animation to load for this test
	static const char *		sSceneName;
};
