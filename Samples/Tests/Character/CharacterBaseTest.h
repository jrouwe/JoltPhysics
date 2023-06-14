// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>
#include <Jolt/Physics/Character/CharacterBase.h>

// Base class for the character tests, initializes the test scene.
class CharacterBaseTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, CharacterBaseTest)

	// Number used to scale the terrain and camera movement to the scene
	virtual float			GetWorldScale() const override								{ return 0.2f; }

	// Initialize the test
	virtual void			Initialize() override;

	// Update the test, called before the physics update
	virtual void			PrePhysicsUpdate(const PreUpdateParams &inParams) override;

	// Override to specify the initial camera state (local to GetCameraPivot)
	virtual void			GetInitialCamera(CameraState &ioState) const override;

	// Override to specify a camera pivot point and orientation (world space)
	virtual RMat44			GetCameraPivot(float inCameraHeading, float inCameraPitch) const override;

	// Optional settings menu
	virtual bool			HasSettingsMenu() const override							{ return true; }
	virtual void			CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu) override;

	// Saving / restoring state for replay
	virtual void			SaveState(StateRecorder &inStream) const override;
	virtual void			RestoreState(StateRecorder &inStream) override;

protected:
	// Get position of the character
	virtual RVec3			GetCharacterPosition() const = 0;

	// Handle user input to the character
	virtual void			HandleInput(Vec3Arg inMovementDirection, bool inJump, bool inSwitchStance, float inDeltaTime) = 0;

	// Draw the character state
	void					DrawCharacterState(const CharacterBase *inCharacter, RMat44Arg inCharacterTransform, Vec3Arg inCharacterVelocity);

	// Add character movement settings
	virtual void			AddCharacterMovementSettings(DebugUI* inUI, UIElement* inSubMenu) { /* Nothing by default */ }

	// Add test configuration settings
	virtual void			AddConfigurationSettings(DebugUI *inUI, UIElement *inSubMenu) { /* Nothing by default */ }

	// Character size
	static constexpr float	cCharacterHeightStanding = 1.35f;
	static constexpr float	cCharacterRadiusStanding = 0.3f;
	static constexpr float	cCharacterHeightCrouching = 0.8f;
	static constexpr float	cCharacterRadiusCrouching = 0.3f;

	// Character movement properties
	inline static bool		sControlMovementDuringJump = true;					///< If false the character cannot change movement direction in mid air
	inline static float		sCharacterSpeed = 6.0f;
	inline static float		sJumpSpeed = 4.0f;

	// The different stances for the character
	RefConst<Shape>			mStandingShape;
	RefConst<Shape>			mCrouchingShape;

	// List of boxes on ramp
	Array<BodyID>			mRampBlocks;
	float					mRampBlocksTimeLeft = 0.0f;

	// Conveyor belt body
	BodyID					mConveyorBeltBody;

	// Sensor body
	BodyID					mSensorBody;

private:
	// Shape types
	enum class EType
	{
		Capsule,
		Cylinder,
		Box
	};

	// Character shape type
	static inline EType		sShapeType = EType::Capsule;

	// List of possible scene names
	static const char *		sScenes[];

	// Filename of animation to load for this test
	static const char *		sSceneName;

	// Scene time (for moving bodies)
	float					mTime = 0.0f;

	// Moving bodies
	BodyID					mRotatingBody;
	BodyID					mRotatingWallBody;
	BodyID					mRotatingAndTranslatingBody;
	BodyID					mSmoothVerticallyMovingBody;
	BodyID					mReversingVerticallyMovingBody;
	float					mReversingVerticallyMovingVelocity = 1.0f;
	BodyID					mHorizontallyMovingBody;
};
