// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>
#include <Jolt/Physics/Character/Character.h>

class CharacterGatheringTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, CharacterGatheringTest)

	virtual					~CharacterGatheringTest() override;

	// Description of the test
	virtual const char *	GetDescription() const override
	{
		return	"Creates a lot of Characters that will start moving towards the origin as a stress test";
	}

	// Initialize the test
	virtual void			Initialize() override;

	// Update the test, called before the physics update
	virtual void			PrePhysicsUpdate(const PreUpdateParams &inParams) override;

	// Update the test, called after the physics update
	virtual void			PostPhysicsUpdate(float inDeltaTime) override;

private:
	static constexpr int	cNumCharactersX = 20;
	static constexpr int	cNumCharactersY = 20;
	static constexpr float	sCharacterSpeed = 6.0f;
	static constexpr float	cCharacterHeightStanding = 1.35f;
	static constexpr float	cCharacterRadiusStanding = 0.3f;
	static constexpr float	cCollisionTolerance = 0.05f;

	Array<Ref<Character>>	mCharacters;
};
