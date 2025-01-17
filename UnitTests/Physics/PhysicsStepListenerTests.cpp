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
		virtual void			OnStep(const PhysicsStepListenerContext &inContext) override
		{
			CHECK(inContext.mDeltaTime == mExpectedDeltaTime);
			CHECK(inContext.mIsFirstStep == ((mCount % mExpectedSteps) == 0));
			int new_count = mCount.fetch_add(1) + 1;
			CHECK(inContext.mIsLastStep == ((new_count % mExpectedSteps) == 0));
		}

		atomic<int>				mCount = 0;
		int						mExpectedSteps;
		float					mExpectedDeltaTime = 0.0f;
	};

	// Perform the actual listener test with a variable amount of collision steps
	static void DoTest(int inCollisionSteps)
	{
		PhysicsTestContext c(1.0f / 60.0f, inCollisionSteps);

		// Initialize and add listeners
		TestStepListener listeners[10];
		for (TestStepListener &l : listeners)
		{
			l.mExpectedDeltaTime = 1.0f / 60.0f / inCollisionSteps;
			l.mExpectedSteps = inCollisionSteps;
		}
		for (TestStepListener &l : listeners)
			c.GetSystem()->AddStepListener(&l);

		// Step the simulation
		c.SimulateSingleStep();

		// There aren't any active bodies so no listeners should have been called
		for (TestStepListener &l : listeners)
			CHECK(l.mCount == 0);

		// Now add an active body
		c.CreateBox(RVec3::sZero(), Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING, Vec3::sOne());

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

		// Step the simulation
		c.SimulateSingleStep();

		// Check that no further callbacks were triggered
		for (TestStepListener &l : listeners)
			CHECK(l.mCount == 2 * inCollisionSteps);
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

	// Test the step listeners with four collision steps
	TEST_CASE("TestStepListener4")
	{
		DoTest(4);
	}
}
