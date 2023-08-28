// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>
#include <Jolt/Physics/Character/CharacterVirtual.h>

// A test that demonstrates how a character may walk around a fast moving/accelerating sci-fi space ship that is equipped with inertial dampeners.
// Note that this is 'game physics' and not real physics, inertial dampeners only exist in the movies.
// You can walk off the ship and remain attached to the ship. A proper implementation would detect this and detach the character.
class CharacterSpaceShipTest : public Test, public CharacterContactListener
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, CharacterSpaceShipTest)

	// Initialize the test
	virtual void			Initialize() override;

	// Update the test, called before the physics update
	virtual void			PrePhysicsUpdate(const PreUpdateParams &inParams) override;

	// Override to specify the initial camera state (local to GetCameraPivot)
	virtual void			GetInitialCamera(CameraState &ioState) const override;

	// Override to specify a camera pivot point and orientation (world space)
	virtual RMat44			GetCameraPivot(float inCameraHeading, float inCameraPitch) const override;

	// Saving / restoring state for replay
	virtual void			SaveState(StateRecorder &inStream) const override;
	virtual void			RestoreState(StateRecorder &inStream) override;

private:
	// Calculate new ship velocity
	void					UpdateShipVelocity();

	/// Callback to adjust the velocity of a body as seen by the character. Can be adjusted to e.g. implement a conveyor belt or an inertial dampener system of a sci-fi space ship.
	virtual void			OnAdjustBodyVelocity(const CharacterVirtual *inCharacter, const Body &inBody2, Vec3 &ioLinearVelocity, Vec3 &ioAngularVelocity) override;

	// Character size
	static constexpr float	cCharacterHeightStanding = 1.35f;
	static constexpr float	cCharacterRadiusStanding = 0.3f;
	static constexpr float	cCharacterSpeed = 6.0f;
	static constexpr float	cJumpSpeed = 4.0f;

	// The 'player' character
	Ref<CharacterVirtual>	mCharacter;

	// The space ship
	BodyID					mSpaceShip;

	// Previous frame space ship transform
	RMat44					mSpaceShipPrevTransform;

	// Space ship velocity
	Vec3					mSpaceShipLinearVelocity;
	Vec3					mSpaceShipAngularVelocity;

	// Global time
	float					mTime = 0.0f;

	// Smoothed value of the player input
	Vec3					mDesiredVelocity = Vec3::sZero();
};
