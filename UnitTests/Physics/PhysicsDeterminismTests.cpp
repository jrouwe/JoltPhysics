// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include "PhysicsTestContext.h"
#include "Layers.h"
#include <Jolt/Physics/Constraints/SwingTwistConstraint.h>
#include <Jolt/Physics/Collision/GroupFilterTable.h>

TEST_SUITE("PhysicsDeterminismTests")
{
	struct BodyProperties
	{
		Vec3	mPositionCOM;
		Quat	mRotation;
		Vec3	mLinearVelocity;
		Vec3	mAngularVelocity;
		AABox	mBounds;
		bool	mIsActive;
	};

	/// Extract all relevant properties of a body for the test
	static void GetBodyProperties(PhysicsTestContext &ioContext, const BodyID &inBodyID, BodyProperties &outProperties)
	{
		BodyLockRead lock(ioContext.GetSystem()->GetBodyLockInterface(), inBodyID);
		if (lock.SucceededAndIsInBroadPhase())
		{
			const Body &body = lock.GetBody();
			outProperties.mIsActive = body.IsActive();
			outProperties.mPositionCOM = body.GetCenterOfMassPosition();
			outProperties.mRotation = body.GetRotation();
			outProperties.mLinearVelocity = body.GetLinearVelocity();
			outProperties.mAngularVelocity = body.GetAngularVelocity();
			outProperties.mBounds = body.GetWorldSpaceBounds();
		}
		else
		{
			CHECK(false);
		}
	}

	/// Step two physics simulations for inTotalTime and check after each step that the simulations are identical
	static void CompareSimulations(PhysicsTestContext &ioContext1, PhysicsTestContext &ioContext2, float inTotalTime)
	{
		CHECK(ioContext1.GetDeltaTime() == ioContext2.GetDeltaTime());

		// Step until we've stepped for inTotalTime
		for (float t = 0; t <= inTotalTime; t += ioContext1.GetDeltaTime())
		{
			// Step the simulation
			ioContext1.SimulateSingleStep();
			ioContext2.SimulateSingleStep();

			// Get all bodies
			BodyIDVector bodies1, bodies2;
			ioContext1.GetSystem()->GetBodies(bodies1);
			ioContext2.GetSystem()->GetBodies(bodies2);
			CHECK(bodies1.size() == bodies2.size());

			// Loop over all bodies
			for (size_t b = 0; b < min(bodies1.size(), bodies2.size()); ++b)
			{
				// Check that the body ID's match
				BodyID b1_id = bodies1[b];
				BodyID b2_id = bodies2[b];
				CHECK(b1_id == b2_id);

				// Get the properties of the body
				BodyProperties properties1, properties2;
				GetBodyProperties(ioContext1, b1_id, properties1);
				GetBodyProperties(ioContext2, b2_id, properties2);
				CHECK(properties1.mIsActive == properties2.mIsActive);
				CHECK(properties1.mPositionCOM == properties2.mPositionCOM);
				CHECK(properties1.mRotation == properties2.mRotation);
				CHECK(properties1.mLinearVelocity == properties2.mLinearVelocity);
				CHECK(properties1.mAngularVelocity == properties2.mAngularVelocity);
				CHECK(properties1.mBounds.mMin == properties2.mBounds.mMin);
				CHECK(properties1.mBounds.mMax == properties2.mBounds.mMax);
			}
		}
	}

	static void CreateGridOfBoxesDiscrete(PhysicsTestContext &ioContext)
	{
		UnitTestRandom random;
		uniform_real_distribution<float> restitution(0.0f, 1.0f);

		ioContext.CreateFloor();

		for (int x = 0; x < 5; ++x)
			for (int z = 0; z < 5; ++z)
			{
				Body &body = ioContext.CreateBox(Vec3(float(x), 5.0f, float(z)), Quat::sRandom(random), EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING, Vec3::sReplicate(0.1f));
				body.SetRestitution(restitution(random));
				body.SetLinearVelocity(Vec3::sRandom(random));
			}
	}

	TEST_CASE("TestGridOfBoxesDiscrete")
	{
		PhysicsTestContext c1(1.0f / 60.0f, 1, 1, 0);
		CreateGridOfBoxesDiscrete(c1);

		PhysicsTestContext c2(1.0f / 60.0f, 1, 1, 15);
		CreateGridOfBoxesDiscrete(c2);

		CompareSimulations(c1, c2, 5.0f);
	}

	static void CreateGridOfBoxesLinearCast(PhysicsTestContext &ioContext)
	{
		UnitTestRandom random;
		uniform_real_distribution<float> restitution(0.0f, 1.0f);

		ioContext.CreateFloor();

		for (int x = 0; x < 5; ++x)
			for (int z = 0; z < 5; ++z)
			{
				Body &body = ioContext.CreateBox(Vec3(float(x), 5.0f, float(z)), Quat::sRandom(random), EMotionType::Dynamic, EMotionQuality::LinearCast, Layers::MOVING, Vec3::sReplicate(0.1f));
				body.SetRestitution(restitution(random));
				body.SetLinearVelocity(Vec3::sRandom(random) - Vec3(0, -5.0f, 0));
			}
	}

	TEST_CASE("TestGridOfBoxesLinearCast")
	{
		PhysicsTestContext c1(1.0f / 60.0f, 1, 1, 0);
		CreateGridOfBoxesLinearCast(c1);

		PhysicsTestContext c2(1.0f / 60.0f, 1, 1, 15);
		CreateGridOfBoxesLinearCast(c2);

		CompareSimulations(c1, c2, 5.0f);
	}

	static void CreateGridOfBoxesConstrained(PhysicsTestContext &ioContext)
	{
		UnitTestRandom random;
		uniform_real_distribution<float> restitution(0.0f, 1.0f);

		ioContext.CreateFloor();

		const int cNumPerAxis = 5;

		// Build a collision group filter that disables collision between adjacent bodies
		Ref<GroupFilterTable> group_filter = new GroupFilterTable(cNumPerAxis);
		for (CollisionGroup::SubGroupID i = 0; i < cNumPerAxis - 1; ++i)
			group_filter->DisableCollision(i, i + 1);

		// Create a number of chains
		for (int x = 0; x < cNumPerAxis; ++x)
		{
			// Create a chain of bodies connected with swing twist constraints
			Body *prev_body = nullptr;
			for (int z = 0; z < cNumPerAxis; ++z)
			{
				Vec3 body_pos = Vec3(float(x), 5.0f, 0.2f * float(z));
				Body &body = ioContext.CreateBox(body_pos, Quat::sRandom(random), EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING, Vec3::sReplicate(0.1f));
				body.SetRestitution(restitution(random));
				body.SetLinearVelocity(Vec3::sRandom(random));
				body.SetCollisionGroup(CollisionGroup(group_filter, CollisionGroup::GroupID(x), CollisionGroup::SubGroupID(z)));

				// Constrain the body to the previous body
				if (prev_body != nullptr)
				{
					SwingTwistConstraintSettings st;
					st.mPosition1 = st.mPosition2 = body_pos - Vec3(0, 0, 0.1f);
					st.mTwistAxis1 = st.mTwistAxis2 = Vec3::sAxisZ();
					st.mPlaneAxis1 = st.mPlaneAxis2 = Vec3::sAxisX();
					st.mNormalHalfConeAngle = DegreesToRadians(45.0f);
					st.mPlaneHalfConeAngle = DegreesToRadians(30.0f);
					st.mTwistMinAngle = DegreesToRadians(-15.0f);
					st.mTwistMaxAngle = DegreesToRadians(15.0f);
					Ref<Constraint> constraint = st.Create(*prev_body, body);
					ioContext.GetSystem()->AddConstraint(constraint);
				}

				prev_body = &body;
			}
		}
	}

	TEST_CASE("TestGridOfBoxesConstrained")
	{
		PhysicsTestContext c1(1.0f / 60.0f, 1, 1, 0);
		CreateGridOfBoxesConstrained(c1);

		PhysicsTestContext c2(1.0f / 60.0f, 1, 1, 15);
		CreateGridOfBoxesConstrained(c2);

		CompareSimulations(c1, c2, 5.0f);
	}
}
