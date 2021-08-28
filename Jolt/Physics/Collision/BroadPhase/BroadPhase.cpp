// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt.h>

#include <Physics/Collision/BroadPhase/BroadPhase.h>

namespace JPH {

void BroadPhase::Init(BodyManager *inBodyManager, const ObjectToBroadPhaseLayer &inObjectToBroadPhaseLayer)
{
	mBodyManager = inBodyManager;
}

} // JPH