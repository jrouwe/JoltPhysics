// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include "PhysicsTestContext.h"
#include <Physics/Constraints/SliderConstraint.h>
#include <Physics/Collision/GroupFilterTable.h>
#include "Layers.h"

TEST_SUITE("SliderConstraintTests")
{
	// Test a box attached to a slider constraint, test that the body doesn't move beyond the min limit
	TEST_CASE("TestSliderConstraintLimitMin")
	{
		const Vec3 cInitialPos(3.0f, 0, 0);
		const float cLimitMin = -7.0f;

		// Create group filter
		Ref<GroupFilterTable> group_filter = new GroupFilterTable;

		// Create two boxes
		PhysicsTestContext c;
		Body &body1 = c.CreateBox(Vec3::sZero(), Quat::sIdentity(), EMotionType::Static, EMotionQuality::Discrete, Layers::NON_MOVING, Vec3(1, 1, 1));
		Body &body2 = c.CreateBox(cInitialPos, Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING, Vec3(1, 1, 1));

		// Give body 2 velocity towards min limit (and ensure that it arrives well before 1 second)
		body2.SetLinearVelocity(-Vec3(10.0f, 0, 0));

		// Bodies will go through each other, make sure they don't collide
		body1.SetCollisionGroup(CollisionGroup(group_filter, 0, 0));
		body2.SetCollisionGroup(CollisionGroup(group_filter, 0, 0));

		// Create slider constraint
		SliderConstraintSettings s;
		s.mSliderAxis = Vec3::sAxisX();
		s.mLimitsMin = cLimitMin;
		s.mLimitsMax = 0.0f;
		c.CreateConstraint<SliderConstraint>(body1, body2, s);

		// Simulate
		c.Simulate(1.0f);

		// Test resulting velocity
		CHECK_APPROX_EQUAL(Vec3::sZero(), body2.GetLinearVelocity(), 1.0e-4f);

		// Test resulting position
		CHECK_APPROX_EQUAL(cInitialPos + cLimitMin * s.mSliderAxis, body2.GetPosition(), 1.0e-4f);
	}

	// Test a box attached to a slider constraint, test that the body doesn't move beyond the max limit
	TEST_CASE("TestSliderConstraintLimitMax")
	{
		const Vec3 cInitialPos(3.0f, 0, 0);
		const float cLimitMax = 7.0f;

		// Create two boxes
		PhysicsTestContext c;
		Body &body1 = c.CreateBox(Vec3::sZero(), Quat::sIdentity(), EMotionType::Static, EMotionQuality::Discrete, Layers::NON_MOVING, Vec3(1, 1, 1));
		Body &body2 = c.CreateBox(cInitialPos, Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING, Vec3(1, 1, 1));

		// Give body 2 velocity towards max limit (and ensure that it arrives well before 1 second)
		body2.SetLinearVelocity(Vec3(10.0f, 0, 0));

		// Create slider constraint
		SliderConstraintSettings s;
		s.mSliderAxis = Vec3::sAxisX();
		s.mLimitsMin = 0.0f;
		s.mLimitsMax = cLimitMax;
		c.CreateConstraint<SliderConstraint>(body1, body2, s);

		// Simulate
		c.Simulate(1.0f);

		// Test resulting velocity
		CHECK_APPROX_EQUAL(Vec3::sZero(), body2.GetLinearVelocity(), 1.0e-4f);

		// Test resulting position
		CHECK_APPROX_EQUAL(cInitialPos + cLimitMax * s.mSliderAxis, body2.GetPosition(), 1.0e-4f);
	}

	// Test a box attached to a slider constraint, test that a motor can drive it to a specific velocity
	TEST_CASE("TestSliderConstraintDriveVelocityStaticVsDynamic")
	{
		const Vec3 cInitialPos(3.0f, 0, 0);
		const float cMotorAcceleration = 2.0f;

		// Create two boxes
		PhysicsTestContext c;
		Body &body1 = c.CreateBox(Vec3::sZero(), Quat::sIdentity(), EMotionType::Static, EMotionQuality::Discrete, Layers::NON_MOVING, Vec3(1, 1, 1));
		Body &body2 = c.CreateBox(cInitialPos, Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING, Vec3(1, 1, 1));

		// Create slider constraint
		SliderConstraintSettings s;
		s.mSliderAxis = Vec3::sAxisX();
		constexpr float mass = Cubed(2.0f) * 1000.0f; // Density * Volume
		s.mMotorSettings = MotorSettings(0.0f, 0.0f, mass * cMotorAcceleration, 0.0f);
		SliderConstraint &constraint = c.CreateConstraint<SliderConstraint>(body1, body2, s);
		constraint.SetMotorState(EMotorState::Velocity);
		constraint.SetTargetVelocity(1.5f * cMotorAcceleration);

		// Simulate
		c.Simulate(1.0f);

		// Test resulting velocity
		Vec3 expected_vel = cMotorAcceleration * s.mSliderAxis;
		CHECK_APPROX_EQUAL(expected_vel, body2.GetLinearVelocity(), 1.0e-4f);

		// Simulate (after 0.5 seconds it should reach the target velocity)
		c.Simulate(1.0f);

		// Test resulting velocity
		expected_vel = 1.5f * cMotorAcceleration * s.mSliderAxis;
		CHECK_APPROX_EQUAL(expected_vel, body2.GetLinearVelocity(), 1.0e-4f);

		// Test resulting position (1.5s of acceleration + 0.5s of constant speed)
		Vec3 expected_pos = c.PredictPosition(cInitialPos, Vec3::sZero(), cMotorAcceleration * s.mSliderAxis, 1.5f) + 0.5f * expected_vel;
		CHECK_APPROX_EQUAL(expected_pos, body2.GetPosition(), 1.0e-4f);
	}

	// Test 2 dynamic boxes attached to a slider constraint, test that a motor can drive it to a specific velocity
	TEST_CASE("TestSliderConstraintDriveVelocityDynamicVsDynamic")
	{
		const Vec3 cInitialPos(3.0f, 0, 0);
		const float cMotorAcceleration = 2.0f;

		// Create two boxes
		PhysicsTestContext c;
		c.ZeroGravity();
		Body &body1 = c.CreateBox(Vec3::sZero(), Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING, Vec3(1, 1, 1));
		Body &body2 = c.CreateBox(cInitialPos, Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING, Vec3(1, 1, 1));

		// Create slider constraint
		SliderConstraintSettings s;
		s.mSliderAxis = Vec3::sAxisX();
		constexpr float mass = Cubed(2.0f) * 1000.0f; // Density * Volume
		s.mMotorSettings = MotorSettings(0.0f, 0.0f, mass * cMotorAcceleration, 0.0f);
		SliderConstraint &constraint = c.CreateConstraint<SliderConstraint>(body1, body2, s);
		constraint.SetMotorState(EMotorState::Velocity);
		constraint.SetTargetVelocity(3.0f * cMotorAcceleration);

		// Simulate
		c.Simulate(1.0f);

		// Test resulting velocity (both boxes move in opposite directions with the same force, so the resulting velocity difference is 2x as big as the previous test)
		Vec3 expected_vel = cMotorAcceleration * s.mSliderAxis;
		CHECK_APPROX_EQUAL(-expected_vel, body1.GetLinearVelocity(), 1.0e-4f);
		CHECK_APPROX_EQUAL(expected_vel, body2.GetLinearVelocity(), 1.0e-4f);

		// Simulate (after 0.5 seconds it should reach the target velocity)
		c.Simulate(1.0f);

		// Test resulting velocity
		expected_vel = 1.5f * cMotorAcceleration * s.mSliderAxis;
		CHECK_APPROX_EQUAL(-expected_vel, body1.GetLinearVelocity(), 1.0e-4f);
		CHECK_APPROX_EQUAL(expected_vel, body2.GetLinearVelocity(), 1.0e-4f);

		// Test resulting position (1.5s of acceleration + 0.5s of constant speed)
		Vec3 expected_pos1 = c.PredictPosition(Vec3::sZero(), Vec3::sZero(), -cMotorAcceleration * s.mSliderAxis, 1.5f) - 0.5f * expected_vel;
		Vec3 expected_pos2 = c.PredictPosition(cInitialPos, Vec3::sZero(), cMotorAcceleration * s.mSliderAxis, 1.5f) + 0.5f * expected_vel;
		CHECK_APPROX_EQUAL(expected_pos1, body1.GetPosition(), 1.0e-4f);
		CHECK_APPROX_EQUAL(expected_pos2, body2.GetPosition(), 1.0e-4f);
	}

	// Test a box attached to a slider constraint, test that a motor can drive it to a specific position
	TEST_CASE("TestSliderConstraintDrivePosition")
	{
		const Vec3 cInitialPos(3.0f, 0, 0);
		const Vec3 cMotorPos(10.0f, 0, 0);

		// Create two boxes
		PhysicsTestContext c;
		Body &body1 = c.CreateBox(Vec3::sZero(), Quat::sIdentity(), EMotionType::Static, EMotionQuality::Discrete, Layers::NON_MOVING, Vec3(1, 1, 1));
		Body &body2 = c.CreateBox(cInitialPos, Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING, Vec3(1, 1, 1));

		// Create slider constraint
		SliderConstraintSettings s;
		s.mSliderAxis = Vec3::sAxisX();
		SliderConstraint &constraint = c.CreateConstraint<SliderConstraint>(body1, body2, s);
		constraint.SetMotorState(EMotorState::Position);
		constraint.SetTargetPosition((cMotorPos - cInitialPos).Dot(s.mSliderAxis));

		// Simulate
		c.Simulate(2.0f);

		// Test resulting velocity
		CHECK_APPROX_EQUAL(Vec3::sZero(), body2.GetLinearVelocity(), 1.0e-4f);

		// Test resulting position
		CHECK_APPROX_EQUAL(cMotorPos, body2.GetPosition(), 1.0e-4f);
	}

	// Test a box attached to a slider constraint, give it initial velocity and test that the friction provides the correct decelleration
	TEST_CASE("TestSliderConstraintFriction")
	{
		const Vec3 cInitialPos(3.0f, 0, 0);
		const Vec3 cInitialVelocity(10.0f, 0, 0);
		const float cFrictionAcceleration = 2.0f;
		const float cSimulationTime = 2.0f;

		// Create two boxes
		PhysicsTestContext c;
		Body &body1 = c.CreateBox(Vec3::sZero(), Quat::sIdentity(), EMotionType::Static, EMotionQuality::Discrete, Layers::NON_MOVING, Vec3(1, 1, 1));
		Body &body2 = c.CreateBox(cInitialPos, Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING, Vec3(1, 1, 1));
		body2.SetLinearVelocity(cInitialVelocity);

		// Create slider constraint
		SliderConstraintSettings s;
		s.mSliderAxis = Vec3::sAxisX();
		constexpr float mass = Cubed(2.0f) * 1000.0f; // Density * Volume
		s.mMaxFrictionForce = mass * cFrictionAcceleration;
		c.CreateConstraint<SliderConstraint>(body1, body2, s);

		// Simulate while applying friction
		c.Simulate(cSimulationTime);

		// Test resulting velocity
		Vec3 expected_vel = cInitialVelocity - cFrictionAcceleration * cSimulationTime * s.mSliderAxis;
		CHECK_APPROX_EQUAL(expected_vel, body2.GetLinearVelocity(), 1.0e-4f);

		// Test resulting position
		Vec3 expected_pos = c.PredictPosition(cInitialPos, cInitialVelocity, -cFrictionAcceleration * s.mSliderAxis, cSimulationTime);
		CHECK_APPROX_EQUAL(expected_pos, body2.GetPosition(), 1.0e-4f);
	}

	// Test if a slider constraint wakes up connected bodies
	TEST_CASE("TestSliderStaticVsKinematic")
	{
		// Create two boxes far away enough so they are not touching
		PhysicsTestContext c;
		Body &body1 = c.CreateBox(Vec3::sZero(), Quat::sIdentity(), EMotionType::Static, EMotionQuality::Discrete, Layers::NON_MOVING, Vec3(1, 1, 1), EActivation::DontActivate);
		Body &body2 = c.CreateBox(Vec3(10, 0, 0), Quat::sIdentity(), EMotionType::Kinematic, EMotionQuality::Discrete, Layers::MOVING, Vec3(1, 1, 1), EActivation::DontActivate);

		// Create slider constraint
		SliderConstraintSettings s;
		s.mSliderAxis = Vec3::sAxisX();
		c.CreateConstraint<SliderConstraint>(body1, body2, s);

		// Verify they're not active
		CHECK(!body1.IsActive());
		CHECK(!body2.IsActive());

		// After a physics step, the bodies should still not be active
		c.SimulateSingleStep();
		CHECK(!body1.IsActive());
		CHECK(!body2.IsActive());

		// Activate the kinematic body
		c.GetSystem()->GetBodyInterface().ActivateBody(body2.GetID());
		CHECK(!body1.IsActive());
		CHECK(body2.IsActive());

		// The static body should not become active (it can't)
		c.SimulateSingleStep();
		CHECK(!body1.IsActive());
		CHECK(body2.IsActive());
	}

	// Test if a slider constraint wakes up connected bodies
	TEST_CASE("TestSliderStaticVsDynamic")
	{
		// Create two boxes far away enough so they are not touching
		PhysicsTestContext c;
		Body &body1 = c.CreateBox(Vec3::sZero(), Quat::sIdentity(), EMotionType::Static, EMotionQuality::Discrete, Layers::NON_MOVING, Vec3(1, 1, 1), EActivation::DontActivate);
		Body &body2 = c.CreateBox(Vec3(10, 0, 0), Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING, Vec3(1, 1, 1), EActivation::DontActivate);

		// Create slider constraint
		SliderConstraintSettings s;
		s.mSliderAxis = Vec3::sAxisX();
		c.CreateConstraint<SliderConstraint>(body1, body2, s);

		// Verify they're not active
		CHECK(!body1.IsActive());
		CHECK(!body2.IsActive());

		// After a physics step, the bodies should still not be active
		c.SimulateSingleStep();
		CHECK(!body1.IsActive());
		CHECK(!body2.IsActive());

		// Activate the dynamic body
		c.GetSystem()->GetBodyInterface().ActivateBody(body2.GetID());
		CHECK(!body1.IsActive());
		CHECK(body2.IsActive());

		// The static body should not become active (it can't)
		c.SimulateSingleStep();
		CHECK(!body1.IsActive());
		CHECK(body2.IsActive());
	}

	// Test if a slider constraint wakes up connected bodies
	TEST_CASE("TestSliderKinematicVsDynamic")
	{
		// Create two boxes far away enough so they are not touching
		PhysicsTestContext c;
		Body &body1 = c.CreateBox(Vec3::sZero(), Quat::sIdentity(), EMotionType::Kinematic, EMotionQuality::Discrete, Layers::MOVING, Vec3(1, 1, 1), EActivation::DontActivate);
		Body &body2 = c.CreateBox(Vec3(10, 0, 0), Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING, Vec3(1, 1, 1), EActivation::DontActivate);

		// Create slider constraint
		SliderConstraintSettings s;
		s.mSliderAxis = Vec3::sAxisX();
		c.CreateConstraint<SliderConstraint>(body1, body2, s);

		// Verify they're not active
		CHECK(!body1.IsActive());
		CHECK(!body2.IsActive());

		// After a physics step, the bodies should still not be active
		c.SimulateSingleStep();
		CHECK(!body1.IsActive());
		CHECK(!body2.IsActive());

		// Activate the keyframed body
		c.GetSystem()->GetBodyInterface().ActivateBody(body1.GetID());
		CHECK(body1.IsActive());
		CHECK(!body2.IsActive());

		// After a physics step, both bodies should be active now
		c.SimulateSingleStep();
		CHECK(body1.IsActive());
		CHECK(body2.IsActive());
	}

	// Test if a slider constraint wakes up connected bodies
	TEST_CASE("TestSliderKinematicVsKinematic")
	{
		// Create two boxes far away enough so they are not touching
		PhysicsTestContext c;
		Body &body1 = c.CreateBox(Vec3::sZero(), Quat::sIdentity(), EMotionType::Kinematic, EMotionQuality::Discrete, Layers::MOVING, Vec3(1, 1, 1), EActivation::DontActivate);
		Body &body2 = c.CreateBox(Vec3(10, 0, 0), Quat::sIdentity(), EMotionType::Kinematic, EMotionQuality::Discrete, Layers::MOVING, Vec3(1, 1, 1), EActivation::DontActivate);

		// Create slider constraint
		SliderConstraintSettings s;
		s.mSliderAxis = Vec3::sAxisX();
		c.CreateConstraint<SliderConstraint>(body1, body2, s);

		// Verify they're not active
		CHECK(!body1.IsActive());
		CHECK(!body2.IsActive());

		// After a physics step, the bodies should still not be active
		c.SimulateSingleStep();
		CHECK(!body1.IsActive());
		CHECK(!body2.IsActive());

		// Activate the first keyframed body
		c.GetSystem()->GetBodyInterface().ActivateBody(body1.GetID());
		CHECK(body1.IsActive());
		CHECK(!body2.IsActive());

		// After a physics step, the second keyframed body should not be woken up
		c.SimulateSingleStep();
		CHECK(body1.IsActive());
		CHECK(!body2.IsActive());
	}

	// Test if a slider constraint wakes up connected bodies
	TEST_CASE("TestSliderDynamicVsDynamic")
	{
		// Create two boxes far away enough so they are not touching
		PhysicsTestContext c;
		Body &body1 = c.CreateBox(Vec3::sZero(), Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING, Vec3(1, 1, 1), EActivation::DontActivate);
		Body &body2 = c.CreateBox(Vec3(10, 0, 0), Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING, Vec3(1, 1, 1), EActivation::DontActivate);

		// Create slider constraint
		SliderConstraintSettings s;
		s.mSliderAxis = Vec3::sAxisX();
		c.CreateConstraint<SliderConstraint>(body1, body2, s);

		// Verify they're not active
		CHECK(!body1.IsActive());
		CHECK(!body2.IsActive());

		// After a physics step, the bodies should still not be active
		c.SimulateSingleStep();
		CHECK(!body1.IsActive());
		CHECK(!body2.IsActive());

		// Activate the first dynamic body
		c.GetSystem()->GetBodyInterface().ActivateBody(body1.GetID());
		CHECK(body1.IsActive());
		CHECK(!body2.IsActive());

		// After a physics step, both bodies should be active now
		c.SimulateSingleStep();
		CHECK(body1.IsActive());
		CHECK(body2.IsActive());
	}
}