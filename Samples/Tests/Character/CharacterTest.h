// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>
#include <Jolt/Physics/Character/Character.h>

// Simple test that test the Character class. Allows the user to move around with the arrow keys and jump with the J button.
class CharacterTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(CharacterTest)

	// Destructor
	virtual					~CharacterTest() override;

	// Number used to scale the terrain and camera movement to the scene
	virtual float			GetWorldScale() const override								{ return 0.2f; }

	// Initialize the test
	virtual void			Initialize() override;

	// Update the test, called before the physics update
	virtual void			PrePhysicsUpdate(const PreUpdateParams &inParams) override;

	// Update the test, called after the physics update
	virtual void			PostPhysicsUpdate(float inDeltaTime) override;

	// Override to specify the initial camera state (local to GetCameraPivot)
	virtual void			GetInitialCamera(CameraState &ioState) const override;

	// Override to specify a camera pivot point and orientation (world space)
	virtual Mat44			GetCameraPivot(float inCameraHeading, float inCameraPitch) const override;

	// Optional settings menu
	virtual bool			HasSettingsMenu() const override							{ return true; }
	virtual void			CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu) override;

private:
	// List of possible scene names
	static const char *		sScenes[];

	// Filename of animation to load for this test
	static const char *		sSceneName;

	// The 'player' character
	Ref<Character>			mCharacter;

	// The different stances for the character
	RefConst<Shape>			mStandingShape;
	RefConst<Shape>			mCrouchingShape;
};