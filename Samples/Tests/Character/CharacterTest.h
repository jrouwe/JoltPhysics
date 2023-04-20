// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Character/CharacterBaseTest.h>
#include <Jolt/Physics/Character/Character.h>

// Simple test that test the Character class. Allows the user to move around with the arrow keys and jump with the J button.
class CharacterTest : public CharacterBaseTest, public ContactListener
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, CharacterTest)

	// Destructor
	virtual					~CharacterTest() override;

	// Initialize the test
	virtual void			Initialize() override;

	// Update the test, called before the physics update
	virtual void			PrePhysicsUpdate(const PreUpdateParams &inParams) override;

	// Update the test, called after the physics update
	virtual void			PostPhysicsUpdate(float inDeltaTime) override;

	// Saving / restoring state for replay
	virtual void			SaveState(StateRecorder &inStream) const override;
	virtual void			RestoreState(StateRecorder &inStream) override;

	// If this test implements a contact listener, it should be returned here
	virtual ContactListener *GetContactListener() override	{ return this; }

	// See: ContactListener
	virtual void			OnContactAdded(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings) override;
	virtual void			OnContactPersisted(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings) override;

protected:
	// Get position of the character
	virtual RVec3			GetCharacterPosition() const override				{ return mCharacter->GetPosition(); }

	// Handle user input to the character
	virtual void			HandleInput(Vec3Arg inMovementDirection, bool inJump, bool inSwitchStance, float inDeltaTime) override;

private:
	// The 'player' character
	Ref<Character>			mCharacter;
};
