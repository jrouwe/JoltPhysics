// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Character/CharacterTestBase.h>
#include <Jolt/Physics/Character/CharacterVirtual.h>

// Simple test that test the CharacterVirtual class. Allows the user to move around with the arrow keys and jump with the J button.
class CharacterVirtualTest : public CharacterTestBase
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(CharacterVirtualTest)

	// Initialize the test
	virtual void			Initialize() override;

	// Update the test, called before the physics update
	virtual void			PrePhysicsUpdate(const PreUpdateParams &inParams) override;

	// Override to specify a camera pivot point and orientation (world space)
	virtual Mat44			GetCameraPivot(float inCameraHeading, float inCameraPitch) const override;

private:
	// The 'player' character
	Ref<CharacterVirtual>	mCharacter;
};
