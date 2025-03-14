// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/BroadPhase/BroadPhaseTest.h>

class BroadPhaseCastRayTest : public BroadPhaseTest
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, BroadPhaseCastRayTest)

	// Description of the test
	virtual const char *GetDescription() const override
	{
		return "Simple test that casts a ray through the broadphase.";
	}

	// Initialize the test
	virtual void	Initialize() override;

	// Update the test, called before the physics update
	virtual void	PrePhysicsUpdate(const PreUpdateParams &inParams) override;
};
