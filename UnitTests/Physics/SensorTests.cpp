// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include "PhysicsTestContext.h"
#include "Layers.h"
#include "LoggingContactListener.h"
#include <Physics/Collision/Shape/BoxShape.h>

TEST_SUITE("SensorTests")
{
	using LogEntry = LoggingContactListener::LogEntry;
	using EType = LoggingContactListener::EType;

	TEST_CASE("TestDynamicVsSensor")
	{
		PhysicsTestContext c;
		c.ZeroGravity();

		// Register listener
		LoggingContactListener listener;
		c.GetSystem()->SetContactListener(&listener);

		// Sensor
		BodyCreationSettings sensor_settings(new BoxShape(Vec3::sReplicate(1)), Vec3::sZero(), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);
		sensor_settings.mIsSensor = true;
		BodyID sensor_id = c.GetBodyInterface().CreateAndAddBody(sensor_settings, EActivation::DontActivate);

		// Dynamic body moving downwards
		Body &dynamic = c.CreateBox(Vec3(0, 2, 0), Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING, Vec3::sReplicate(0.5f));
		dynamic.SetLinearVelocity(Vec3(0, -1, 0));

		// After a single step the dynamic object should not have touched the sensor yet
		c.SimulateSingleStep();
		CHECK(listener.GetEntryCount() == 0);

		// After half a second we should be touching the sensor
		c.Simulate(0.5f);
		CHECK(listener.Contains(EType::Add, dynamic.GetID(), sensor_id));
		listener.Clear();

		// The next step we require that the contact persists
		c.SimulateSingleStep();
		CHECK(listener.Contains(EType::Persist, dynamic.GetID(), sensor_id));
		listener.Clear();

		// After 3 more seconds we should have left the sensor at the bottom side
		c.Simulate(3.0f + c.GetDeltaTime());
		CHECK(listener.Contains(EType::Remove, dynamic.GetID(), sensor_id));
	}

	TEST_CASE("TestKinematicVsSensor")
	{
		PhysicsTestContext c;

		// Register listener
		LoggingContactListener listener;
		c.GetSystem()->SetContactListener(&listener);

		// Sensor
		BodyCreationSettings sensor_settings(new BoxShape(Vec3::sReplicate(1)), Vec3::sZero(), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);
		sensor_settings.mIsSensor = true;
		BodyID sensor_id = c.GetBodyInterface().CreateAndAddBody(sensor_settings, EActivation::DontActivate);

		// Kinematic body moving downwards
		Body &kinematic = c.CreateBox(Vec3(0, 2, 0), Quat::sIdentity(), EMotionType::Kinematic, EMotionQuality::Discrete, Layers::MOVING, Vec3::sReplicate(0.5f));
		kinematic.SetLinearVelocity(Vec3(0, -1, 0));

		// After a single step the kinematic object should not have touched the sensor yet
		c.SimulateSingleStep();
		CHECK(listener.GetEntryCount() == 0);

		// After half a second we should be touching the sensor
		c.Simulate(0.5f);
		CHECK(listener.Contains(EType::Add, kinematic.GetID(), sensor_id));
		listener.Clear();

		// The next step we require that the contact persists
		c.SimulateSingleStep();
		CHECK(listener.Contains(EType::Persist, kinematic.GetID(), sensor_id));
		listener.Clear();

		// After 3 more seconds we should have left the sensor at the bottom side
		c.Simulate(3.0f + c.GetDeltaTime());
		CHECK(listener.Contains(EType::Remove, kinematic.GetID(), sensor_id));
	}
}