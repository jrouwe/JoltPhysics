// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>
#include <Jolt/Physics/Body/BodyID.h>

// This test will demonstrates how to use layers to disable collisions between other objects and how to change them on the fly.
// The bodies will switch between the MOVING layer and the DEBRIS layer (debris only collides with static).
class ChangeObjectLayerTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(ChangeObjectLayerTest)

	// Initialize the test
	virtual void			Initialize() override;

	// Update the test, called before the physics update
	virtual void			PrePhysicsUpdate(const PreUpdateParams &inParams) override;

	// Saving / restoring state for replay
	virtual void			SaveState(StateRecorder &inStream) const override;
	virtual void			RestoreState(StateRecorder &inStream) override;

private:
	BodyID					mMoving;
	BodyIDVector			mDebris;
	bool					mIsDebris = true;
	float					mTime = 0.0f;
};
