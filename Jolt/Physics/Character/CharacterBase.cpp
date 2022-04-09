// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Character/CharacterBase.h>

JPH_NAMESPACE_BEGIN

CharacterBase::CharacterBase(const CharacterBaseSettings *inSettings, PhysicsSystem *inSystem) :
	mSystem(inSystem),
	mShape(inSettings->mShape)
{
	// Initialize max slope angle
	SetMaxSlopeAngle(inSettings->mMaxSlopeAngle);
}

JPH_NAMESPACE_END
