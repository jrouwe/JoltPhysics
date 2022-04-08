// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Character/CharacterTestBase.h>
#include <Jolt/Physics/Character/CharacterVirtual.h>

// Simple test that test the CharacterVirtual class. Allows the user to move around with the arrow keys and jump with the J button.
class CharacterVirtualTest : public CharacterTestBase, public CharacterContactListener
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(CharacterVirtualTest)

	// Initialize the test
	virtual void			Initialize() override;

	// Update the test, called before the physics update
	virtual void			PrePhysicsUpdate(const PreUpdateParams &inParams) override;

	// Called whenever the character collides with a body. Returns true if the contact can push the character.
	virtual void			OnContactAdded(const CharacterVirtual *inCharacter, const BodyID &inBodyID2, const SubShapeID &inSubShapeID2, Vec3Arg inContactPosition, Vec3Arg inContactNormal, CharacterContactSettings &ioSettings) override;

protected:
	// Get position of the character
	virtual Vec3			GetCharacterPosition() const override				{ return mCharacter->GetPosition(); }

	// Handle user input to the character
	virtual void			HandleInput(Vec3Arg inMovementDirection, bool inJump, bool inSwitchStance, float inDeltaTime) override;

private:
	// The 'player' character
	Ref<CharacterVirtual>	mCharacter;

	// Smoothed value of the player input
	Vec3					mSmoothMovementDirection = Vec3::sZero();
};
