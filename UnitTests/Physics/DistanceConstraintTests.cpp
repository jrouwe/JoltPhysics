// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include "PhysicsTestContext.h"
#include <Jolt/Physics/Constraints/DistanceConstraint.h>
#include "Layers.h"

TEST_SUITE("DistanceConstraintTests")
{
	// Test if the distance constraint can be used to create a spring
	TEST_CASE("TestDistanceSpring")
	{
		// Configuration of the spring
		const RVec3 cInitialPosition(10, 0, 0);
		const float cFrequency = 2.0f;
		const float cDamping = 0.1f;

		for (int mode = 0; mode < 2; ++mode)
		{
			// Create a sphere
			PhysicsTestContext context;
			context.ZeroGravity();
			Body &body = context.CreateSphere(cInitialPosition, 0.5f, EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING);
			body.GetMotionProperties()->SetLinearDamping(0.0f);

			// Calculate stiffness and damping of spring
			float m = 1.0f / body.GetMotionProperties()->GetInverseMass();
			float omega = 2.0f * JPH_PI * cFrequency;
			float k = m * Square(omega);
			float c = 2.0f * m * cDamping * omega;

			// Create spring
			DistanceConstraintSettings constraint;
			constraint.mPoint2 = cInitialPosition;
			if (mode == 0)
			{
				// First iteration use stiffness and damping
				constraint.mLimitsSpringSettings.mMode = ESpringMode::StiffnessAndDamping;
				constraint.mLimitsSpringSettings.mStiffness = k;
				constraint.mLimitsSpringSettings.mDamping = c;
			}
			else
			{
				// Second iteration use frequency and damping
				constraint.mLimitsSpringSettings.mMode = ESpringMode::FrequencyAndDamping;
				constraint.mLimitsSpringSettings.mFrequency = cFrequency;
				constraint.mLimitsSpringSettings.mDamping = cDamping;
			}
			constraint.mMinDistance = constraint.mMaxDistance = 0.0f;
			context.CreateConstraint<DistanceConstraint>(Body::sFixedToWorld, body, constraint);

			// Simulate spring
			Real x = cInitialPosition.GetX();
			float v = 0.0f;
			float dt = context.GetDeltaTime();
			for (int i = 0; i < 120; ++i)
			{
				// Using the equations from page 32 of Soft Constraints: Reinventing The Spring - Erin Catto - GDC 2011 for an implicit euler spring damper
				v = (v - dt * k / m * float(x)) / (1.0f + dt * c / m + Square(dt) * k / m);
				x += v * dt;

				// Run physics simulation
				context.SimulateSingleStep();

				// Test if simulation matches prediction
				CHECK_APPROX_EQUAL(x, body.GetPosition().GetX(), 5.0e-6_r);
				CHECK(body.GetPosition().GetY() == 0);
				CHECK(body.GetPosition().GetZ() == 0);
			}
		}
	}
}
