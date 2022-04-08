// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include "PhysicsTestContext.h"
#include "Layers.h"
#include "LoggingBodyActivationListener.h"
#include "LoggingContactListener.h"
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Jolt/Physics/Body/BodyLockMulti.h>

TEST_SUITE("PhysicsTests")
{
	// Gravity vector
	const Vec3 cGravity = Vec3(0.0f, -9.81f, 0.0f);

	// Test the test framework's helper functions
	TEST_CASE("TestPhysicsTestContext")
	{
		// Test that the Symplectic Euler integrator is close enough to the real value
		const float cSimulationTime = 2.0f;

		// For position: x = x0 + v0 * t + 1/2 * a * t^2
		const Vec3 cInitialPos(0.0f, 10.0f, 0.0f);
		PhysicsTestContext c;
		Vec3 simulated_pos = c.PredictPosition(cInitialPos, Vec3::sZero(), cGravity, cSimulationTime);
		Vec3 integrated_position = cInitialPos + 0.5f * cGravity * Square(cSimulationTime);
		CHECK_APPROX_EQUAL(integrated_position, simulated_pos, 0.2f);

		// For rotation
		const Quat cInitialRot(Quat::sRotation(Vec3::sAxisY(), 0.1f));
		const Vec3 cAngularAcceleration(0.0f, 2.0f, 0.0f);
		Quat simulated_rot = c.PredictOrientation(cInitialRot, Vec3::sZero(), cAngularAcceleration, cSimulationTime);
		Vec3 integrated_acceleration = 0.5f * cAngularAcceleration * Square(cSimulationTime);
		float integrated_acceleration_len = integrated_acceleration.Length();
		Quat integrated_rot = Quat::sRotation(integrated_acceleration / integrated_acceleration_len, integrated_acceleration_len) * cInitialRot;
		CHECK_APPROX_EQUAL(integrated_rot, simulated_rot, 0.02f);
	}

	TEST_CASE("TestPhysicsBodyLock")
	{
		PhysicsTestContext c;

		// Check that we cannot lock the invalid body ID
		{
			BodyLockRead lock(c.GetSystem()->GetBodyLockInterface(), BodyID());
			CHECK_FALSE(lock.Succeeded());
			CHECK_FALSE(lock.SucceededAndIsInBroadPhase());
		}

		BodyID body1_id;
		{
			// Create a box
			Body &body1 = c.CreateBox(Vec3::sZero(), Quat::sIdentity(), EMotionType::Static, EMotionQuality::Discrete, 0, Vec3::sReplicate(1.0f));
			body1_id = body1.GetID();
			CHECK(body1_id.GetIndex() == 0);
			CHECK(body1_id.GetSequenceNumber() == 1);

			// Create another box
			Body &body2 = c.CreateBox(Vec3::sZero(), Quat::sIdentity(), EMotionType::Static, EMotionQuality::Discrete, 0, Vec3::sReplicate(1.0f));
			BodyID body2_id = body2.GetID();
			CHECK(body2_id.GetIndex() == 1);
			CHECK(body2_id.GetSequenceNumber() == 1);

			// Check that we can lock the first box
			{
				BodyLockRead lock1(c.GetSystem()->GetBodyLockInterface(), body1_id);
				CHECK(lock1.Succeeded());
				CHECK(lock1.SucceededAndIsInBroadPhase());
			}

			// Remove the first box
			c.GetSystem()->GetBodyInterface().RemoveBody(body1_id);

			// Check that we can lock the first box
			{
				BodyLockWrite lock1(c.GetSystem()->GetBodyLockInterface(), body1_id);
				CHECK(lock1.Succeeded());
				CHECK_FALSE(lock1.SucceededAndIsInBroadPhase());
			}

			// Destroy the first box
			c.GetSystem()->GetBodyInterface().DestroyBody(body1_id);

			// Check that we can not lock the body anymore
			{
				BodyLockWrite lock1(c.GetSystem()->GetBodyLockInterface(), body1_id);
				CHECK_FALSE(lock1.Succeeded());
				CHECK_FALSE(lock1.SucceededAndIsInBroadPhase());
			}
		}

		// Create another box
		Body &body3 = c.CreateBox(Vec3::sZero(), Quat::sIdentity(), EMotionType::Static, EMotionQuality::Discrete, 0, Vec3::sReplicate(1.0f));
		BodyID body3_id = body3.GetID();
		CHECK(body3_id.GetIndex() == 0); // Check index reused
		CHECK(body3_id.GetSequenceNumber() == 2); // Check sequence number changed

		// Check that we can lock it
		{
			BodyLockRead lock3(c.GetSystem()->GetBodyLockInterface(), body3_id);
			CHECK(lock3.Succeeded());
			CHECK(lock3.SucceededAndIsInBroadPhase());
		}

		// Check that we can't lock the old body with the same body index anymore
		{
			BodyLockRead lock1(c.GetSystem()->GetBodyLockInterface(), body1_id);
			CHECK_FALSE(lock1.Succeeded());
			CHECK_FALSE(lock1.SucceededAndIsInBroadPhase());
		}
	}		

	TEST_CASE("TestPhysicsBodyLockMulti")
	{
		PhysicsTestContext c;

		// Check that we cannot lock the invalid body ID
		{
			BodyID bodies[] = { BodyID(), BodyID() };
			BodyLockMultiRead lock(c.GetSystem()->GetBodyLockInterface(), bodies, 2);
			CHECK(lock.GetBody(0) == nullptr);
			CHECK(lock.GetBody(1) == nullptr);
		}

		{
			// Create two bodies
			Body &body1 = c.CreateBox(Vec3::sZero(), Quat::sIdentity(), EMotionType::Static, EMotionQuality::Discrete, 0, Vec3::sReplicate(1.0f));
			Body &body2 = c.CreateBox(Vec3::sZero(), Quat::sIdentity(), EMotionType::Static, EMotionQuality::Discrete, 0, Vec3::sReplicate(1.0f));
			BodyID bodies[] = { body1.GetID(), body2.GetID() };

			{
				// Lock the bodies
				BodyLockMultiWrite lock(c.GetSystem()->GetBodyLockInterface(), bodies, 2);
				CHECK(lock.GetBody(0) == &body1);
				CHECK(lock.GetBody(1) == &body2);
			}

			// Destroy body 1
			c.GetSystem()->GetBodyInterface().RemoveBody(bodies[0]);
			c.GetSystem()->GetBodyInterface().DestroyBody(bodies[0]);

			{
				// Lock the bodies
				BodyLockMultiRead lock(c.GetSystem()->GetBodyLockInterface(), bodies, 2);
				CHECK(lock.GetBody(0) == nullptr);
				CHECK(lock.GetBody(1) == &body2);
			}
		}
	}

	TEST_CASE("TestPhysicsBodyID")
	{
		{
			BodyID body_id(0);
			CHECK(body_id.GetIndex() == 0);
			CHECK(body_id.GetSequenceNumber() == 0);
		}

		{
			BodyID body_id(~BodyID::cBroadPhaseBit);
			CHECK(body_id.GetIndex() == BodyID::cMaxBodyIndex);
			CHECK(body_id.GetSequenceNumber() == BodyID::cMaxSequenceNumber);
		}
	}

	TEST_CASE("TestPhysicsBodyIDSequenceNumber")
	{
		PhysicsTestContext c(1.0f / 60.0f, 1, 1);
		BodyInterface &bi = c.GetBodyInterface();

		// Create a body and check it's id
		BodyID body0_id = c.CreateBox(Vec3::sZero(), Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING, Vec3(1, 1, 1)).GetID();
		CHECK(body0_id == BodyID(0, 1)); // Body 0, sequence number 1

		// Check that the sequence numbers aren't reused until after 256 iterations
		for (int seq_no = 1; seq_no < 258; ++seq_no)
		{
			BodyID body1_id = c.CreateBox(Vec3::sZero(), Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING, Vec3(1, 1, 1)).GetID();
			CHECK(body1_id == BodyID(1, uint8(seq_no))); // Body 1

			bi.RemoveBody(body1_id);
			bi.DestroyBody(body1_id);
		}

		bi.RemoveBody(body0_id);
		bi.DestroyBody(body0_id);
	}

	TEST_CASE("TestPhysicsBodyUserData")
	{
		PhysicsTestContext c(1.0f / 60.0f, 1, 1);
		BodyInterface &bi = c.GetBodyInterface();

		// Create a body and pass user data through the creation settings
		BodyCreationSettings body_settings(new BoxShape(Vec3::sReplicate(1.0f)), Vec3::sZero(), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
		body_settings.mUserData = 0x1234567887654321;
		Body *body = bi.CreateBody(body_settings);
		CHECK(body->GetUserData() == 0x1234567887654321);

		// Change the user data
		body->SetUserData(0x5678123443218765);
		CHECK(body->GetUserData() == 0x5678123443218765);
	}

	TEST_CASE("TestPhysicsPosition")
	{
		PhysicsTestContext c(1.0f / 60.0f, 1, 1);
		BodyInterface &bi = c.GetBodyInterface();

		// Translate / rotate the box
		Vec3 box_pos(1, 2, 3);
		Quat box_rotation = Quat::sRotation(Vec3::sAxisX(), 0.25f * JPH_PI);

		// Translate / rotate the body
		Vec3 body_pos(4, 5, 6);
		Quat body_rotation = Quat::sRotation(Vec3::sAxisY(), 0.3f * JPH_PI);
		Mat44 body_transform = Mat44::sRotationTranslation(body_rotation, body_pos);
		Mat44 com_transform = body_transform * Mat44::sTranslation(box_pos);

		// Create body
		BodyCreationSettings body_settings(new RotatedTranslatedShapeSettings(box_pos, box_rotation, new BoxShape(Vec3::sReplicate(1.0f))), body_pos, body_rotation, EMotionType::Static, Layers::NON_MOVING);
		Body *body = bi.CreateBody(body_settings);

		// Check that the correct positions / rotations are reported
		CHECK_APPROX_EQUAL(body->GetPosition(), body_pos);
		CHECK_APPROX_EQUAL(body->GetRotation(), body_rotation);
		CHECK_APPROX_EQUAL(body->GetWorldTransform(), body_transform);
		CHECK_APPROX_EQUAL(body->GetCenterOfMassPosition(), com_transform.GetTranslation());
		CHECK_APPROX_EQUAL(body->GetCenterOfMassTransform(), com_transform);
		CHECK_APPROX_EQUAL(body->GetInverseCenterOfMassTransform(), com_transform.InversedRotationTranslation());
	}

	TEST_CASE("TestPhysicsOverrideMassAndInertia")
	{
		PhysicsTestContext c(1.0f / 60.0f, 1, 1);
		BodyInterface &bi = c.GetBodyInterface();

		const float cDensity = 1234.0f;
		const Vec3 cBoxExtent(2.0f, 4.0f, 6.0f);
		const float cExpectedMass = cBoxExtent.GetX() * cBoxExtent.GetY() * cBoxExtent.GetZ() * cDensity;
		// See: https://en.wikipedia.org/wiki/List_of_moments_of_inertia
		const Vec3 cSquaredExtents = Vec3(Square(cBoxExtent.GetY()) + Square(cBoxExtent.GetZ()), Square(cBoxExtent.GetX()) + Square(cBoxExtent.GetZ()), Square(cBoxExtent.GetX()) + Square(cBoxExtent.GetY()));
		const Vec3 cExpectedInertiaDiagonal = cExpectedMass / 12.0f * cSquaredExtents;

		Ref<BoxShapeSettings> shape_settings = new BoxShapeSettings(0.5f * cBoxExtent);
		shape_settings->SetDensity(cDensity);

		BodyCreationSettings body_settings(shape_settings, Vec3::sZero(), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);

		// Create body as is
		Body &b1 = *bi.CreateBody(body_settings);
		CHECK_APPROX_EQUAL(b1.GetMotionProperties()->GetInverseMass(), 1.0f / cExpectedMass);
		CHECK_APPROX_EQUAL(b1.GetMotionProperties()->GetInertiaRotation(), Quat::sIdentity());
		CHECK_APPROX_EQUAL(b1.GetMotionProperties()->GetInverseInertiaDiagonal(), cExpectedInertiaDiagonal.Reciprocal());

		// Override only the mass
		const float cOverriddenMass = 13.0f;
		const Vec3 cOverriddenMassInertiaDiagonal = cOverriddenMass / 12.0f * cSquaredExtents;

		body_settings.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
		body_settings.mMassPropertiesOverride.mMass = cOverriddenMass;
		Body &b2 = *bi.CreateBody(body_settings);
		CHECK_APPROX_EQUAL(b2.GetMotionProperties()->GetInverseMass(), 1.0f / cOverriddenMass);
		CHECK_APPROX_EQUAL(b2.GetMotionProperties()->GetInertiaRotation(), Quat::sIdentity());
		CHECK_APPROX_EQUAL(b2.GetMotionProperties()->GetInverseInertiaDiagonal(), cOverriddenMassInertiaDiagonal.Reciprocal());

		// Override both the mass and inertia
		const Vec3 cOverriddenInertiaDiagonal(3.0f, 2.0f, 1.0f); // From big to small so that MassProperties::DecomposePrincipalMomentsOfInertia returns the same rotation as we put in
		const Quat cOverriddenInertiaRotation = Quat::sRotation(Vec3(1, 1, 1).Normalized(), 0.1f * JPH_PI);

		body_settings.mOverrideMassProperties = EOverrideMassProperties::MassAndInertiaProvided;
		body_settings.mMassPropertiesOverride.mInertia = Mat44::sRotation(cOverriddenInertiaRotation) * Mat44::sScale(cOverriddenInertiaDiagonal) * Mat44::sRotation(cOverriddenInertiaRotation.Inversed());
		Body &b3 = *bi.CreateBody(body_settings);
		CHECK_APPROX_EQUAL(b3.GetMotionProperties()->GetInverseMass(), 1.0f / cOverriddenMass);
		CHECK_APPROX_EQUAL(b3.GetMotionProperties()->GetInertiaRotation(), cOverriddenInertiaRotation);
		CHECK_APPROX_EQUAL(b3.GetMotionProperties()->GetInverseInertiaDiagonal(), cOverriddenInertiaDiagonal.Reciprocal());
	}

	// Test a box free falling under gravity
	static void TestPhysicsFreeFall(PhysicsTestContext &ioContext)
	{
		const Vec3 cInitialPos(0.0f, 10.0f, 0.0f);
		const float cSimulationTime = 2.0f;

		// Create box
		Body &body = ioContext.CreateBox(cInitialPos, Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING, Vec3(1, 1, 1));
		CHECK_APPROX_EQUAL(cInitialPos, body.GetPosition());
		CHECK_APPROX_EQUAL(Vec3::sZero(), body.GetLinearVelocity());
			
		ioContext.Simulate(cSimulationTime);
			
		// Test resulting velocity (due to gravity)
		CHECK_APPROX_EQUAL(cSimulationTime * cGravity, body.GetLinearVelocity(), 1.0e-4f);
			
		// Test resulting position
		Vec3 expected_pos = ioContext.PredictPosition(cInitialPos, Vec3::sZero(), cGravity, cSimulationTime);
		CHECK_APPROX_EQUAL(expected_pos, body.GetPosition());
	}

	TEST_CASE("TestPhysicsFreeFall")
	{
		PhysicsTestContext c(1.0f / 60.0f, 1, 1);
		TestPhysicsFreeFall(c);
	}

	TEST_CASE("TestPhysicsFreeFallSubStep")
	{
		PhysicsTestContext c1(2.0f / 60.0f, 1, 2);
		TestPhysicsFreeFall(c1);

		PhysicsTestContext c2(4.0f / 60.0f, 1, 4);
		TestPhysicsFreeFall(c2);

		PhysicsTestContext c3(4.0f / 60.0f, 2, 2);
		TestPhysicsFreeFall(c3);

		PhysicsTestContext c4(2.0f / 60.0f, 2, 1);
		TestPhysicsFreeFall(c4);

		PhysicsTestContext c5(8.0f / 60.0f, 4, 2);
		TestPhysicsFreeFall(c5);

		PhysicsTestContext c6(4.0f / 60.0f, 4, 1);
		TestPhysicsFreeFall(c6);
	}

	// Test acceleration of a box with force applied
	static void TestPhysicsApplyForce(PhysicsTestContext &ioContext)
	{
		const Vec3 cInitialPos(0.0f, 10.0f, 0.0f);
		const Vec3 cAcceleration(2.0f, 0.0f, 0.0f);
		const float cSimulationTime = 2.0f;

		// Create box
		Body &body = ioContext.CreateBox(cInitialPos, Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING, Vec3(1, 1, 1));
		CHECK_APPROX_EQUAL(cInitialPos, body.GetPosition());
		CHECK_APPROX_EQUAL(Vec3::sZero(), body.GetLinearVelocity());

		// Validate mass
		float mass = Cubed(2.0f) * 1000.0f; // Density * Volume
		CHECK_APPROX_EQUAL(1.0f / mass, body.GetMotionProperties()->GetInverseMass());

		// Simulate while applying force
		ioContext.Simulate(cSimulationTime, [&]() { body.AddForce(mass * cAcceleration); });
			
		// Test resulting velocity (due to gravity and applied force)
		CHECK_APPROX_EQUAL(cSimulationTime * (cGravity + cAcceleration), body.GetLinearVelocity(), 1.0e-4f);
			
		// Test resulting position
		Vec3 expected_pos = ioContext.PredictPosition(cInitialPos, Vec3::sZero(), cGravity + cAcceleration, cSimulationTime);
		CHECK_APPROX_EQUAL(expected_pos, body.GetPosition());
	}

	TEST_CASE("TestPhysicsApplyForce")
	{
		PhysicsTestContext c(1.0f / 60.0f, 1, 1);
		TestPhysicsApplyForce(c);
	}

	TEST_CASE("TestPhysicsApplyForceSubStep")
	{
		PhysicsTestContext c1(2.0f / 60.0f, 1, 2);
		TestPhysicsApplyForce(c1);

		PhysicsTestContext c2(4.0f / 60.0f, 1, 4);
		TestPhysicsApplyForce(c2);

		PhysicsTestContext c3(4.0f / 60.0f, 2, 2);
		TestPhysicsApplyForce(c3);

		PhysicsTestContext c4(2.0f / 60.0f, 2, 1);
		TestPhysicsApplyForce(c4);

		PhysicsTestContext c5(8.0f / 60.0f, 4, 2);
		TestPhysicsApplyForce(c5);

		PhysicsTestContext c6(4.0f / 60.0f, 4, 1);
		TestPhysicsApplyForce(c6);
	}

	// Test angular accelartion for a box by applying torque every frame
	static void TestPhysicsApplyTorque(PhysicsTestContext &ioContext)
	{
		const Vec3 cInitialPos(0.0f, 10.0f, 0.0f);
		const Vec3 cAngularAcceleration(0.0f, 2.0f, 0.0f);
		const float cSimulationTime = 2.0f;

		// Create box
		Body &body = ioContext.CreateBox(cInitialPos, Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING, Vec3(1, 1, 1));
		CHECK_APPROX_EQUAL(Quat::sIdentity(), body.GetRotation());
		CHECK_APPROX_EQUAL(Vec3::sZero(), body.GetAngularVelocity());

		// Validate mass and inertia
		constexpr float mass = Cubed(2.0f) * 1000.0f; // Density * Volume
		CHECK_APPROX_EQUAL(1.0f / mass, body.GetMotionProperties()->GetInverseMass());
		constexpr float inertia = mass * 8.0f / 12.0f; // See: https://en.wikipedia.org/wiki/List_of_moments_of_inertia
		CHECK_APPROX_EQUAL(Mat44::sScale(1.0f / inertia), body.GetMotionProperties()->GetLocalSpaceInverseInertia());
			
		// Simulate while applying torque
		ioContext.Simulate(cSimulationTime, [&]() { body.AddTorque(inertia * cAngularAcceleration); });
			
		// Get resulting angular velocity
		CHECK_APPROX_EQUAL(cSimulationTime * cAngularAcceleration, body.GetAngularVelocity(), 1.0e-4f);
			
		// Test resulting rotation
		Quat expected_rot = ioContext.PredictOrientation(Quat::sIdentity(), Vec3::sZero(), cAngularAcceleration, cSimulationTime);
		CHECK_APPROX_EQUAL(expected_rot, body.GetRotation(), 1.0e-4f);
	}

	TEST_CASE("TestPhysicsApplyTorque")
	{
		PhysicsTestContext c(1.0f / 60.0f, 1, 1);
		TestPhysicsApplyTorque(c);
	}

	TEST_CASE("TestPhysicsApplyTorqueSubStep")
	{
		PhysicsTestContext c1(2.0f / 60.0f, 1, 2);
		TestPhysicsApplyTorque(c1);

		PhysicsTestContext c2(4.0f / 60.0f, 1, 4);
		TestPhysicsApplyTorque(c2);

		PhysicsTestContext c3(4.0f / 60.0f, 2, 2);
		TestPhysicsApplyTorque(c3);

		PhysicsTestContext c4(2.0f / 60.0f, 2, 1);
		TestPhysicsApplyTorque(c4);

		PhysicsTestContext c5(8.0f / 60.0f, 4, 2);
		TestPhysicsApplyTorque(c5);

		PhysicsTestContext c6(4.0f / 60.0f, 4, 1);
		TestPhysicsApplyTorque(c6);
	}		

	// Let a sphere bounce on the floor with restition = 1
	static void TestPhysicsCollisionElastic(PhysicsTestContext &ioContext)
	{
		const float cSimulationTime = 1.0f;
		const Vec3 cDistanceTraveled = ioContext.PredictPosition(Vec3::sZero(), Vec3::sZero(), cGravity, cSimulationTime);
		const float cFloorHitEpsilon = 1.0e-4f; // Apply epsilon so that we're sure that the collision algorithm will find a collision
		const Vec3 cFloorHitPos(0.0f, 1.0f - cFloorHitEpsilon, 0.0f); // Sphere with radius 1 will hit floor when 1 above the floor
		const Vec3 cInitialPos = cFloorHitPos - cDistanceTraveled;

		// Create sphere
		ioContext.CreateFloor();
		Body &body = ioContext.CreateSphere(cInitialPos, 1.0f, EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING);
		body.SetRestitution(1.0f);

		// Simulate until at floor
		ioContext.Simulate(cSimulationTime);
		CHECK_APPROX_EQUAL(cFloorHitPos, body.GetPosition());

		// Assert collision not yet processed
		CHECK_APPROX_EQUAL(cSimulationTime * cGravity, body.GetLinearVelocity(), 1.0e-4f); 

		// Simulate one more step to process the collision
		ioContext.Simulate(ioContext.GetDeltaTime());

		// Assert that collision is processed and velocity is reversed (which is required for a fully elastic collision). 
		// Note that the physics engine will first apply gravity for the time step and then do collision detection, 
		// hence the reflected velocity is actually 1 sub-step times gravity bigger than it would be in reality
		// For the remainder of cDeltaTime normal gravity will be applied
		float sub_step_delta_time = ioContext.GetSubStepDeltaTime();
		float remaining_step_time = ioContext.GetDeltaTime() - ioContext.GetSubStepDeltaTime();
		Vec3 reflected_velocity_after_sub_step = -(cSimulationTime + sub_step_delta_time) * cGravity;
		Vec3 reflected_velocity_after_full_step = reflected_velocity_after_sub_step + remaining_step_time * cGravity;
		CHECK_APPROX_EQUAL(reflected_velocity_after_full_step, body.GetLinearVelocity(), 1.0e-4f);

		// Body should have bounced back
		Vec3 pos_after_bounce_sub_step = cFloorHitPos + reflected_velocity_after_sub_step * sub_step_delta_time;
		Vec3 pos_after_bounce_full_step = ioContext.PredictPosition(pos_after_bounce_sub_step, reflected_velocity_after_sub_step, cGravity, remaining_step_time);
		CHECK_APPROX_EQUAL(pos_after_bounce_full_step, body.GetPosition());

		// Simulate same time, with a fully elastic body we should reach the initial position again
		// In our physics engine because of the velocity being too big we actually end up a bit higher than our initial position
		Vec3 expected_pos = ioContext.PredictPosition(pos_after_bounce_full_step, reflected_velocity_after_full_step, cGravity, cSimulationTime);
		ioContext.Simulate(cSimulationTime);
		CHECK_APPROX_EQUAL(expected_pos, body.GetPosition(), 1.0e-4f);
		CHECK(expected_pos.GetY() >= cInitialPos.GetY());
	}

	TEST_CASE("TestPhysicsCollisionElastic")
	{
		PhysicsTestContext c(1.0f / 60.0f, 1, 1);
		TestPhysicsCollisionElastic(c);
	}

	TEST_CASE("TestPhysicsCollisionElasticSubStep")
	{
		PhysicsTestContext c1(2.0f / 60.0f, 1, 2);
		TestPhysicsCollisionElastic(c1);

		PhysicsTestContext c2(4.0f / 60.0f, 1, 4);
		TestPhysicsCollisionElastic(c2);

		PhysicsTestContext c3(4.0f / 60.0f, 2, 2);
		TestPhysicsCollisionElastic(c3);

		PhysicsTestContext c4(2.0f / 60.0f, 2, 1);
		TestPhysicsCollisionElastic(c4);

		PhysicsTestContext c5(4.0f / 60.0f, 4, 1);
		TestPhysicsCollisionElastic(c5);
	}		

	// Let a sphere bounce on the floor with restitution = 0
	static void TestPhysicsCollisionInelastic(PhysicsTestContext &ioContext)
	{
		const float cSimulationTime = 1.0f;
		const Vec3 cDistanceTraveled = ioContext.PredictPosition(Vec3::sZero(), Vec3::sZero(), cGravity, cSimulationTime);
		const float cFloorHitEpsilon = 1.0e-4f; // Apply epsilon so that we're sure that the collision algorithm will find a collision
		const Vec3 cFloorHitPos(0.0f, 1.0f - cFloorHitEpsilon, 0.0f); // Sphere with radius 1 will hit floor when 1 above the floor
		const Vec3 cInitialPos = cFloorHitPos - cDistanceTraveled;

		// Create sphere
		ioContext.CreateFloor();
		Body &body = ioContext.CreateSphere(cInitialPos, 1.0f, EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING);
		body.SetRestitution(0.0f);

		// Simulate until at floor
		ioContext.Simulate(cSimulationTime);
		CHECK_APPROX_EQUAL(cFloorHitPos, body.GetPosition());

		// Assert collision not yet processed
		CHECK_APPROX_EQUAL(cSimulationTime * cGravity, body.GetLinearVelocity(), 1.0e-4f); 

		// Simulate one more step to process the collision
		ioContext.Simulate(ioContext.GetDeltaTime());

		// Assert that all velocity was lost in the collision
		CHECK_APPROX_EQUAL(Vec3::sZero(), body.GetLinearVelocity(), 1.0e-4f);

		// Assert that we're on the floor
		CHECK_APPROX_EQUAL(cFloorHitPos, body.GetPosition(), 1.0e-4f);

		// Simulate some more to validate that we remain on the floor
		ioContext.Simulate(cSimulationTime);
		CHECK_APPROX_EQUAL(Vec3::sZero(), body.GetLinearVelocity(), 1.0e-4f);
		CHECK_APPROX_EQUAL(cFloorHitPos, body.GetPosition(), 1.0e-4f);
	}

	TEST_CASE("TestPhysicsCollisionInelastic")
	{
		PhysicsTestContext c(1.0f / 60.0f, 1, 1);
		TestPhysicsCollisionInelastic(c);
	}

	TEST_CASE("TestPhysicsCollisionInelasticSubStep")
	{
		PhysicsTestContext c1(2.0f / 60.0f, 1, 2);
		TestPhysicsCollisionInelastic(c1);

		PhysicsTestContext c2(4.0f / 60.0f, 1, 4);
		TestPhysicsCollisionInelastic(c2);

		PhysicsTestContext c3(4.0f / 60.0f, 2, 2);
		TestPhysicsCollisionInelastic(c3);

		PhysicsTestContext c4(2.0f / 60.0f, 2, 1);
		TestPhysicsCollisionInelastic(c4);

		PhysicsTestContext c5(4.0f / 60.0f, 4, 1);
		TestPhysicsCollisionInelastic(c5);
	}		
		
	// Let box intersect with floor by cPenetrationSlop. It should not move, this is the maximum penetration allowed.
	static void TestPhysicsPenetrationSlop1(PhysicsTestContext &ioContext)
	{
		const float cPenetrationSlop = ioContext.GetSystem()->GetPhysicsSettings().mPenetrationSlop;
		const float cSimulationTime = 1.0f;
		const Vec3 cInitialPos(0.0f, 1.0f - cPenetrationSlop, 0.0f);

		// Create box, penetrating with floor
		ioContext.CreateFloor();
		Body &body = ioContext.CreateBox(cInitialPos, Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING, Vec3(1, 1, 1));

		// Simulate
		ioContext.Simulate(cSimulationTime);

		// Test slop not resolved
		CHECK_APPROX_EQUAL(cInitialPos, body.GetPosition(), 1.0e-5f);
		CHECK_APPROX_EQUAL(Vec3::sZero(), body.GetLinearVelocity());
		CHECK_APPROX_EQUAL(Vec3::sZero(), body.GetAngularVelocity());
	}

	TEST_CASE("TestPhysicsPenetrationSlop1")
	{
		PhysicsTestContext c(1.0f / 60.0f, 1, 1);
		TestPhysicsPenetrationSlop1(c);
	}

	TEST_CASE("TestPhysicsPenetrationSlop1SubStep")
	{
		PhysicsTestContext c(1.0f / 30.0f, 1, 2);
		TestPhysicsPenetrationSlop1(c);

		PhysicsTestContext c2(1.0f / 30.0f, 2, 1);
		TestPhysicsPenetrationSlop1(c2);
	}		

	// Let box intersect with floor with more than cPenetrationSlop. It should be resolved by SolvePositionConstraint until interpenetration is cPenetrationSlop.
	static void TestPhysicsPenetrationSlop2(PhysicsTestContext &ioContext)
	{
		const float cPenetrationSlop = ioContext.GetSystem()->GetPhysicsSettings().mPenetrationSlop;
		const float cSimulationTime = 1.0f;
		const Vec3 cInitialPos(0.0f, 1.0f - 2.0f * cPenetrationSlop, 0.0f);
		const Vec3 cFinalPos(0.0f, 1.0f - cPenetrationSlop, 0.0f);

		// Create box, penetrating with floor
		ioContext.CreateFloor();
		Body &body = ioContext.CreateBox(cInitialPos, Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING, Vec3(1, 1, 1));

		// Simulate
		ioContext.Simulate(cSimulationTime);

		// Test resolved until slop
		CHECK_APPROX_EQUAL(cFinalPos, body.GetPosition(), 1.0e-5f);
		CHECK_APPROX_EQUAL(Vec3::sZero(), body.GetLinearVelocity());
		CHECK_APPROX_EQUAL(Vec3::sZero(), body.GetAngularVelocity());
	}

	TEST_CASE("TestPhysicsPenetrationSlop2")
	{
		PhysicsTestContext c(1.0f / 60.0f, 1, 1);
		TestPhysicsPenetrationSlop2(c);
	}

	TEST_CASE("TestPhysicsPenetrationSlop2SubStep")
	{
		PhysicsTestContext c(1.0f / 30.0f, 1, 2);
		TestPhysicsPenetrationSlop2(c);

		PhysicsTestContext c2(1.0f / 30.0f, 2, 1);
		TestPhysicsPenetrationSlop2(c2);
	}		

	// Let box intersect with floor with less than cPenetrationSlop. Body should not move because SolveVelocityConstraint should reset velocity.
	static void TestPhysicsPenetrationSlop3(PhysicsTestContext &ioContext)
	{
		const float cPenetrationSlop = ioContext.GetSystem()->GetPhysicsSettings().mPenetrationSlop;
		const float cSimulationTime = 1.0f;
		const Vec3 cInitialPos(0.0f, 1.0f - 0.1f * cPenetrationSlop, 0.0f);

		// Create box, penetrating with floor
		ioContext.CreateFloor();
		Body &body = ioContext.CreateBox(cInitialPos, Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING, Vec3(1, 1, 1));

		// Simulate
		ioContext.Simulate(cSimulationTime);

		// Test body remained static
		CHECK_APPROX_EQUAL(cInitialPos, body.GetPosition(), 1.0e-5f);
		CHECK_APPROX_EQUAL(Vec3::sZero(), body.GetLinearVelocity());
		CHECK_APPROX_EQUAL(Vec3::sZero(), body.GetAngularVelocity());
	}

	TEST_CASE("TestPhysicsPenetrationSlop3")
	{
		PhysicsTestContext c(1.0f / 60.0f, 1, 1);
		TestPhysicsPenetrationSlop3(c);
	}

	TEST_CASE("TestPhysicsPenetrationSlop3SubStep")
	{
		PhysicsTestContext c(1.0f / 30.0f, 1, 2);
		TestPhysicsPenetrationSlop3(c);

		PhysicsTestContext c2(1.0f / 30.0f, 2, 1);
		TestPhysicsPenetrationSlop3(c2);
	}

	TEST_CASE("TestPhysicsOutsideOfSpeculativeContactDistance")
	{
		PhysicsTestContext c(1.0f / 60.0f, 1, 1);
		Body &floor = c.CreateFloor();
		c.ZeroGravity();

		LoggingContactListener contact_listener;
		c.GetSystem()->SetContactListener(&contact_listener);

		// Create a box and a sphere just outside the speculative contact distance
		const float cSpeculativeContactDistance = c.GetSystem()->GetPhysicsSettings().mSpeculativeContactDistance;
		const float cDistanceAboveFloor = 1.1f * cSpeculativeContactDistance;
		const Vec3 cInitialPosBox(0, 1.0f + cDistanceAboveFloor, 0.0f);
		const Vec3 cInitialPosSphere = cInitialPosBox + Vec3(5, 0, 0);

		// Make it move 1 m per step down
		const Vec3 cVelocity(0, -1.0f / c.GetDeltaTime(), 0);

		Body &box = c.CreateBox(cInitialPosBox, Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING, Vec3(1, 1, 1));
		box.SetLinearVelocity(cVelocity);

		Body &sphere = c.CreateSphere(cInitialPosSphere, 1.0f, EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING);
		sphere.SetLinearVelocity(cVelocity);

		// Simulate a step
		c.SimulateSingleStep();

		// Check that it is now penetrating the floor (collision should not have been detected as it is a discrete body and there was no collision initially)
		CHECK(contact_listener.GetEntryCount() == 0);
		CHECK_APPROX_EQUAL(box.GetPosition(), cInitialPosBox + cVelocity * c.GetDeltaTime());
		CHECK_APPROX_EQUAL(sphere.GetPosition(), cInitialPosSphere + cVelocity * c.GetDeltaTime());

		// Simulate a step
		c.SimulateSingleStep();

		// Check that the contacts are detected now
		CHECK(contact_listener.GetEntryCount() == 4); // 2 validates and 2 contacts
		CHECK(contact_listener.Contains(LoggingContactListener::EType::Validate, box.GetID(), floor.GetID()));
		CHECK(contact_listener.Contains(LoggingContactListener::EType::Add, box.GetID(), floor.GetID()));
		CHECK(contact_listener.Contains(LoggingContactListener::EType::Validate, sphere.GetID(), floor.GetID()));
		CHECK(contact_listener.Contains(LoggingContactListener::EType::Add, sphere.GetID(), floor.GetID()));
	}

	TEST_CASE("TestPhysicsInsideSpeculativeContactDistanceNoRestitution")
	{
		PhysicsTestContext c(1.0f / 60.0f, 1, 1);
		Body &floor = c.CreateFloor();
		c.ZeroGravity();

		LoggingContactListener contact_listener;
		c.GetSystem()->SetContactListener(&contact_listener);

		// Create a box and a sphere just inside the speculative contact distance
		const float cSpeculativeContactDistance = c.GetSystem()->GetPhysicsSettings().mSpeculativeContactDistance;
		const float cDistanceAboveFloor = 0.9f * cSpeculativeContactDistance;
		const Vec3 cInitialPosBox(0, 1.0f + cDistanceAboveFloor, 0.0f);
		const Vec3 cInitialPosSphere = cInitialPosBox + Vec3(5, 0, 0);

		// Make it move 1 m per step down
		const Vec3 cVelocity(0, -1.0f / c.GetDeltaTime(), 0);

		Body &box = c.CreateBox(cInitialPosBox, Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING, Vec3(1, 1, 1));
		box.SetLinearVelocity(cVelocity);

		Body &sphere = c.CreateSphere(cInitialPosSphere, 1.0f, EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING);
		sphere.SetLinearVelocity(cVelocity);

		// Simulate a step
		c.SimulateSingleStep();

		// Check that it is now on the floor and that 2 collisions have been detected
		CHECK(contact_listener.GetEntryCount() == 4); // 2 validates and 2 contacts
		CHECK(contact_listener.Contains(LoggingContactListener::EType::Validate, box.GetID(), floor.GetID()));
		CHECK(contact_listener.Contains(LoggingContactListener::EType::Add, box.GetID(), floor.GetID()));
		CHECK(contact_listener.Contains(LoggingContactListener::EType::Validate, sphere.GetID(), floor.GetID()));
		CHECK(contact_listener.Contains(LoggingContactListener::EType::Add, sphere.GetID(), floor.GetID()));
		contact_listener.Clear();

		// Velocity should have been reduced to exactly hit the floor in this step
		const Vec3 cExpectedVelocity(0, -cDistanceAboveFloor / c.GetDeltaTime(), 0);

		// Box collision is less accurate than sphere as it hits with 4 corners so there's some floating point precision loss in the calculation
		CHECK_APPROX_EQUAL(box.GetPosition(), Vec3(0, 1, 0), 1.0e-3f);
		CHECK_APPROX_EQUAL(box.GetLinearVelocity(), cExpectedVelocity, 0.05f);
		CHECK_APPROX_EQUAL(box.GetAngularVelocity(), Vec3::sZero(), 1.0e-2f);

		// Sphere has only 1 contact point so is much more accurate
		CHECK_APPROX_EQUAL(sphere.GetPosition(), Vec3(5, 1, 0));
		CHECK_APPROX_EQUAL(sphere.GetLinearVelocity(), cExpectedVelocity, 1.0e-4f); 
		CHECK_APPROX_EQUAL(sphere.GetAngularVelocity(), Vec3::sZero(), 1.0e-4f);

		// Simulate a step
		c.SimulateSingleStep();

		// Check that the contacts persisted
		CHECK(contact_listener.GetEntryCount() >= 2); // 2 persist and possibly 2 validates depending on if the cache got reused
		CHECK(contact_listener.Contains(LoggingContactListener::EType::Persist, box.GetID(), floor.GetID()));
		CHECK(contact_listener.Contains(LoggingContactListener::EType::Persist, sphere.GetID(), floor.GetID()));

		// Box should have come to rest
		CHECK_APPROX_EQUAL(box.GetPosition(), Vec3(0, 1, 0), 1.0e-3f);
		CHECK_APPROX_EQUAL(box.GetLinearVelocity(), Vec3::sZero(), 0.05f);
		CHECK_APPROX_EQUAL(box.GetAngularVelocity(), Vec3::sZero(), 1.0e-2f);

		// Sphere should have come to rest
		CHECK_APPROX_EQUAL(sphere.GetPosition(), Vec3(5, 1, 0), 1.0e-4f);
		CHECK_APPROX_EQUAL(sphere.GetLinearVelocity(), Vec3::sZero(), 1.0e-4f); 
		CHECK_APPROX_EQUAL(sphere.GetAngularVelocity(), Vec3::sZero(), 1.0e-4f);
	}

	TEST_CASE("TestPhysicsInsideSpeculativeContactDistanceWithRestitution")
	{
		PhysicsTestContext c(1.0f / 60.0f, 1, 1);
		Body &floor = c.CreateFloor();
		c.ZeroGravity();

		LoggingContactListener contact_listener;
		c.GetSystem()->SetContactListener(&contact_listener);

		// Create a box and a sphere just inside the speculative contact distance
		const float cSpeculativeContactDistance = c.GetSystem()->GetPhysicsSettings().mSpeculativeContactDistance;
		const float cDistanceAboveFloor = 0.9f * cSpeculativeContactDistance;
		const Vec3 cInitialPosBox(0, 1.0f + cDistanceAboveFloor, 0.0f);
		const Vec3 cInitialPosSphere = cInitialPosBox + Vec3(5, 0, 0);

		// Make it move 1 m per step down
		const Vec3 cVelocity(0, -1.0f / c.GetDeltaTime(), 0);

		Body &box = c.CreateBox(cInitialPosBox, Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING, Vec3(1, 1, 1));
		box.SetLinearVelocity(cVelocity);
		box.SetRestitution(1.0f);

		Body &sphere = c.CreateSphere(cInitialPosSphere, 1.0f, EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING);
		sphere.SetLinearVelocity(cVelocity);
		sphere.SetRestitution(1.0f);

		// Simulate a step
		c.SimulateSingleStep();

		// Check that it has triggered contact points and has bounced from it's initial position (effectively travelling the extra distance to the floor and back for free)
		CHECK(contact_listener.GetEntryCount() == 4); // 2 validates and 2 contacts
		CHECK(contact_listener.Contains(LoggingContactListener::EType::Validate, box.GetID(), floor.GetID()));
		CHECK(contact_listener.Contains(LoggingContactListener::EType::Add, box.GetID(), floor.GetID()));
		CHECK(contact_listener.Contains(LoggingContactListener::EType::Validate, sphere.GetID(), floor.GetID()));
		CHECK(contact_listener.Contains(LoggingContactListener::EType::Add, sphere.GetID(), floor.GetID()));
		contact_listener.Clear();

		// Box collision is less accurate than sphere as it hits with 4 corners so there's some floating point precision loss in the calculation
		CHECK_APPROX_EQUAL(box.GetPosition(), cInitialPosBox - cVelocity * c.GetDeltaTime(), 0.01f);
		CHECK_APPROX_EQUAL(box.GetLinearVelocity(), -cVelocity, 0.1f); 
		CHECK_APPROX_EQUAL(box.GetAngularVelocity(), Vec3::sZero(), 0.02f);

		// Sphere has only 1 contact point so is much more accurate
		CHECK_APPROX_EQUAL(sphere.GetPosition(), cInitialPosSphere - cVelocity * c.GetDeltaTime(), 1.0e-5f);
		CHECK_APPROX_EQUAL(sphere.GetLinearVelocity(), -cVelocity, 2.0e-4f); 
		CHECK_APPROX_EQUAL(sphere.GetAngularVelocity(), Vec3::sZero(), 2.0e-4f);

		// Simulate a step
		c.SimulateSingleStep();

		// Check that all contact points are removed
		CHECK(contact_listener.GetEntryCount() == 2); // 2 removes
		CHECK(contact_listener.Contains(LoggingContactListener::EType::Remove, box.GetID(), floor.GetID()));
		CHECK(contact_listener.Contains(LoggingContactListener::EType::Remove, sphere.GetID(), floor.GetID()));
	}

	TEST_CASE("TestPhysicsInsideSpeculativeContactDistanceMovingAway")
	{
		PhysicsTestContext c(1.0f / 60.0f, 1, 1);
		Body &floor = c.CreateFloor();
		c.ZeroGravity();

		LoggingContactListener contact_listener;
		c.GetSystem()->SetContactListener(&contact_listener);

		// Create a box and a sphere just inside the speculative contact distance
		const float cSpeculativeContactDistance = c.GetSystem()->GetPhysicsSettings().mSpeculativeContactDistance;
		const float cDistanceAboveFloor = 0.9f * cSpeculativeContactDistance;
		const Vec3 cInitialPosBox(0, 1.0f + cDistanceAboveFloor, 0.0f);
		const Vec3 cInitialPosSphere = cInitialPosBox + Vec3(5, 0, 0);

		// Make it move 1 m per step up
		const Vec3 cVelocity(0, 1.0f / c.GetDeltaTime(), 0);

		Body &box = c.CreateBox(cInitialPosBox, Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING, Vec3(1, 1, 1));
		box.SetLinearVelocity(cVelocity);
		box.SetRestitution(1.0f);

		Body &sphere = c.CreateSphere(cInitialPosSphere, 1.0f, EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING);
		sphere.SetLinearVelocity(cVelocity);
		sphere.SetRestitution(1.0f);

		// Simulate a step
		c.SimulateSingleStep();

		// Check that it has triggered contact points (note that this is wrong since the object never touched the floor but that's the downside of the speculative contacts -> you'll get an incorrect collision callback)
		CHECK(contact_listener.GetEntryCount() == 4); // 2 validates and 2 contacts
		CHECK(contact_listener.Contains(LoggingContactListener::EType::Validate, box.GetID(), floor.GetID()));
		CHECK(contact_listener.Contains(LoggingContactListener::EType::Add, box.GetID(), floor.GetID()));
		CHECK(contact_listener.Contains(LoggingContactListener::EType::Validate, sphere.GetID(), floor.GetID()));
		CHECK(contact_listener.Contains(LoggingContactListener::EType::Add, sphere.GetID(), floor.GetID()));
		contact_listener.Clear();

		// Box should have moved unimpeded
		CHECK_APPROX_EQUAL(box.GetPosition(), cInitialPosBox + cVelocity * c.GetDeltaTime());
		CHECK_APPROX_EQUAL(box.GetLinearVelocity(), cVelocity); 
		CHECK_APPROX_EQUAL(box.GetAngularVelocity(), Vec3::sZero());

		// Sphere should have moved unimpeded
		CHECK_APPROX_EQUAL(sphere.GetPosition(), cInitialPosSphere + cVelocity * c.GetDeltaTime());
		CHECK_APPROX_EQUAL(sphere.GetLinearVelocity(), cVelocity); 
		CHECK_APPROX_EQUAL(sphere.GetAngularVelocity(), Vec3::sZero());

		// Simulate a step
		c.SimulateSingleStep();

		// Check that all contact points are removed
		CHECK(contact_listener.GetEntryCount() == 2); // 2 removes
		CHECK(contact_listener.Contains(LoggingContactListener::EType::Remove, box.GetID(), floor.GetID()));
		CHECK(contact_listener.Contains(LoggingContactListener::EType::Remove, sphere.GetID(), floor.GetID()));
	}

	static void TestPhysicsActivationDeactivation(PhysicsTestContext &ioContext)
	{
		const float cPenetrationSlop = ioContext.GetSystem()->GetPhysicsSettings().mPenetrationSlop;

		// Install activation listener
		LoggingBodyActivationListener activation_listener;
		ioContext.GetSystem()->SetBodyActivationListener(&activation_listener);

		// Create floor
		Body &floor = ioContext.CreateBox(Vec3(0, -1, 0), Quat::sIdentity(), EMotionType::Static, EMotionQuality::Discrete, Layers::NON_MOVING, Vec3(100, 1, 100));
		CHECK(!floor.IsActive());

		// Create inactive box
		Body &box = ioContext.CreateBox(Vec3(0, 5, 0), Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING, Vec3::sReplicate(0.5f), EActivation::DontActivate);
		CHECK(!box.IsActive());
		CHECK(activation_listener.GetEntryCount() == 0);

		// Box should not activate by itself
		ioContext.Simulate(1.0f);
		CHECK(box.GetPosition() == Vec3(0, 5, 0));
		CHECK(!box.IsActive());
		CHECK(activation_listener.GetEntryCount() == 0);

		// Activate the body and validate it is active now
		ioContext.GetBodyInterface().ActivateBody(box.GetID());
		CHECK(box.IsActive());
		CHECK(box.GetLinearVelocity().IsNearZero());
		CHECK(activation_listener.GetEntryCount() == 1);
		CHECK(activation_listener.Contains(LoggingBodyActivationListener::EType::Activated, box.GetID()));
		activation_listener.Clear();

		// Do a single step and check that the body is still active and has gained some velocity
		ioContext.SimulateSingleStep();
		CHECK(box.IsActive());
		CHECK(activation_listener.GetEntryCount() == 0);
		CHECK(!box.GetLinearVelocity().IsNearZero());

		// Simulate 5 seconds and check it has settled on the floor and is no longer active
		ioContext.Simulate(5.0f);
		CHECK_APPROX_EQUAL(box.GetPosition(), Vec3(0, 0.5f, 0), 1.1f * cPenetrationSlop);
		CHECK_APPROX_EQUAL(box.GetLinearVelocity(), Vec3::sZero());
		CHECK_APPROX_EQUAL(box.GetAngularVelocity(), Vec3::sZero());
		CHECK(!box.IsActive());
		CHECK(activation_listener.GetEntryCount() == 1);
		CHECK(activation_listener.Contains(LoggingBodyActivationListener::EType::Deactivated, box.GetID()));
	}

	TEST_CASE("TestPhysicsActivationDeactivation")
	{
		PhysicsTestContext c1(1.0f / 60.0f, 1, 1);
		TestPhysicsActivationDeactivation(c1);

		PhysicsTestContext c2(2.0f / 60.0f, 1, 2);
		TestPhysicsActivationDeactivation(c2);

		PhysicsTestContext c3(2.0f / 60.0f, 2, 1);
		TestPhysicsActivationDeactivation(c3);

		PhysicsTestContext c4(4.0f / 60.0f, 4, 1);
		TestPhysicsActivationDeactivation(c4);

		PhysicsTestContext c5(8.0f / 60.0f, 4, 2);
		TestPhysicsActivationDeactivation(c5);
	}

	// A test that checks that a row of penetrating boxes will all activate and handle collision in 1 frame so that active bodies cannot tunnel through inactive bodies
	static void TestPhysicsActivateDuringStep(PhysicsTestContext &ioContext, bool inReverseCreate)
	{
		const float cPenetrationSlop = ioContext.GetSystem()->GetPhysicsSettings().mPenetrationSlop;
		const int cNumBodies = 10;
		const float cBoxExtent = 0.5f;

		PhysicsSystem *system = ioContext.GetSystem();
		BodyInterface &bi = ioContext.GetBodyInterface();

		LoggingBodyActivationListener activation_listener;
		system->SetBodyActivationListener(&activation_listener);

		LoggingContactListener contact_listener;
		system->SetContactListener(&contact_listener);

		// Create a row of penetrating boxes. Since some of the algorithms rely on body index, we create them normally and reversed to test both cases
		BodyIDVector body_ids;
		if (inReverseCreate)
			for (int i = cNumBodies - 1; i >= 0; --i)
				body_ids.insert(body_ids.begin(), ioContext.CreateBox(Vec3(i * (2.0f * cBoxExtent - cPenetrationSlop), 0, 0), Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING, Vec3::sReplicate(cBoxExtent), EActivation::DontActivate).GetID());
		else
			for (int i = 0; i < cNumBodies; ++i)
				body_ids.push_back(ioContext.CreateBox(Vec3(i * (2.0f * cBoxExtent - cPenetrationSlop), 0, 0), Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING, Vec3::sReplicate(0.5f), EActivation::DontActivate).GetID());

		// Test that nothing is active yet
		CHECK(activation_listener.GetEntryCount() == 0);
		CHECK(contact_listener.GetEntryCount() == 0);
		for (BodyID id : body_ids)
			CHECK(!bi.IsActive(id));

		// Activate the left most box and give it a velocity that is high enough to make it tunnel through the second box in a single step
		bi.SetLinearVelocity(body_ids.front(), Vec3(500, 0, 0));

		// Test that only the left most box is active
		CHECK(activation_listener.GetEntryCount() == 1);
		CHECK(contact_listener.GetEntryCount() == 0);
		CHECK(bi.IsActive(body_ids.front()));
		CHECK(activation_listener.Contains(LoggingBodyActivationListener::EType::Activated, body_ids.front()));
		for (int i = 1; i < cNumBodies; ++i)
			CHECK(!bi.IsActive(body_ids[i]));
		activation_listener.Clear();

		// Step the world
		ioContext.SimulateSingleStep();
		
		// Other bodies should now be awake and each body should only collide with its neighbour
		CHECK(activation_listener.GetEntryCount() == cNumBodies - 1);
		CHECK(contact_listener.GetEntryCount() == 2 * (cNumBodies - 1));

		for (int i = 0; i < cNumBodies; ++i)
		{
			BodyID id = body_ids[i];

			// Check body is active
			CHECK(bi.IsActive(id));

			// Check that body moved to the right
			CHECK(bi.GetPosition(id).GetX() > i * (2.0f * cBoxExtent - cPenetrationSlop));
		}

		for (int i = 1; i < cNumBodies; ++i)
		{
			BodyID id1 = body_ids[i - 1];
			BodyID id2 = body_ids[i];

			// Check that we received activation events for each body
			CHECK(activation_listener.Contains(LoggingBodyActivationListener::EType::Activated, id2));

			// Check that we received a validate and an add for each body pair
			int validate = contact_listener.Find(LoggingContactListener::EType::Validate, id1, id2);
			CHECK(validate >= 0);
			int add = contact_listener.Find(LoggingContactListener::EType::Add, id1, id2);
			CHECK(add >= 0);
			CHECK(add > validate);

			// Check that bodies did not tunnel through each other
			CHECK(bi.GetPosition(id1).GetX() < bi.GetPosition(id2).GetX());
		}
	}

	TEST_CASE("TestPhysicsActivateDuringStep")
	{
		PhysicsTestContext c(1.0f / 60.0f, 1, 1);
		TestPhysicsActivateDuringStep(c, false);

		PhysicsTestContext c2(1.0f / 60.0f, 1, 1);
		TestPhysicsActivateDuringStep(c2, true);
	}

	TEST_CASE("TestPhysicsBroadPhaseLayers")
	{
		PhysicsTestContext c(1.0f / 60.0f, 1, 1);
		BodyInterface &bi = c.GetBodyInterface();

		// Reduce slop
		PhysicsSettings settings = c.GetSystem()->GetPhysicsSettings();
		settings.mPenetrationSlop = 0.0f;
		c.GetSystem()->SetPhysicsSettings(settings);

		// Create static floor
		c.CreateFloor();

		// Create MOVING boxes
		Body &moving1 = c.CreateBox(Vec3(0, 1, 0), Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING, Vec3::sReplicate(0.5f), EActivation::Activate);
		Body &moving2 = c.CreateBox(Vec3(0, 2, 0), Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING, Vec3::sReplicate(0.5f), EActivation::Activate);

		// Create HQ_DEBRIS boxes
		Body &hq_debris1 = c.CreateBox(Vec3(0, 3, 0), Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::Discrete, Layers::HQ_DEBRIS, Vec3::sReplicate(0.5f), EActivation::Activate);
		Body &hq_debris2 = c.CreateBox(Vec3(0, 4, 0), Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::Discrete, Layers::HQ_DEBRIS, Vec3::sReplicate(0.5f), EActivation::Activate);

		// Create LQ_DEBRIS boxes
		Body &lq_debris1 = c.CreateBox(Vec3(0, 5, 0), Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::Discrete, Layers::LQ_DEBRIS, Vec3::sReplicate(0.5f), EActivation::Activate);
		Body &lq_debris2 = c.CreateBox(Vec3(0, 6, 0), Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::Discrete, Layers::LQ_DEBRIS, Vec3::sReplicate(0.5f), EActivation::Activate);

		// Check layers
		CHECK(moving1.GetObjectLayer() == Layers::MOVING);
		CHECK(moving2.GetObjectLayer() == Layers::MOVING);
		CHECK(hq_debris1.GetObjectLayer() == Layers::HQ_DEBRIS);
		CHECK(hq_debris2.GetObjectLayer() == Layers::HQ_DEBRIS);
		CHECK(lq_debris1.GetObjectLayer() == Layers::LQ_DEBRIS);
		CHECK(lq_debris2.GetObjectLayer() == Layers::LQ_DEBRIS);
		CHECK(moving1.GetBroadPhaseLayer() == BroadPhaseLayers::MOVING);
		CHECK(moving2.GetBroadPhaseLayer() == BroadPhaseLayers::MOVING);
		CHECK(hq_debris1.GetBroadPhaseLayer() == BroadPhaseLayers::MOVING);
		CHECK(hq_debris2.GetBroadPhaseLayer() == BroadPhaseLayers::MOVING);
		CHECK(lq_debris1.GetBroadPhaseLayer() == BroadPhaseLayers::LQ_DEBRIS);
		CHECK(lq_debris2.GetBroadPhaseLayer() == BroadPhaseLayers::LQ_DEBRIS);

		// Simulate the boxes falling
		c.Simulate(5.0f);

		// Everything should sleep
		CHECK_FALSE(moving1.IsActive());
		CHECK_FALSE(moving2.IsActive());
		CHECK_FALSE(hq_debris1.IsActive());
		CHECK_FALSE(hq_debris2.IsActive());
		CHECK_FALSE(lq_debris1.IsActive());
		CHECK_FALSE(lq_debris2.IsActive());

		// MOVING boxes should have stacked
		float slop = 0.02f;
		CHECK_APPROX_EQUAL(moving1.GetPosition(), Vec3(0, 0.5f, 0), slop);
		CHECK_APPROX_EQUAL(moving2.GetPosition(), Vec3(0, 1.5f, 0), slop);

		// HQ_DEBRIS boxes should have stacked on MOVING boxes but don't collide with each other
		CHECK_APPROX_EQUAL(hq_debris1.GetPosition(), Vec3(0, 2.5f, 0), slop);
		CHECK_APPROX_EQUAL(hq_debris2.GetPosition(), Vec3(0, 2.5f, 0), slop);

		// LQ_DEBRIS should have fallen through all but the floor
		CHECK_APPROX_EQUAL(lq_debris1.GetPosition(), Vec3(0, 0.5f, 0), slop);
		CHECK_APPROX_EQUAL(lq_debris2.GetPosition(), Vec3(0, 0.5f, 0), slop);

		// Now change HQ_DEBRIS to LQ_DEBRIS
		bi.SetObjectLayer(hq_debris1.GetID(), Layers::LQ_DEBRIS);
		bi.SetObjectLayer(hq_debris2.GetID(), Layers::LQ_DEBRIS);
		bi.ActivateBody(hq_debris1.GetID());
		bi.ActivateBody(hq_debris2.GetID());

		// Check layers
		CHECK(moving1.GetObjectLayer() == Layers::MOVING);
		CHECK(moving2.GetObjectLayer() == Layers::MOVING);
		CHECK(hq_debris1.GetObjectLayer() == Layers::LQ_DEBRIS);
		CHECK(hq_debris2.GetObjectLayer() == Layers::LQ_DEBRIS);
		CHECK(lq_debris1.GetObjectLayer() == Layers::LQ_DEBRIS);
		CHECK(lq_debris2.GetObjectLayer() == Layers::LQ_DEBRIS);
		CHECK(moving1.GetBroadPhaseLayer() == BroadPhaseLayers::MOVING);
		CHECK(moving2.GetBroadPhaseLayer() == BroadPhaseLayers::MOVING);
		CHECK(hq_debris1.GetBroadPhaseLayer() == BroadPhaseLayers::LQ_DEBRIS);
		CHECK(hq_debris2.GetBroadPhaseLayer() == BroadPhaseLayers::LQ_DEBRIS);
		CHECK(lq_debris1.GetBroadPhaseLayer() == BroadPhaseLayers::LQ_DEBRIS);
		CHECK(lq_debris2.GetBroadPhaseLayer() == BroadPhaseLayers::LQ_DEBRIS);

		// Simulate again
		c.Simulate(5.0f);

		// Everything should sleep
		CHECK_FALSE(moving1.IsActive());
		CHECK_FALSE(moving2.IsActive());
		CHECK_FALSE(hq_debris1.IsActive());
		CHECK_FALSE(hq_debris2.IsActive());
		CHECK_FALSE(lq_debris1.IsActive());
		CHECK_FALSE(lq_debris2.IsActive());

		// MOVING boxes should have stacked
		CHECK_APPROX_EQUAL(moving1.GetPosition(), Vec3(0, 0.5f, 0), slop);
		CHECK_APPROX_EQUAL(moving2.GetPosition(), Vec3(0, 1.5f, 0), slop);

		// HQ_DEBRIS (now LQ_DEBRIS) boxes have fallen through all but the floor
		CHECK_APPROX_EQUAL(hq_debris1.GetPosition(), Vec3(0, 0.5f, 0), slop);
		CHECK_APPROX_EQUAL(hq_debris2.GetPosition(), Vec3(0, 0.5f, 0), slop);

		// LQ_DEBRIS should have fallen through all but the floor
		CHECK_APPROX_EQUAL(lq_debris1.GetPosition(), Vec3(0, 0.5f, 0), slop);
		CHECK_APPROX_EQUAL(lq_debris2.GetPosition(), Vec3(0, 0.5f, 0), slop);

		// Now change MOVING to HQ_DEBRIS (this doesn't change the broadphase layer so avoids adding/removing bodies)
		bi.SetObjectLayer(moving1.GetID(), Layers::HQ_DEBRIS);
		bi.SetObjectLayer(moving2.GetID(), Layers::HQ_DEBRIS);
		bi.ActivateBody(moving1.GetID());
		bi.ActivateBody(moving2.GetID());

		// Check layers
		CHECK(moving1.GetObjectLayer() == Layers::HQ_DEBRIS);
		CHECK(moving2.GetObjectLayer() == Layers::HQ_DEBRIS);
		CHECK(hq_debris1.GetObjectLayer() == Layers::LQ_DEBRIS);
		CHECK(hq_debris2.GetObjectLayer() == Layers::LQ_DEBRIS);
		CHECK(lq_debris1.GetObjectLayer() == Layers::LQ_DEBRIS);
		CHECK(lq_debris2.GetObjectLayer() == Layers::LQ_DEBRIS);
		CHECK(moving1.GetBroadPhaseLayer() == BroadPhaseLayers::MOVING); // Broadphase layer didn't change
		CHECK(moving2.GetBroadPhaseLayer() == BroadPhaseLayers::MOVING);
		CHECK(hq_debris1.GetBroadPhaseLayer() == BroadPhaseLayers::LQ_DEBRIS);
		CHECK(hq_debris2.GetBroadPhaseLayer() == BroadPhaseLayers::LQ_DEBRIS);
		CHECK(lq_debris1.GetBroadPhaseLayer() == BroadPhaseLayers::LQ_DEBRIS);
		CHECK(lq_debris2.GetBroadPhaseLayer() == BroadPhaseLayers::LQ_DEBRIS);

		// Simulate again
		c.Simulate(5.0f);

		// Everything should sleep
		CHECK_FALSE(moving1.IsActive());
		CHECK_FALSE(moving2.IsActive());
		CHECK_FALSE(hq_debris1.IsActive());
		CHECK_FALSE(hq_debris2.IsActive());
		CHECK_FALSE(lq_debris1.IsActive());
		CHECK_FALSE(lq_debris2.IsActive());

		// MOVING boxes now also fall through
		CHECK_APPROX_EQUAL(moving1.GetPosition(), Vec3(0, 0.5f, 0), slop);
		CHECK_APPROX_EQUAL(moving2.GetPosition(), Vec3(0, 0.5f, 0), slop);

		// HQ_DEBRIS (now LQ_DEBRIS) boxes have fallen through all but the floor
		CHECK_APPROX_EQUAL(hq_debris1.GetPosition(), Vec3(0, 0.5f, 0), slop);
		CHECK_APPROX_EQUAL(hq_debris2.GetPosition(), Vec3(0, 0.5f, 0), slop);

		// LQ_DEBRIS should have fallen through all but the floor
		CHECK_APPROX_EQUAL(lq_debris1.GetPosition(), Vec3(0, 0.5f, 0), slop);
		CHECK_APPROX_EQUAL(lq_debris2.GetPosition(), Vec3(0, 0.5f, 0), slop);
	}
}
