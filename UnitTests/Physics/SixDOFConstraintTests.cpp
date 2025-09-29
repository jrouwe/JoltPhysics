// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include "PhysicsTestContext.h"
#include <Jolt/Physics/Constraints/SixDOFConstraint.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
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

	// Test combination of locked rotation axis with a 6DOF constraint
	TEST_CASE("TestSixDOFLockedRotation")
	{
		PhysicsTestContext context;
		BodyInterface &bi = context.GetBodyInterface();
		PhysicsSystem *system = context.GetSystem();

		RefConst<Shape> box_shape = new BoxShape(Vec3::sReplicate(1.0f));

		// Static 'anchor' body
		BodyCreationSettings settings1(box_shape, RVec3::sZero(), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);
		Body &body1 = *bi.CreateBody(settings1);
		bi.AddBody(body1.GetID(), EActivation::Activate);

		// Dynamic body that cannot rotate around X and Y
		const RVec3 position2(3, 0, 0);
		const Quat rotation2 = Quat::sIdentity();
		BodyCreationSettings settings2(box_shape, position2, rotation2, EMotionType::Dynamic, Layers::MOVING);
		settings2.mAllowedDOFs = EAllowedDOFs::RotationZ | EAllowedDOFs::TranslationX | EAllowedDOFs::TranslationY | EAllowedDOFs::TranslationZ;
		Body &body2 = *bi.CreateBody(settings2);
		bi.AddBody(body2.GetID(), EActivation::Activate);

		// Lock all 6 axis with a 6DOF constraint
		SixDOFConstraintSettings six_dof;
		six_dof.MakeFixedAxis(SixDOFConstraintSettings::EAxis::TranslationX);
		six_dof.MakeFixedAxis(SixDOFConstraintSettings::EAxis::TranslationY);
		six_dof.MakeFixedAxis(SixDOFConstraintSettings::EAxis::TranslationZ);
		six_dof.MakeFixedAxis(SixDOFConstraintSettings::EAxis::RotationX);
		six_dof.MakeFixedAxis(SixDOFConstraintSettings::EAxis::RotationY);
		six_dof.MakeFixedAxis(SixDOFConstraintSettings::EAxis::RotationZ);
		system->AddConstraint(six_dof.Create(body1, body2));

		context.Simulate(1.0f);

		// Check that body didn't rotate
		CHECK_APPROX_EQUAL(body2.GetPosition(), position2, 5.0e-3f);
		CHECK_APPROX_EQUAL(body2.GetRotation(), rotation2, 5.0e-3f);
	}
}
