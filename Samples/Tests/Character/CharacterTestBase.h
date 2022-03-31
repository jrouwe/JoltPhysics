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

	// Override to specify the initial camera state (local to GetCameraPivot)
	virtual void			GetInitialCamera(CameraState &ioState) const override;

	// Optional settings menu
	virtual bool			HasSettingsMenu() const override							{ return true; }
	virtual void			CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu) override;

protected:
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
