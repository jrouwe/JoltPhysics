// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include "PhysicsTestContext.h"
#include "Layers.h"
#include "LoggingContactListener.h"
#include <Jolt/Physics/Collision/Shape/BoxShape.h>

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

	TEST_CASE("TestDynamicSleepingVsStaticSensor")
	{
		PhysicsTestContext c;

		// Register listener
		LoggingContactListener listener;
		c.GetSystem()->SetContactListener(&listener);

		// Sensor
		BodyCreationSettings sensor_settings(new BoxShape(Vec3::sReplicate(1)), Vec3::sZero(), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);
		sensor_settings.mIsSensor = true;
		Body &sensor = *c.GetBodyInterface().CreateBody(sensor_settings);
		c.GetBodyInterface().AddBody(sensor.GetID(), EActivation::DontActivate);

		// Floor
		Body &floor = c.CreateFloor();

		// Dynamic body on floor (make them penetrate)
		Body &dynamic = c.CreateBox(Vec3(0, 0.5f - c.GetSystem()->GetPhysicsSettings().mMaxPenetrationDistance, 0), Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING, Vec3::sReplicate(0.5f), EActivation::DontActivate);

		// After a single step (because the object is sleeping) there should not be a contact
		c.SimulateSingleStep();
		CHECK(listener.GetEntryCount() == 0);

		// Activate the body
		c.GetBodyInterface().ActivateBody(dynamic.GetID());

		// After a single step we should have detected the collision with the floor and the sensor
		c.SimulateSingleStep();
		CHECK(listener.GetEntryCount() == 4);
		CHECK(listener.Contains(EType::Validate, floor.GetID(), dynamic.GetID()));
		CHECK(listener.Contains(EType::Add, floor.GetID(), dynamic.GetID()));
		CHECK(listener.Contains(EType::Validate, sensor.GetID(), dynamic.GetID()));
		CHECK(listener.Contains(EType::Add, sensor.GetID(), dynamic.GetID()));
		listener.Clear();

		// After a second the body should have gone to sleep and the contacts should have been removed
		c.Simulate(1.0f);
		CHECK(!dynamic.IsActive());
		CHECK(listener.Contains(EType::Remove, floor.GetID(), dynamic.GetID()));
		CHECK(listener.Contains(EType::Remove, sensor.GetID(), dynamic.GetID()));
	}

	TEST_CASE("TestDynamicSleepingVsKinematicSensor")
	{
		PhysicsTestContext c;

		// Register listener
		LoggingContactListener listener;
		c.GetSystem()->SetContactListener(&listener);

		// Kinematic sensor that is active (so will keep detecting contacts with sleeping bodies)
		BodyCreationSettings sensor_settings(new BoxShape(Vec3::sReplicate(1)), Vec3::sZero(), Quat::sIdentity(), EMotionType::Kinematic, Layers::NON_MOVING);
		sensor_settings.mIsSensor = true;
		Body &sensor = *c.GetBodyInterface().CreateBody(sensor_settings);
		c.GetBodyInterface().AddBody(sensor.GetID(), EActivation::Activate);

		// Floor
		Body &floor = c.CreateFloor();

		// Dynamic body on floor (make them penetrate)
		Body &dynamic = c.CreateBox(Vec3(0, 0.5f - c.GetSystem()->GetPhysicsSettings().mMaxPenetrationDistance, 0), Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING, Vec3::sReplicate(0.5f), EActivation::DontActivate);

		// After a single step, there should be a contact with the sensor only (the sensor is active)
		c.SimulateSingleStep();
		CHECK(listener.GetEntryCount() == 2);
		CHECK(listener.Contains(EType::Validate, sensor.GetID(), dynamic.GetID()));
		CHECK(listener.Contains(EType::Add, sensor.GetID(), dynamic.GetID()));
		listener.Clear();

		// The sensor should be in its own island
		CHECK(sensor.GetMotionProperties()->GetIslandIndexInternal() != Body::cInactiveIndex);
		CHECK(dynamic.GetMotionProperties()->GetIslandIndexInternal() == Body::cInactiveIndex);

		// Activate the body
		c.GetBodyInterface().ActivateBody(dynamic.GetID());

		// After a single step we should have detected collision with the floor and the collision with the sensor should have persisted
		c.SimulateSingleStep();
		CHECK(listener.GetEntryCount() == 3);
		CHECK(listener.Contains(EType::Validate, floor.GetID(), dynamic.GetID()));
		CHECK(listener.Contains(EType::Add, floor.GetID(), dynamic.GetID()));
		CHECK(listener.Contains(EType::Persist, sensor.GetID(), dynamic.GetID()));
		listener.Clear();

		// The sensor should not be part of the same island as the dynamic body (they won't interact, so this is not needed)
		CHECK(sensor.GetMotionProperties()->GetIslandIndexInternal() != Body::cInactiveIndex);
		CHECK(dynamic.GetMotionProperties()->GetIslandIndexInternal() != Body::cInactiveIndex);
		CHECK(sensor.GetMotionProperties()->GetIslandIndexInternal() != dynamic.GetMotionProperties()->GetIslandIndexInternal());

		// After a second the body should have gone to sleep and the contacts with the floor should have been removed, but not with the sensor
		c.Simulate(1.0f);
		CHECK(!dynamic.IsActive());
		CHECK(listener.Contains(EType::Remove, floor.GetID(), dynamic.GetID()));
		CHECK(!listener.Contains(EType::Remove, sensor.GetID(), dynamic.GetID()));
	}
}
