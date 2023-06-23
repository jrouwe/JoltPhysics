// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include "PhysicsTestContext.h"
#include "Layers.h"
#include <Jolt/Physics/PhysicsStepListener.h>

TEST_SUITE("StepListenerTest")
{
	// Custom step listener that keeps track how often it has been called
	class TestStepListener : public PhysicsStepListener
	{
	public:
		virtual void			OnStep(float inDeltaTime, PhysicsSystem &inPhysicsSystem) override
		{
			CHECK(inDeltaTime == mExpectedDeltaTime);

			++mCount;
		}

		atomic<int>				mCount = 0;
		float					mExpectedDeltaTime = 0.0f;
	};

	// Perform the actual listener test with a variable amount of collision steps
	static void DoTest(int inCollisionSteps)
	{
		PhysicsTestContext c(1.0f / 60.0f, inCollisionSteps);

		// Initialize and add listeners
		TestStepListener listeners[10];
		for (TestStepListener &l : listeners)
			l.mExpectedDeltaTime = 1.0f / 60.0f / inCollisionSteps;
		for (TestStepListener &l : listeners)
			c.GetSystem()->AddStepListener(&l);

		// Step the simulation
		c.SimulateSingleStep();

		// There aren't any active bodies so no listeners should have been called
		for (TestStepListener &l : listeners)
			CHECK(l.mCount == 0);

		// Now add an active body
		c.CreateBox(RVec3::sZero(), Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING, Vec3::sReplicate(1.0f));

		// Step the simulation
		c.SimulateSingleStep();

		for (TestStepListener &l : listeners)
			CHECK(l.mCount == inCollisionSteps);

		// Step the simulation
		c.SimulateSingleStep();

		for (TestStepListener &l : listeners)
			CHECK(l.mCount == 2 * inCollisionSteps);

		// Unregister all listeners
		for (TestStepListener &l : listeners)
			c.GetSystem()->RemoveStepListener(&l);
	}

	// Test the step listeners with a single collision step
	TEST_CASE("TestStepListener1")
	{
		DoTest(1);
	}

	// Test the step listeners with two collision steps
	TEST_CASE("TestStepListener2")
	{
		DoTest(2);
	}
}
