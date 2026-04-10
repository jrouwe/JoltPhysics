// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include "PhysicsTestContext.h"
#include "Layers.h"
#include "LoggingContactListener.h"
#include "LoggingSoftBodyContactListener.h"
#include <Jolt/Physics/SoftBody/SoftBodySharedSettings.h>
#include <Jolt/Physics/SoftBody/SoftBodyCreationSettings.h>
#include <Jolt/Physics/SoftBody/SoftBodyMotionProperties.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>

TEST_SUITE("SoftBodyTests")
{
	TEST_CASE("TestBendConstraint")
	{
		// Possible values for x3
		const Float3 x3_values[] = {
			Float3(0, 0, 1),	// forming flat plane
			Float3(0, 0, -1),	// overlapping
			Float3(0, 1, 0),	// 90 degrees concave
			Float3(0, -1, 0),	// 90 degrees convex
			Float3(0, 1, 1),	// 45 degrees concave
			Float3(0, -1, -1)	// 135 degrees convex
		};

		for (const Float3 &x3 : x3_values)
		{
			PhysicsTestContext c;
			PhysicsSystem *s = c.GetSystem();
			BodyInterface &bi = s->GetBodyInterface();

			// Create settings
			Ref<SoftBodySharedSettings> shared_settings = new SoftBodySharedSettings;

			/* Create two triangles with a shared edge, x3 = free, the rest is locked
			   x2
			e1/  \e3
			 /    \
			x0----x1
			 \ e0 /
			e2\  /e4
			   x3
			*/
			SoftBodySharedSettings::Vertex v;
			v.mPosition = Float3(-1, 0, 0);
			v.mInvMass = 0;
			shared_settings->mVertices.push_back(v);
			v.mPosition = Float3(1, 0, 0);
			shared_settings->mVertices.push_back(v);
			v.mPosition = Float3(0, 0, -1);
			shared_settings->mVertices.push_back(v);
			v.mPosition = x3;
			v.mInvMass = 1;
			shared_settings->mVertices.push_back(v);

			// Create the 2 triangles
			shared_settings->AddFace(SoftBodySharedSettings::Face(0, 1, 2));
			shared_settings->AddFace(SoftBodySharedSettings::Face(0, 3, 1));

			// Create edge and dihedral constraints
			SoftBodySharedSettings::VertexAttributes va;
			va.mShearCompliance = FLT_MAX;
			va.mBendCompliance = 0;
			shared_settings->CreateConstraints(&va, 1, SoftBodySharedSettings::EBendType::Dihedral);

			// Optimize the settings
			shared_settings->Optimize();

			// Create the soft body
			SoftBodyCreationSettings sb_settings(shared_settings, RVec3::sZero(), Quat::sIdentity(), Layers::MOVING);
			sb_settings.mGravityFactor = 0.0f;
			sb_settings.mAllowSleeping = false;
			sb_settings.mUpdatePosition = false;
			Body &body = *bi.CreateSoftBody(sb_settings);
			bi.AddBody(body.GetID(), EActivation::Activate);
			SoftBodyMotionProperties *mp = static_cast<SoftBodyMotionProperties *>(body.GetMotionProperties());

			// Test 4 angles to see if there are singularities (the dot product between the triangles has the same value for 2 configurations)
			for (float angle : { 0.0f, 90.0f, 180.0f, 270.0f })
			{
				// Perturb x3
				Vec3 perturbed_x3(x3);
				mp->GetVertex(3).mPosition = 0.5f * (Mat44::sRotationX(DegreesToRadians(angle)) * perturbed_x3);

				// Simulate
				c.Simulate(0.25f);

				// Should return to the original position
				CHECK_APPROX_EQUAL(mp->GetVertex(3).mPosition, Vec3(x3), 1.0e-3f);
			}
		}
	}

	// Test that applying a force to a soft body and rigid body of the same mass has the same effect
	TEST_CASE("TestApplyForce")
	{
		PhysicsTestContext c;
		PhysicsSystem *s = c.GetSystem();
		BodyInterface &bi = s->GetBodyInterface();

		// Soft body cube
		SoftBodyCreationSettings sb_box_settings(SoftBodySharedSettings::sCreateCube(6, 0.2f), RVec3::sZero(), Quat::sIdentity(), Layers::MOVING);
		sb_box_settings.mGravityFactor = 0.0f;
		sb_box_settings.mLinearDamping = 0.0f;
		Body &sb_box = *bi.CreateSoftBody(sb_box_settings);
		BodyID sb_id = sb_box.GetID();
		bi.AddBody(sb_id, EActivation::Activate);
		constexpr float cMass = 216; // 6 * 6 * 6 * 1 kg
		CHECK_APPROX_EQUAL(sb_box.GetMotionProperties()->GetInverseMass(), 1.0f / cMass);

		// Rigid body cube of same size and mass
		const RVec3 cRBBoxPos(0, 2, 0);
		BodyCreationSettings rb_box_settings(new BoxShape(Vec3::sReplicate(0.5f)), cRBBoxPos, Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
		rb_box_settings.mGravityFactor = 0.0f;
		rb_box_settings.mLinearDamping = 0.0f;
		rb_box_settings.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
		rb_box_settings.mMassPropertiesOverride.mMass = cMass;
		Body &rb_box = *bi.CreateBody(rb_box_settings);
		BodyID rb_id = rb_box.GetID();
		bi.AddBody(rb_id, EActivation::Activate);

		// Simulate for 3 seconds while applying the same force
		constexpr int cNumSteps = 180;
		const Vec3 cForce(10000.0f, 0, 0);
		for (int i = 0; i < cNumSteps; ++i)
		{
			bi.AddForce(sb_id, cForce, EActivation::Activate);
			bi.AddForce(rb_id, cForce, EActivation::Activate);
			c.SimulateSingleStep();
		}

		// Check that the rigid body moved as expected
		const float cTotalTime = cNumSteps * c.GetStepDeltaTime();
		const Vec3 cAcceleration = cForce / cMass;
		const RVec3 cExpectedPos = c.PredictPosition(cRBBoxPos, Vec3::sZero(), cAcceleration, cTotalTime);
		CHECK_APPROX_EQUAL(rb_box.GetPosition(), cExpectedPos);
		const Vec3 cExpectedVel = cAcceleration * cTotalTime;
		CHECK_APPROX_EQUAL(rb_box.GetLinearVelocity(), cExpectedVel, 1.0e-3f);
		CHECK_APPROX_EQUAL(rb_box.GetAngularVelocity(), Vec3::sZero());

		// Check that the soft body moved within 1% of that
		const RVec3 cExpectedPosSB = cExpectedPos - cRBBoxPos;
		CHECK_APPROX_EQUAL(sb_box.GetPosition(), cExpectedPosSB, 0.01f * cExpectedPosSB.Length());
		CHECK_APPROX_EQUAL(sb_box.GetLinearVelocity(), cExpectedVel, 2.0e-3f);
		CHECK_APPROX_EQUAL(sb_box.GetAngularVelocity(), Vec3::sZero(), 0.01f);
	}

	// Test that a rigid body sphere with high speed tunnels through a soft body cube when using discrete motion quality,
	// but collides and transfers momentum correctly when using linear cast motion quality. Test that we can affect
	// this behavior with the soft body contact listener.
	TEST_CASE("TestLinearCastSphereVsSoftBody")
	{
		constexpr float cDeltaTime = 1.0f / 60.0f;
		constexpr float cSphereMass = 200.0f;
		constexpr float cSoftBodyMass = 6.0f * 6.0f * 6.0f; // * 1 kg

		// Sphere starts at x = 2 and moves by -5 in one time step, towards the soft body cube at the origin (spans [-0.5, 0.5])
		// Velocity is high enough to tunnel through the cube in one simulation step
		const RVec3 cSphereStartPos(2.0f, 0, 0);
		const Vec3 cSphereVelocity(-5.0f / cDeltaTime, 0, 0);

		// Expensive to make, create outside of loop
		RefConst<SoftBodySharedSettings> cube = SoftBodySharedSettings::sCreateCube(6, 0.2f);

		enum class Test
		{
			NoContactListener,
			WithContactListener,
			WithRejectingContactListener
		};

		for (Test test : { Test::NoContactListener, Test::WithContactListener, Test::WithRejectingContactListener })
			for (EMotionQuality motion_quality : { EMotionQuality::Discrete, EMotionQuality::LinearCast })
			{
				PhysicsTestContext c(cDeltaTime, 1);
				c.ZeroGravity();
				BodyInterface &bi = c.GetBodyInterface();

				// Create rigid body sphere approaching the cube
				BodyCreationSettings sphere_settings(new SphereShape(1.0f), cSphereStartPos, Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
				sphere_settings.mMotionQuality = motion_quality;
				sphere_settings.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
				sphere_settings.mMassPropertiesOverride.mMass = cSphereMass;
				sphere_settings.mLinearVelocity = cSphereVelocity;
				sphere_settings.mLinearDamping = 0.0f;
				Body &sphere = c.CreateBody(sphere_settings, EActivation::Activate);
				CHECK_APPROX_EQUAL(sphere.GetMotionProperties()->GetInverseMass(), 1.0f / cSphereMass);

				// Create soft body cube at origin (size 1 with 5 subdivisions)
				SoftBodyCreationSettings sb_settings(cube, RVec3::sZero(), Quat::sIdentity(), Layers::MOVING);
				Body &soft_body = *bi.CreateSoftBody(sb_settings);
				bi.AddBody(soft_body.GetID(), EActivation::Activate);
				CHECK_APPROX_EQUAL(soft_body.GetMotionProperties()->GetInverseMass(), 1.0f / cSoftBodyMass);

				LoggingContactListener listener;
				c.GetSystem()->SetContactListener(&listener);

				// Optionally install a soft body contact listener
				LoggingSoftBodyContactListener soft_body_listener;
				if (test != Test::NoContactListener)
					c.GetSystem()->SetSoftBodyContactListener(&soft_body_listener);
				if (test == Test::WithRejectingContactListener)
					soft_body_listener.SetValidateValueToReturn(SoftBodyValidateResult::RejectContact);

				// Simulate one step
				c.SimulateSingleStep();

				// We shouldn't be receiving any contacts on the normal contact listener
				CHECK(listener.GetEntryCount() == 0);

				if (motion_quality == EMotionQuality::Discrete || test == Test::WithRejectingContactListener)
				{
					// With discrete motion  quality / rejected contacts the sphere should have tunneled through the soft body without any collision
					CHECK_APPROX_EQUAL(sphere.GetPosition(), cSphereStartPos + cDeltaTime * cSphereVelocity);
					CHECK_APPROX_EQUAL(sphere.GetLinearVelocity(), cSphereVelocity);
					CHECK(soft_body.GetLinearVelocity() == Vec3::sZero());

					// No contacts should have been detected
					if (motion_quality == EMotionQuality::Discrete)
						CHECK(soft_body_listener.GetEntryCount() == 0);
					else
					{
						CHECK(soft_body_listener.Contains(LoggingSoftBodyContactListener::EType::Validate, soft_body.GetID(), sphere.GetID())); // Should have gotten a validate that we rejected
						CHECK(!soft_body_listener.Contains(LoggingSoftBodyContactListener::EType::Add, soft_body.GetID(), sphere.GetID()));
					}
				}
				else // LinearCast
				{
					// With linear cast motion quality the sphere should have collided with the soft body and stopped there (we do time stealing)
					CHECK_APPROX_EQUAL(sphere.GetPosition(), RVec3(1.5f, 0, 0), 1.001f * c.GetSystem()->GetPhysicsSettings().mPenetrationSlop);

					// The soft body should have gained velocity in the direction the sphere was traveling
					CHECK(soft_body.GetLinearVelocity().GetX() < 0.0f);

					// Check that momentum is conserved (GetLinearVelocity returns the center of mass velocity of the soft body)
					// Note that because we apply the collision impulse to a face, some rotational velocity will be introduced and we lose some linear momentum
					float initial_momentum = cSphereMass * cSphereVelocity.GetX();
					float final_momentum = cSphereMass * sphere.GetLinearVelocity().GetX() + cSoftBodyMass * soft_body.GetLinearVelocity().GetX();
					CHECK_APPROX_EQUAL(final_momentum, initial_momentum, 0.05f * abs(initial_momentum));

					if (test == Test::NoContactListener)
						CHECK(soft_body_listener.GetEntryCount() == 0);
					else
					{
						// We should have received one contact for the soft body
						CHECK(soft_body_listener.Contains(LoggingSoftBodyContactListener::EType::Validate, soft_body.GetID(), sphere.GetID()));
						CHECK(soft_body_listener.Contains(LoggingSoftBodyContactListener::EType::Add, soft_body.GetID(), sphere.GetID()));
					}
				}
			}
	}
}
