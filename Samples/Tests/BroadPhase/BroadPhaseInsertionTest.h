// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/BroadPhase/BroadPhaseTest.h>
#include <random>

// Test that adds/removes objects to/from the broadphase and casts a ray through the boxes to see if the collision results are correct
class BroadPhaseInsertionTest : public BroadPhaseTest
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, BroadPhaseInsertionTest)

	// Initialize the test
	virtual void			Initialize() override;

	// Update the test, called before the physics update
	virtual void			PrePhysicsUpdate(const PreUpdateParams &inParams) override;

private:
	default_random_engine	mRandomGenerator;
	size_t					mCurrentBody = 0;
	int						mDirection = 1;
};
