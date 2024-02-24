// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include "PhysicsTestContext.h"
#include <Jolt/Physics/Constraints/SixDOFConstraint.h>
#include "Layers.h"

TEST_SUITE("SixDOFConstraintTests")
{
	// Test if the 6DOF constraint can be used to create a spring
	TEST_CASE("TestSixDOFSpring")
	{
		// Configuration of the spring
		const float cFrequency = 2.0f;
		const float cDamping = 0.1f;

		// Test all permutations of axis
		for (uint spring_axis = 0b001; spring_axis <= 0b111; ++spring_axis)
		{
			// Test all spring modes
			for (int mode = 0; mode < 2; ++mode)
			{
				const RVec3 cInitialPosition(10.0f * (spring_axis & 1), 8.0f * (spring_axis & 2), 6.0f * (spring_axis & 4));

				// Create a sphere
				PhysicsTestContext context;
				context.ZeroGravity();
				Body& body = context.CreateSphere(cInitialPosition, 0.5f, EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING);
				body.GetMotionProperties()->SetLinearDamping(0.0f);

				// Calculate stiffness and damping of spring
				float m = 1.0f / body.GetMotionProperties()->GetInverseMass();
				float omega = 2.0f * JPH_PI * cFrequency;
				float k = m * Square(omega);
				float c = 2.0f * m * cDamping * omega;

				// Create spring
				SixDOFConstraintSettings constraint;
				constraint.mPosition2 = cInitialPosition;
				for (int axis = 0; axis < 3; ++axis)
				{
					// Check if this axis is supposed to be a spring
					if (((1 << axis) & spring_axis) != 0)
					{
						if (mode == 0)
						{
							// First iteration use stiffness and damping
							constraint.mLimitsSpringSettings[axis].mMode = ESpringMode::StiffnessAndDamping;
							constraint.mLimitsSpringSettings[axis].mStiffness = k;
							constraint.mLimitsSpringSettings[axis].mDamping = c;
						}
						else
						{
							// Second iteration use frequency and damping
							constraint.mLimitsSpringSettings[axis].mMode = ESpringMode::FrequencyAndDamping;
							constraint.mLimitsSpringSettings[axis].mFrequency = cFrequency;
							constraint.mLimitsSpringSettings[axis].mDamping = cDamping;
						}
						constraint.mLimitMin[axis] = constraint.mLimitMax[axis] = 0.0f;
					}
				}
				context.CreateConstraint<SixDOFConstraint>(Body::sFixedToWorld, body, constraint);

				// Simulate spring
				RVec3 x = cInitialPosition;
				Vec3 v = Vec3::sZero();
				float dt = context.GetDeltaTime();
				for (int i = 0; i < 120; ++i)
				{
					// Using the equations from page 32 of Soft Constraints: Reinventing The Spring - Erin Catto - GDC 2011 for an implicit euler spring damper
					for (int axis = 0; axis < 3; ++axis)
						if (((1 << axis) & spring_axis) != 0) // Only update velocity for axis where there is a spring
							v.SetComponent(axis, (v[axis] - dt * k / m * float(x[axis])) / (1.0f + dt * c / m + Square(dt) * k / m));
					x += v * dt;

					// Run physics simulation
					context.SimulateSingleStep();

					// Test if simulation matches prediction
					CHECK_APPROX_EQUAL(x, body.GetPosition(), 1.0e-5_r);
				}
			}
		}
	}
}
