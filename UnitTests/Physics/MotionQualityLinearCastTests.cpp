// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include "PhysicsTestContext.h"
#include "Layers.h"
#include "LoggingContactListener.h"
#include "LoggingBodyActivationListener.h"

TEST_SUITE("MotionQualityLinearCastTests")
{
	static const float cBoxExtent = 0.5f;
	static const float cFrequency = 60.0f;
	static const Vec3 cVelocity(2.0f * cFrequency, 0, 0); // High enough velocity to step 2 meters in a single simulation step
	static const RVec3 cPos1(-1, 0, 0);
	static const RVec3 cPos2(1, 0, 0);

	// Two boxes colliding in the center, each has enough velocity to tunnel though in 1 step
	TEST_CASE("TestDiscreteBoxVsDiscreteBox")
	{
		PhysicsTestContext c(1.0f / cFrequency, 1);
		c.ZeroGravity();

		// Register listener
		LoggingContactListener listener;
		c.GetSystem()->SetContactListener(&listener);

		Body &box1 = c.CreateBox(cPos1, Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING, Vec3::sReplicate(cBoxExtent));
		box1.SetLinearVelocity(cVelocity);

		// Test that the inner radius of the box makes sense (used internally by linear cast)
		CHECK_APPROX_EQUAL(box1.GetShape()->GetInnerRadius(), cBoxExtent);

		Body &box2 = c.CreateBox(cPos2, Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING, Vec3::sReplicate(cBoxExtent));
		box2.SetLinearVelocity(-cVelocity);

		c.SimulateSingleStep();

		// No collisions should be reported and the bodies should have moved according to their velocity (tunneling through eachother)
		CHECK(listener.GetEntryCount() == 0);
		CHECK_APPROX_EQUAL(box1.GetPosition(), cPos1 + cVelocity / cFrequency);
		CHECK_APPROX_EQUAL(box1.GetLinearVelocity(), cVelocity);
		CHECK_APPROX_EQUAL(box1.GetAngularVelocity(), Vec3::sZero());
		CHECK_APPROX_EQUAL(box2.GetPosition(), cPos2 - cVelocity / cFrequency);
		CHECK_APPROX_EQUAL(box2.GetLinearVelocity(), -cVelocity);
		CHECK_APPROX_EQUAL(box2.GetAngularVelocity(), Vec3::sZero());
	}

	// Two boxes colliding in the center, each has enough velocity to step over the other in 1 step, restitution = 1
	TEST_CASE("TestLinearCastBoxVsLinearCastBoxElastic")
	{
		PhysicsTestContext c(1.0f / cFrequency, 1);
		c.ZeroGravity();

		const float cPenetrationSlop = c.GetSystem()->GetPhysicsSettings().mPenetrationSlop;

		// Register listener
		LoggingContactListener listener;
		c.GetSystem()->SetContactListener(&listener);

		Body &box1 = c.CreateBox(cPos1, Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::LinearCast, Layers::MOVING, Vec3::sReplicate(cBoxExtent));
		box1.SetLinearVelocity(cVelocity);
		box1.SetRestitution(1.0f);

		Body &box2 = c.CreateBox(cPos2, Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::LinearCast, Layers::MOVING, Vec3::sReplicate(cBoxExtent));
		box2.SetLinearVelocity(-cVelocity);
		box2.SetRestitution(1.0f);

		c.SimulateSingleStep();

		// The bodies should have collided and the velocities reversed
		CHECK(listener.GetEntryCount() == 2);
		CHECK(listener.Contains(LoggingContactListener::EType::Validate, box1.GetID(), box2.GetID()));
		CHECK(listener.Contains(LoggingContactListener::EType::Add, box1.GetID(), box2.GetID()));
		CHECK_APPROX_EQUAL(box1.GetPosition(), RVec3(-cBoxExtent, 0, 0), cPenetrationSlop);
		CHECK_APPROX_EQUAL(box1.GetLinearVelocity(), -cVelocity);
		CHECK_APPROX_EQUAL(box1.GetAngularVelocity(), Vec3::sZero());
		CHECK_APPROX_EQUAL(box2.GetPosition(), RVec3(cBoxExtent, 0, 0), cPenetrationSlop);
		CHECK_APPROX_EQUAL(box2.GetLinearVelocity(), cVelocity);
		CHECK_APPROX_EQUAL(box2.GetAngularVelocity(), Vec3::sZero());

		listener.Clear();
		c.SimulateSingleStep();

		// In the second step the bodies should have moved away, but since they were initially overlapping we should have a contact persist callback
		CHECK(listener.GetEntryCount() == 2);
		CHECK(listener.Contains(LoggingContactListener::EType::Validate, box1.GetID(), box2.GetID()));
		CHECK(listener.Contains(LoggingContactListener::EType::Persist, box1.GetID(), box2.GetID()));
		CHECK_APPROX_EQUAL(box1.GetPosition(), RVec3(-cBoxExtent, 0, 0) - cVelocity / cFrequency, cPenetrationSlop);
		CHECK_APPROX_EQUAL(box1.GetLinearVelocity(), -cVelocity);
		CHECK_APPROX_EQUAL(box1.GetAngularVelocity(), Vec3::sZero());
		CHECK_APPROX_EQUAL(box2.GetPosition(), RVec3(cBoxExtent, 0, 0) + cVelocity / cFrequency, cPenetrationSlop);
		CHECK_APPROX_EQUAL(box2.GetLinearVelocity(), cVelocity);
		CHECK_APPROX_EQUAL(box2.GetAngularVelocity(), Vec3::sZero());

		listener.Clear();
		c.SimulateSingleStep();

		// In the third step the bodies have separated and a contact remove callback should have been received
		CHECK(listener.GetEntryCount() == 1);
		CHECK(listener.Contains(LoggingContactListener::EType::Remove, box1.GetID(), box2.GetID()));
		CHECK_APPROX_EQUAL(box1.GetPosition(), RVec3(-cBoxExtent, 0, 0) - 2.0f * cVelocity / cFrequency, cPenetrationSlop);
		CHECK_APPROX_EQUAL(box1.GetLinearVelocity(), -cVelocity);
		CHECK_APPROX_EQUAL(box1.GetAngularVelocity(), Vec3::sZero());
		CHECK_APPROX_EQUAL(box2.GetPosition(), RVec3(cBoxExtent, 0, 0) + 2.0f * cVelocity / cFrequency, cPenetrationSlop);
		CHECK_APPROX_EQUAL(box2.GetLinearVelocity(), cVelocity);
		CHECK_APPROX_EQUAL(box2.GetAngularVelocity(), Vec3::sZero());
	}

	// Two boxes colliding in the center, each has enough velocity to step over the other in 1 step, restitution = 0
	TEST_CASE("TestLinearCastBoxVsLinearCastBoxInelastic")
	{
		PhysicsTestContext c(1.0f / cFrequency, 1);
		c.ZeroGravity();

		const float cPenetrationSlop = c.GetSystem()->GetPhysicsSettings().mPenetrationSlop;

		// Register listener
		LoggingContactListener listener;
		c.GetSystem()->SetContactListener(&listener);

		Body &box1 = c.CreateBox(cPos1, Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::LinearCast, Layers::MOVING, Vec3::sReplicate(cBoxExtent));
		box1.SetLinearVelocity(cVelocity);

		Body &box2 = c.CreateBox(cPos2, Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::LinearCast, Layers::MOVING, Vec3::sReplicate(cBoxExtent));
		box2.SetLinearVelocity(-cVelocity);

		c.SimulateSingleStep();

		// The bodies should have collided and both are stopped
		CHECK(listener.GetEntryCount() == 2);
		CHECK(listener.Contains(LoggingContactListener::EType::Validate, box1.GetID(), box2.GetID()));
		CHECK(listener.Contains(LoggingContactListener::EType::Add, box1.GetID(), box2.GetID()));
		CHECK_APPROX_EQUAL(box1.GetPosition(), RVec3(-cBoxExtent, 0, 0), cPenetrationSlop);
		CHECK_APPROX_EQUAL(box1.GetLinearVelocity(), Vec3::sZero());
		CHECK_APPROX_EQUAL(box1.GetAngularVelocity(), Vec3::sZero());
		CHECK_APPROX_EQUAL(box2.GetPosition(), RVec3(cBoxExtent, 0, 0), cPenetrationSlop);
		CHECK_APPROX_EQUAL(box2.GetLinearVelocity(), Vec3::sZero());
		CHECK_APPROX_EQUAL(box2.GetAngularVelocity(), Vec3::sZero());

		// The bodies should persist to contact as they are not moving
		for (int i = 0; i < 10; ++i)
		{
			listener.Clear();
			c.SimulateSingleStep();

			if (i == 0)
			{
				// Only in the first step we will receive a validate callback since after this step the contact cache will be used
				CHECK(listener.GetEntryCount() == 2);
				CHECK(listener.Contains(LoggingContactListener::EType::Validate, box1.GetID(), box2.GetID()));
			}
			else
				CHECK(listener.GetEntryCount() == 1);
			CHECK(listener.Contains(LoggingContactListener::EType::Persist, box1.GetID(), box2.GetID()));
			CHECK_APPROX_EQUAL(box1.GetPosition(), RVec3(-cBoxExtent, 0, 0), cPenetrationSlop);
			CHECK_APPROX_EQUAL(box1.GetLinearVelocity(), Vec3::sZero());
			CHECK_APPROX_EQUAL(box1.GetAngularVelocity(), Vec3::sZero());
			CHECK_APPROX_EQUAL(box2.GetPosition(), RVec3(cBoxExtent, 0, 0), cPenetrationSlop);
			CHECK_APPROX_EQUAL(box2.GetLinearVelocity(), Vec3::sZero());
			CHECK_APPROX_EQUAL(box2.GetAngularVelocity(), Vec3::sZero());
		}
	}

	// Two boxes colliding in the center, linear cast vs inactive linear cast
	TEST_CASE("TestLinearCastBoxVsInactiveLinearCastBox")
	{
		PhysicsTestContext c(1.0f / cFrequency, 1);
		c.ZeroGravity();

		const float cPenetrationSlop = c.GetSystem()->GetPhysicsSettings().mPenetrationSlop;

		// Register listener
		LoggingContactListener listener;
		c.GetSystem()->SetContactListener(&listener);
		LoggingBodyActivationListener activation;
		c.GetSystem()->SetBodyActivationListener(&activation);

		Body &box1 = c.CreateBox(cPos1, Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::LinearCast, Layers::MOVING, Vec3::sReplicate(cBoxExtent));
		box1.SetLinearVelocity(cVelocity);

		Body &box2 = c.CreateBox(cPos2, Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::LinearCast, Layers::MOVING, Vec3::sReplicate(cBoxExtent), EActivation::DontActivate);
		CHECK(!box2.IsActive());

		c.SimulateSingleStep();

		// The bodies should have collided and body 2 should be activated, have velocity, but not moved in this step
		CHECK(listener.GetEntryCount() == 2);
		CHECK(listener.Contains(LoggingContactListener::EType::Validate, box1.GetID(), box2.GetID()));
		CHECK(listener.Contains(LoggingContactListener::EType::Add, box1.GetID(), box2.GetID()));
		Vec3 new_velocity = 0.5f * cVelocity;
		CHECK_APPROX_EQUAL(box1.GetPosition(), cPos2 - Vec3(2.0f * cBoxExtent, 0, 0), cPenetrationSlop);
		CHECK_APPROX_EQUAL(box1.GetLinearVelocity(), new_velocity);
		CHECK_APPROX_EQUAL(box1.GetAngularVelocity(), Vec3::sZero());
		CHECK_APPROX_EQUAL(box2.GetPosition(), cPos2);
		CHECK_APPROX_EQUAL(box2.GetLinearVelocity(), new_velocity);
		CHECK_APPROX_EQUAL(box2.GetAngularVelocity(), Vec3::sZero());
		CHECK(box2.IsActive());
		CHECK(activation.Contains(LoggingBodyActivationListener::EType::Activated, box2.GetID()));

		listener.Clear();
		c.SimulateSingleStep();

		// In the next step body 2 should have started to move
		CHECK(listener.GetEntryCount() == 2);
		CHECK(listener.Contains(LoggingContactListener::EType::Validate, box1.GetID(), box2.GetID()));
		CHECK(listener.Contains(LoggingContactListener::EType::Persist, box1.GetID(), box2.GetID()));
		CHECK_APPROX_EQUAL(box1.GetPosition(), cPos2 - Vec3(2.0f * cBoxExtent, 0, 0) + new_velocity / cFrequency, cPenetrationSlop);
		CHECK_APPROX_EQUAL(box1.GetLinearVelocity(), new_velocity);
		CHECK_APPROX_EQUAL(box1.GetAngularVelocity(), Vec3::sZero());
		CHECK_APPROX_EQUAL(box2.GetPosition(), cPos2 + new_velocity / cFrequency);
		CHECK_APPROX_EQUAL(box2.GetLinearVelocity(), new_velocity);
		CHECK_APPROX_EQUAL(box2.GetAngularVelocity(), Vec3::sZero());
	}

	// Two boxes colliding in the center, linear cast vs inactive discrete
	TEST_CASE("TestLinearCastBoxVsInactiveDiscreteBox")
	{
		PhysicsTestContext c(1.0f / cFrequency, 1);
		c.ZeroGravity();

		const float cPenetrationSlop = c.GetSystem()->GetPhysicsSettings().mPenetrationSlop;

		// Register listener
		LoggingContactListener listener;
		c.GetSystem()->SetContactListener(&listener);
		LoggingBodyActivationListener activation;
		c.GetSystem()->SetBodyActivationListener(&activation);

		Body &box1 = c.CreateBox(cPos1, Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::LinearCast, Layers::MOVING, Vec3::sReplicate(cBoxExtent));
		box1.SetLinearVelocity(cVelocity);

		Body &box2 = c.CreateBox(cPos2, Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING, Vec3::sReplicate(cBoxExtent), EActivation::DontActivate);
		CHECK(!box2.IsActive());

		c.SimulateSingleStep();

		// The bodies should have collided and body 2 should be activated, have velocity, but not moved in this step
		CHECK(listener.GetEntryCount() == 2);
		CHECK(listener.Contains(LoggingContactListener::EType::Validate, box1.GetID(), box2.GetID()));
		CHECK(listener.Contains(LoggingContactListener::EType::Add, box1.GetID(), box2.GetID()));
		Vec3 new_velocity = 0.5f * cVelocity;
		CHECK_APPROX_EQUAL(box1.GetPosition(), cPos2 - Vec3(2.0f * cBoxExtent, 0, 0), cPenetrationSlop);
		CHECK_APPROX_EQUAL(box1.GetLinearVelocity(), new_velocity);
		CHECK_APPROX_EQUAL(box1.GetAngularVelocity(), Vec3::sZero());
		CHECK_APPROX_EQUAL(box2.GetPosition(), cPos2);
		CHECK_APPROX_EQUAL(box2.GetLinearVelocity(), new_velocity);
		CHECK_APPROX_EQUAL(box2.GetAngularVelocity(), Vec3::sZero());
		CHECK(box2.IsActive());
		CHECK(activation.Contains(LoggingBodyActivationListener::EType::Activated, box2.GetID()));

		listener.Clear();
		c.SimulateSingleStep();

		// In the next step body 2 should have started to move
		CHECK(listener.GetEntryCount() == 2);
		CHECK(listener.Contains(LoggingContactListener::EType::Validate, box1.GetID(), box2.GetID()));
		CHECK(listener.Contains(LoggingContactListener::EType::Persist, box1.GetID(), box2.GetID()));
		CHECK_APPROX_EQUAL(box1.GetPosition(), cPos2 - Vec3(2.0f * cBoxExtent, 0, 0) + new_velocity / cFrequency, cPenetrationSlop);
		CHECK_APPROX_EQUAL(box1.GetLinearVelocity(), new_velocity);
		CHECK_APPROX_EQUAL(box1.GetAngularVelocity(), Vec3::sZero());
		CHECK_APPROX_EQUAL(box2.GetPosition(), cPos2 + new_velocity / cFrequency);
		CHECK_APPROX_EQUAL(box2.GetLinearVelocity(), new_velocity);
		CHECK_APPROX_EQUAL(box2.GetAngularVelocity(), Vec3::sZero());
	}

	// Two boxes colliding under an angle, linear cast vs inactive discrete
	TEST_CASE("TestLinearCastBoxVsInactiveDiscreteBoxAngled")
	{
		const Vec3 cAngledOffset1(1, 0, -2);
		const Vec3 cAngledVelocity = -cFrequency * 2 * cAngledOffset1;

		PhysicsTestContext c(1.0f / cFrequency, 1);
		c.ZeroGravity();

		const float cPenetrationSlop = c.GetSystem()->GetPhysicsSettings().mPenetrationSlop;

		// Register listener
		LoggingContactListener listener;
		c.GetSystem()->SetContactListener(&listener);
		LoggingBodyActivationListener activation;
		c.GetSystem()->SetBodyActivationListener(&activation);

		// Make sure box1 exactly hits the face of box2 in the center
		RVec3 pos1 = RVec3(2.0f * cBoxExtent, 0, 0) + cAngledOffset1;
		Body &box1 = c.CreateBox(pos1, Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::LinearCast, Layers::MOVING, Vec3::sReplicate(cBoxExtent));
		box1.SetLinearVelocity(cAngledVelocity);
		box1.SetRestitution(1.0f);
		box1.SetFriction(0.0f);

		Body &box2 = c.CreateBox(RVec3::sZero(), Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING, Vec3::sReplicate(cBoxExtent), EActivation::DontActivate);
		box2.SetRestitution(1.0f);
		box2.SetFriction(0.0f);
		CHECK(!box2.IsActive());

		c.SimulateSingleStep();

		// The bodies should have collided and body 2 should be activated, have inherited the x velocity of body 1, but not moved in this step. Body 1 should have lost all of its velocity in x direction.
		CHECK(listener.GetEntryCount() == 2);
		CHECK(listener.Contains(LoggingContactListener::EType::Validate, box1.GetID(), box2.GetID()));
		CHECK(listener.Contains(LoggingContactListener::EType::Add, box1.GetID(), box2.GetID()));
		Vec3 new_velocity1 = Vec3(0, 0, cAngledVelocity.GetZ());
		Vec3 new_velocity2 = Vec3(cAngledVelocity.GetX(), 0, 0);
		CHECK_APPROX_EQUAL(box1.GetPosition(), RVec3(2.0f * cBoxExtent, 0, 0), 2.3f * cPenetrationSlop); // We're moving 2x as fast in the z direction and the slop is allowed in x direction: sqrt(1^2 + 2^2) ~ 2.3
		CHECK_APPROX_EQUAL(box1.GetLinearVelocity(), new_velocity1, 1.0e-4f);
		CHECK_APPROX_EQUAL(box1.GetAngularVelocity(), Vec3::sZero(), 2.0e-4f);
		CHECK_APPROX_EQUAL(box2.GetPosition(), RVec3::sZero());
		CHECK_APPROX_EQUAL(box2.GetLinearVelocity(), new_velocity2, 1.0e-4f);
		CHECK_APPROX_EQUAL(box2.GetAngularVelocity(), Vec3::sZero(), 2.0e-4f);
		CHECK(box2.IsActive());
		CHECK(activation.Contains(LoggingBodyActivationListener::EType::Activated, box2.GetID()));
	}

	// Two boxes colliding in the center, linear cast vs fast moving discrete, should tunnel through because all discrete bodies are moved before linear cast bodies are tested
	TEST_CASE("TestLinearCastBoxVsFastDiscreteBox")
	{
		PhysicsTestContext c(1.0f / cFrequency, 1);
		c.ZeroGravity();

		// Register listener
		LoggingContactListener listener;
		c.GetSystem()->SetContactListener(&listener);

		Body &box1 = c.CreateBox(cPos1, Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::LinearCast, Layers::MOVING, Vec3::sReplicate(cBoxExtent));
		box1.SetLinearVelocity(cVelocity);

		Body &box2 = c.CreateBox(cPos2, Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING, Vec3::sReplicate(cBoxExtent));
		box2.SetLinearVelocity(-cVelocity);

		c.SimulateSingleStep();

		// No collisions should be reported and the bodies should have moved according to their velocity (tunneling through eachother)
		CHECK(listener.GetEntryCount() == 0);
		CHECK_APPROX_EQUAL(box1.GetPosition(), cPos1 + cVelocity / cFrequency);
		CHECK_APPROX_EQUAL(box1.GetLinearVelocity(), cVelocity);
		CHECK_APPROX_EQUAL(box1.GetAngularVelocity(), Vec3::sZero());
		CHECK_APPROX_EQUAL(box2.GetPosition(), cPos2 - cVelocity / cFrequency);
		CHECK_APPROX_EQUAL(box2.GetLinearVelocity(), -cVelocity);
		CHECK_APPROX_EQUAL(box2.GetAngularVelocity(), Vec3::sZero());
	}

	// Two boxes colliding in the center, linear cast vs moving discrete, discrete is slow enough not to tunnel through linear cast body
	TEST_CASE("TestLinearCastBoxVsSlowDiscreteBox")
	{
		PhysicsTestContext c(1.0f / cFrequency, 1);
		c.ZeroGravity();

		const float cPenetrationSlop = c.GetSystem()->GetPhysicsSettings().mPenetrationSlop;

		// Register listener
		LoggingContactListener listener;
		c.GetSystem()->SetContactListener(&listener);

		Body &box1 = c.CreateBox(cPos1, Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::LinearCast, Layers::MOVING, Vec3::sReplicate(cBoxExtent));
		box1.SetLinearVelocity(cVelocity);

		// In 1 step it should move -0.1 meter on the X axis
		const Vec3 cBox2Velocity = Vec3(-0.1f * cFrequency, 0, 0);

		Body &box2 = c.CreateBox(cPos2, Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING, Vec3::sReplicate(cBoxExtent));
		box2.SetLinearVelocity(cBox2Velocity);

		c.SimulateSingleStep();

		// The bodies should have collided and body 2 should have moved according to its discrete step
		CHECK(listener.GetEntryCount() == 2);
		CHECK(listener.Contains(LoggingContactListener::EType::Validate, box1.GetID(), box2.GetID()));
		CHECK(listener.Contains(LoggingContactListener::EType::Add, box1.GetID(), box2.GetID()));
		RVec3 new_pos2 = cPos2 + cBox2Velocity / cFrequency;
		Vec3 new_velocity = 0.5f * (cVelocity + cBox2Velocity);
		CHECK_APPROX_EQUAL(box1.GetPosition(), new_pos2 - Vec3(2.0f * cBoxExtent, 0, 0), cPenetrationSlop);
		CHECK_APPROX_EQUAL(box1.GetLinearVelocity(), new_velocity);
		CHECK_APPROX_EQUAL(box1.GetAngularVelocity(), Vec3::sZero());
		CHECK_APPROX_EQUAL(box2.GetPosition(), new_pos2);
		CHECK_APPROX_EQUAL(box2.GetLinearVelocity(), new_velocity);
		CHECK_APPROX_EQUAL(box2.GetAngularVelocity(), Vec3::sZero());
	}
}
