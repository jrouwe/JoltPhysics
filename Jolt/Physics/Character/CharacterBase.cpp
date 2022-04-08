// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Character/CharacterBase.h>
#include <Jolt/Physics/Collision/PhysicsMaterial.h>

JPH_NAMESPACE_BEGIN

CharacterBase::CharacterBase(CharacterBaseSettings *inSettings, PhysicsSystem *inSystem) :
	mSystem(inSystem),
	mShape(inSettings->mShape),
	mGroundMaterial(PhysicsMaterial::sDefault)
{
	// Initialize max slope angle
	SetMaxSlopeAngle(inSettings->mMaxSlopeAngle);
}

JPH_NAMESPACE_END
