// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/BroadPhase/BroadPhaseTest.h>

// Simple test that casts a ray through the broadphase
class BroadPhaseCastRayTest : public BroadPhaseTest
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(BroadPhaseCastRayTest)

	// Initialize the test
	virtual void	Initialize() override;

	// Update the test, called before the physics update
	virtual void	PrePhysicsUpdate(const PreUpdateParams &inParams) override;
};
