// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include "PhysicsTestContext.h"
#include <Jolt/Physics/Constraints/HingeConstraint.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include "Layers.h"

TEST_SUITE("HingeConstraintTests")
{
	// Test if the hinge constraint can be used to create a spring
	TEST_CASE("TestHingeSpring")
	{
		// Configuration of the spring
		const float cInitialAngle = DegreesToRadians(100.0f);
		const float cFrequency = 2.0f;
		const float cDamping = 0.1f;

		for (int mode = 0; mode < 2; ++mode)
		{
			// Create a sphere
			PhysicsTestContext context;
			Body &body = context.CreateBody(new SphereShapeSettings(0.5f), RVec3::sZero(), Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING, EActivation::Activate);
			body.GetMotionProperties()->SetAngularDamping(0.0f);
			body.SetAllowSleeping(false);

			// Calculate stiffness and damping of spring
			float inertia = body.GetMotionProperties()->GetInverseInertiaForRotation(Mat44::sIdentity()).Inversed3x3().GetAxisY().Length();
			float omega = 2.0f * JPH_PI * cFrequency;
			float k = inertia * Square(omega);
			float c = 2.0f * inertia * cDamping * omega;

			// Create spring
			HingeConstraintSettings constraint;
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
			constraint.mLimitsMin = constraint.mLimitsMax = 0.0f;
			context.CreateConstraint<HingeConstraint>(Body::sFixedToWorld, body, constraint);

			// Rotate the body to the initial angle
			context.GetBodyInterface().SetRotation(body.GetID(), Quat::sRotation(Vec3::sAxisY(), cInitialAngle), EActivation::Activate);

			// Simulate angular spring
			float angle = cInitialAngle;
			float angular_v = 0.0f;
			float dt = context.GetDeltaTime();
			for (int i = 0; i < 120; ++i)
			{
				// Using the equations from page 32 of Soft Constraints: Reinventing The Spring - Erin Catto - GDC 2011 for an implicit euler spring damper
				angular_v = (angular_v - dt * k / inertia * angle) / (1.0f + dt * c / inertia + Square(dt) * k / inertia);
				angle += angular_v * dt;

				// Run physics simulation
				context.SimulateSingleStep();

				// Decompose body rotation
				Vec3 actual_axis;
				float actual_angle;
				body.GetRotation().GetAxisAngle(actual_axis, actual_angle);
				if (actual_axis.GetY() < 0.0f)
					actual_angle = -actual_angle;

				// Test if simulation matches prediction
				CHECK_APPROX_EQUAL(angle, actual_angle, DegreesToRadians(0.1f));
				CHECK_APPROX_EQUAL(actual_axis.GetX(), 0);
				CHECK_APPROX_EQUAL(actual_axis.GetZ(), 0);
			}
		}
	}
}
