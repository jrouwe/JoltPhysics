// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include "PhysicsTestContext.h"
#include "Layers.h"
#include "LoggingContactListener.h"

TEST_SUITE("ContactListenerTests")
{
	// Gravity vector
	const Vec3 cGravity = Vec3(0.0f, -9.81f, 0.0f);

	using LogEntry = LoggingContactListener::LogEntry;
	using EType = LoggingContactListener::EType;

	// Let a sphere bounce on the floor with restition = 1
	TEST_CASE("TestContactListenerElastic")
	{
		PhysicsTestContext c(1.0f / 60.0f, 1, 1);

		const float cSimulationTime = 1.0f;
		const Vec3 cDistanceTraveled = c.PredictPosition(Vec3::sZero(), Vec3::sZero(), cGravity, cSimulationTime);
		const float cFloorHitEpsilon = 1.0e-4f; // Apply epsilon so that we're sure that the collision algorithm will find a collision
		const Vec3 cFloorHitPos(0.0f, 1.0f - cFloorHitEpsilon, 0.0f); // Sphere with radius 1 will hit floor when 1 above the floor
		const Vec3 cInitialPos = cFloorHitPos - cDistanceTraveled;
		const float cPenetrationSlop = c.GetSystem()->GetPhysicsSettings().mPenetrationSlop;

		// Register listener
		LoggingContactListener listener;
		c.GetSystem()->SetContactListener(&listener);

		// Create sphere
		Body &floor = c.CreateFloor();
		Body &body = c.CreateSphere(cInitialPos, 1.0f, EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING);
		body.SetRestitution(1.0f);
		CHECK(floor.GetID() < body.GetID());

		// Simulate until at floor
		c.Simulate(cSimulationTime);

		// Assert collision not yet processed
		CHECK(listener.GetEntryCount() == 0);

		// Simulate one more step to process the collision
		c.Simulate(c.GetDeltaTime());

		// We expect a validate and a contact point added message
		CHECK(listener.GetEntryCount() == 2);
		if (listener.GetEntryCount() == 2)
		{
			// Check validate callback
			const LogEntry &validate = listener.GetEntry(0);
			CHECK(validate.mType == EType::Validate);
			CHECK(validate.mBody1 == body.GetID()); // Dynamic body should always be the 1st
			CHECK(validate.mBody2 == floor.GetID());

			// Check add contact callback
			const LogEntry &add_contact = listener.GetEntry(1);
			CHECK(add_contact.mType == EType::Add);
			CHECK(add_contact.mBody1 == floor.GetID()); // Lowest ID should be first
			CHECK(add_contact.mManifold.mSubShapeID1.GetValue() == SubShapeID().GetValue()); // Floor doesn't have any sub shapes
			CHECK(add_contact.mBody2 == body.GetID()); // Highest ID should be second
			CHECK(add_contact.mManifold.mSubShapeID2.GetValue() == SubShapeID().GetValue()); // Sphere doesn't have any sub shapes
			CHECK_APPROX_EQUAL(add_contact.mManifold.mWorldSpaceNormal, Vec3::sAxisY()); // Normal should move body 2 out of collision
			CHECK(add_contact.mManifold.mWorldSpaceContactPointsOn1.size() == 1);
			CHECK(add_contact.mManifold.mWorldSpaceContactPointsOn2.size() == 1);
			CHECK(add_contact.mManifold.mWorldSpaceContactPointsOn1[0].IsClose(Vec3::sZero(), Square(cPenetrationSlop)));
			CHECK(add_contact.mManifold.mWorldSpaceContactPointsOn2[0].IsClose(Vec3::sZero(), Square(cPenetrationSlop)));
		}
		listener.Clear();

		// Simulate same time, with a fully elastic body we should reach the initial position again
		c.Simulate(cSimulationTime);

		// We should only have a remove contact point
		CHECK(listener.GetEntryCount() == 1);
		if (listener.GetEntryCount() == 1)
		{
			// Check remove contact callback
			const LogEntry &remove = listener.GetEntry(0);
			CHECK(remove.mType == EType::Remove);
			CHECK(remove.mBody1 == floor.GetID()); // Lowest ID should be first
			CHECK(remove.mBody2 == body.GetID()); // Highest ID should be second
		}		
	}

	// Let a sphere fall on the floor with restition = 0, then give it horizontal velocity, then take it away from the floor
	TEST_CASE("TestContactListenerInelastic")
	{
		PhysicsTestContext c(1.0f / 60.0f, 1, 1);

		const float cSimulationTime = 1.0f;
		const Vec3 cDistanceTraveled = c.PredictPosition(Vec3::sZero(), Vec3::sZero(), cGravity, cSimulationTime);
		const float cFloorHitEpsilon = 1.0e-4f; // Apply epsilon so that we're sure that the collision algorithm will find a collision
		const Vec3 cFloorHitPos(0.0f, 1.0f - cFloorHitEpsilon, 0.0f); // Sphere with radius 1 will hit floor when 1 above the floor
		const Vec3 cInitialPos = cFloorHitPos - cDistanceTraveled;
		const float cPenetrationSlop = c.GetSystem()->GetPhysicsSettings().mPenetrationSlop;

		// Register listener
		LoggingContactListener listener;
		c.GetSystem()->SetContactListener(&listener);

		// Create sphere
		Body &floor = c.CreateFloor();
		Body &body = c.CreateSphere(cInitialPos, 1.0f, EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING);
		body.SetRestitution(0.0f);
		body.SetAllowSleeping(false);
		CHECK(floor.GetID() < body.GetID());

		// Simulate until at floor
		c.Simulate(cSimulationTime);

		// Assert collision not yet processed
		CHECK(listener.GetEntryCount() == 0);

		// Simulate one more step to process the collision
		c.Simulate(c.GetDeltaTime());
		CHECK_APPROX_EQUAL(body.GetPosition(), cFloorHitPos, cPenetrationSlop);

		// We expect a validate and a contact point added message
		CHECK(listener.GetEntryCount() == 2);
		if (listener.GetEntryCount() == 2)
		{
			// Check validate callback
			const LogEntry &validate = listener.GetEntry(0);
			CHECK(validate.mType == EType::Validate);
			CHECK(validate.mBody1 == body.GetID()); // Dynamic body should always be the 1st
			CHECK(validate.mBody2 == floor.GetID());

			// Check add contact callback
			const LogEntry &add_contact = listener.GetEntry(1);
			CHECK(add_contact.mType == EType::Add);
			CHECK(add_contact.mBody1 == floor.GetID()); // Lowest ID first
			CHECK(add_contact.mManifold.mSubShapeID1.GetValue() == SubShapeID().GetValue()); // Floor doesn't have any sub shapes
			CHECK(add_contact.mBody2 == body.GetID()); // Highest ID second
			CHECK(add_contact.mManifold.mSubShapeID2.GetValue() == SubShapeID().GetValue()); // Sphere doesn't have any sub shapes
			CHECK_APPROX_EQUAL(add_contact.mManifold.mWorldSpaceNormal, Vec3::sAxisY()); // Normal should move body 2 out of collision
			CHECK(add_contact.mManifold.mWorldSpaceContactPointsOn1.size() == 1);
			CHECK(add_contact.mManifold.mWorldSpaceContactPointsOn2.size() == 1);
			CHECK(add_contact.mManifold.mWorldSpaceContactPointsOn1[0].IsClose(Vec3::sZero(), Square(cPenetrationSlop)));
			CHECK(add_contact.mManifold.mWorldSpaceContactPointsOn2[0].IsClose(Vec3::sZero(), Square(cPenetrationSlop)));
		}
		listener.Clear();

		// Simulate 10 steps
		c.Simulate(10 * c.GetDeltaTime());
		CHECK_APPROX_EQUAL(body.GetPosition(), cFloorHitPos, cPenetrationSlop);

		// We're not moving, we should have persisted contacts only
		CHECK(listener.GetEntryCount() == 10);
		for (size_t i = 0; i < listener.GetEntryCount(); ++i)
		{
			// Check persist callback
			const LogEntry &persist_contact = listener.GetEntry(i);
			CHECK(persist_contact.mType == EType::Persist);
			CHECK(persist_contact.mBody1 == floor.GetID()); // Lowest ID first
			CHECK(persist_contact.mManifold.mSubShapeID1.GetValue() == SubShapeID().GetValue()); // Floor doesn't have any sub shapes
			CHECK(persist_contact.mBody2 == body.GetID()); // Highest ID second
			CHECK(persist_contact.mManifold.mSubShapeID2.GetValue() == SubShapeID().GetValue()); // Sphere doesn't have any sub shapes
			CHECK_APPROX_EQUAL(persist_contact.mManifold.mWorldSpaceNormal, Vec3::sAxisY()); // Normal should move body 2 out of collision
			CHECK(persist_contact.mManifold.mWorldSpaceContactPointsOn1.size() == 1);
			CHECK(persist_contact.mManifold.mWorldSpaceContactPointsOn2.size() == 1);
			CHECK(persist_contact.mManifold.mWorldSpaceContactPointsOn1[0].IsClose(Vec3::sZero(), Square(cPenetrationSlop)));
			CHECK(persist_contact.mManifold.mWorldSpaceContactPointsOn2[0].IsClose(Vec3::sZero(), Square(cPenetrationSlop)));
		}
		listener.Clear();

		// Make the body able to go to sleep
		body.SetAllowSleeping(true);

		// Let the body go to sleep
		c.Simulate(1.0f);
		CHECK_APPROX_EQUAL(body.GetPosition(), cFloorHitPos, cPenetrationSlop);

		// Check it went to sleep and that we received a contact removal callback
		CHECK(!body.IsActive());
		CHECK(listener.GetEntryCount() > 0);
		for (size_t i = 0; i < listener.GetEntryCount(); ++i)
		{
			// Check persist / removed callbacks
			const LogEntry &entry = listener.GetEntry(i);
			CHECK(entry.mBody1 == floor.GetID());
			CHECK(entry.mBody2 == body.GetID());
			CHECK(entry.mType == ((i == listener.GetEntryCount() - 1)? EType::Remove : EType::Persist)); // The last entry should remove the contact as the body went to sleep
		}
		listener.Clear();

		// Wake the body up again
		c.GetBodyInterface().ActivateBody(body.GetID());
		CHECK(body.IsActive());

		// Simulate 1 time step to detect the collision with the floor again
		c.SimulateSingleStep();

		// Check that the contact got readded
		CHECK(listener.GetEntryCount() == 2);
		CHECK(listener.Contains(EType::Validate, floor.GetID(), body.GetID()));
		CHECK(listener.Contains(EType::Add, floor.GetID(), body.GetID()));
		listener.Clear();

		// Prevent body from going to sleep again
		body.SetAllowSleeping(false);

		// Make the sphere move horizontal
		body.SetLinearVelocity(Vec3::sAxisX());

		// Simulate 10 steps
		c.Simulate(10 * c.GetDeltaTime());

		// We should have 10 persisted contacts events
		int validate = 0;
		int persisted = 0;
		for (size_t i = 0; i < listener.GetEntryCount(); ++i)
		{
			const LogEntry &entry = listener.GetEntry(i);
			switch (entry.mType)
			{
			case EType::Validate:
				++validate;
				break;

			case EType::Persist:
				// Check persist callback
				CHECK(entry.mBody1 == floor.GetID()); // Lowest ID first
				CHECK(entry.mManifold.mSubShapeID1.GetValue() == SubShapeID().GetValue()); // Floor doesn't have any sub shapes
				CHECK(entry.mBody2 == body.GetID()); // Highest ID second
				CHECK(entry.mManifold.mSubShapeID2.GetValue() == SubShapeID().GetValue()); // Sphere doesn't have any sub shapes
				CHECK_APPROX_EQUAL(entry.mManifold.mWorldSpaceNormal, Vec3::sAxisY()); // Normal should move body 2 out of collision
				CHECK(entry.mManifold.mWorldSpaceContactPointsOn1.size() == 1);
				CHECK(entry.mManifold.mWorldSpaceContactPointsOn2.size() == 1);
				CHECK(abs(entry.mManifold.mWorldSpaceContactPointsOn1[0].GetY()) < cPenetrationSlop);
				CHECK(abs(entry.mManifold.mWorldSpaceContactPointsOn2[0].GetY()) < cPenetrationSlop);
				++persisted;
				break;

			case EType::Add:
			case EType::Remove:
			default:
				CHECK(false); // Unexpected event
			}
		}
		CHECK(validate <= 10); // We may receive extra validate callbacks when the object is moving
		CHECK(persisted == 10);
		listener.Clear();

		// Move the sphere away from the floor
		c.GetBodyInterface().SetPosition(body.GetID(), cInitialPos, EActivation::Activate);

		// Simulate 10 steps
		c.Simulate(10 * c.GetDeltaTime());

		// We should only have a remove contact point
		CHECK(listener.GetEntryCount() == 1);
		if (listener.GetEntryCount() == 1)
		{
			// Check remove contact callback
			const LogEntry &remove = listener.GetEntry(0);
			CHECK(remove.mType == EType::Remove);
			CHECK(remove.mBody1 == floor.GetID()); // Lowest ID first
			CHECK(remove.mBody2 == body.GetID()); // Highest ID second
		}		
	}
}
