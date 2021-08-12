// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt.h>

#include <Physics/PhysicsUpdateContext.h>

namespace JPH {

PhysicsUpdateContext::~PhysicsUpdateContext()
{
	JPH_ASSERT(mBodyPairs == nullptr);
	JPH_ASSERT(mActiveConstraints == nullptr);
}

} // JPH