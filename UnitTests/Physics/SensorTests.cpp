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
		CHECK(!listener.Contains(EType::Remove, dynamic.GetID(), sensor_id));
		listener.Clear();

		// After 3 more seconds we should have left the sensor at the bottom side
		c.Simulate(3.0f + c.GetDeltaTime());
		CHECK(listener.Contains(EType::Remove, dynamic.GetID(), sensor_id));
		CHECK_APPROX_EQUAL(dynamic.GetPosition(), Vec3(0, -1.5f - 3.0f * c.GetDeltaTime(), 0), 1.0e-4f);
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
		CHECK(!listener.Contains(EType::Remove, kinematic.GetID(), sensor_id));
		listener.Clear();

		// After 3 more seconds we should have left the sensor at the bottom side
		c.Simulate(3.0f + c.GetDeltaTime());
		CHECK(listener.Contains(EType::Remove, kinematic.GetID(), sensor_id));
		CHECK_APPROX_EQUAL(kinematic.GetPosition(), Vec3(0, -1.5f - 3.0f * c.GetDeltaTime(), 0), 1.0e-4f);
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

		// The dynamic object should not be part of an island
		CHECK(!sensor.IsActive());
		CHECK(dynamic.GetMotionProperties()->GetIslandIndexInternal() == Body::cInactiveIndex);

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

		// The dynamic object should be part of an island now
		CHECK(!sensor.IsActive());
		CHECK(dynamic.GetMotionProperties()->GetIslandIndexInternal() != Body::cInactiveIndex);

		// After a second the body should have gone to sleep and the contacts should have been removed
		c.Simulate(1.0f);
		CHECK(!dynamic.IsActive());
		CHECK(listener.Contains(EType::Remove, floor.GetID(), dynamic.GetID()));
		CHECK(listener.Contains(EType::Remove, sensor.GetID(), dynamic.GetID()));

		// The dynamic object should not be part of an island
		CHECK(!sensor.IsActive());
		CHECK(dynamic.GetMotionProperties()->GetIslandIndexInternal() == Body::cInactiveIndex);
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

		// The second step, the contact with the sensor should have persisted
		c.SimulateSingleStep();
		CHECK(listener.GetEntryCount() == 1);
		CHECK(listener.Contains(EType::Persist, sensor.GetID(), dynamic.GetID()));
		listener.Clear();

		// The sensor should still be in its own island
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

		// After another step we should have persisted the collision with the floor and sensor
		c.SimulateSingleStep();
		CHECK(listener.GetEntryCount() >= 2); // Depending on if we used the contact cache or not there will be validate callbacks too
		CHECK(listener.Contains(EType::Persist, floor.GetID(), dynamic.GetID()));
		CHECK(!listener.Contains(EType::Remove, floor.GetID(), dynamic.GetID()));
		CHECK(listener.Contains(EType::Persist, sensor.GetID(), dynamic.GetID()));
		CHECK(!listener.Contains(EType::Remove, sensor.GetID(), dynamic.GetID()));
		listener.Clear();

		// The same islands as the previous step should have been created
		CHECK(sensor.GetMotionProperties()->GetIslandIndexInternal() != Body::cInactiveIndex);
		CHECK(dynamic.GetMotionProperties()->GetIslandIndexInternal() != Body::cInactiveIndex);
		CHECK(sensor.GetMotionProperties()->GetIslandIndexInternal() != dynamic.GetMotionProperties()->GetIslandIndexInternal());

		// After a second the body should have gone to sleep and the contacts with the floor should have been removed, but not with the sensor
		c.Simulate(1.0f);
		CHECK(!dynamic.IsActive());
		CHECK(listener.Contains(EType::Remove, floor.GetID(), dynamic.GetID()));
		CHECK(!listener.Contains(EType::Remove, sensor.GetID(), dynamic.GetID()));
	}

	TEST_CASE("TestContactListenerMakesSensor")
	{
		PhysicsTestContext c;
		c.ZeroGravity();

		class SensorOverridingListener : public LoggingContactListener
		{
		public:
			virtual void		OnContactAdded(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings) override
			{
				LoggingContactListener::OnContactAdded(inBody1, inBody2, inManifold, ioSettings);

				JPH_ASSERT(ioSettings.mIsSensor == false);
				if (inBody1.GetID() == mBodyThatSeesSensorID || inBody2.GetID() == mBodyThatSeesSensorID)
					ioSettings.mIsSensor = true;
			}

			virtual void		OnContactPersisted(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings) override
			{
				LoggingContactListener::OnContactPersisted(inBody1, inBody2, inManifold, ioSettings);

				JPH_ASSERT(ioSettings.mIsSensor == false);
				if (inBody1.GetID() == mBodyThatSeesSensorID || inBody2.GetID() == mBodyThatSeesSensorID)
					ioSettings.mIsSensor = true;
			}

			BodyID				mBodyThatSeesSensorID;
		};

		// Register listener
		SensorOverridingListener listener;
		c.GetSystem()->SetContactListener(&listener);

		// Body that will appear as a sensor to one object and as static to another
		BodyID static_id = c.GetBodyInterface().CreateAndAddBody(BodyCreationSettings(new BoxShape(Vec3(5, 1, 5)), Vec3::sZero(), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);

		// Dynamic body moving down that will do a normal collision
		Body &dynamic1 = c.CreateBox(Vec3(-2, 2, 0), Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING, Vec3::sReplicate(0.5f));
		dynamic1.SetAllowSleeping(false);
		dynamic1.SetLinearVelocity(Vec3(0, -1, 0));

		// Dynamic body moving down that will only see the static object as a sensor
		Body &dynamic2 = c.CreateBox(Vec3(2, 2, 0), Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING, Vec3::sReplicate(0.5f));
		dynamic2.SetAllowSleeping(false);
		dynamic2.SetLinearVelocity(Vec3(0, -1, 0));
		listener.mBodyThatSeesSensorID = dynamic2.GetID();

		// After a single step the dynamic object should not have touched the sensor yet
		c.SimulateSingleStep();
		CHECK(listener.GetEntryCount() == 0);

		// After half a second both bodies should be touching the sensor
		c.Simulate(0.5f);
		CHECK(listener.Contains(EType::Add, dynamic1.GetID(), static_id));
		CHECK(listener.Contains(EType::Add, dynamic2.GetID(), static_id));
		listener.Clear();

		// The next step we require that the contact persists
		c.SimulateSingleStep();
		CHECK(listener.Contains(EType::Persist, dynamic1.GetID(), static_id));
		CHECK(!listener.Contains(EType::Remove, dynamic1.GetID(), static_id));
		CHECK(listener.Contains(EType::Persist, dynamic2.GetID(), static_id));
		CHECK(!listener.Contains(EType::Remove, dynamic2.GetID(), static_id));
		listener.Clear();

		// After 3 more seconds one body should be resting on the static body, the other should have fallen through
		c.Simulate(3.0f + c.GetDeltaTime());
		CHECK(listener.Contains(EType::Persist, dynamic1.GetID(), static_id));
		CHECK(!listener.Contains(EType::Remove, dynamic1.GetID(), static_id));
		CHECK(listener.Contains(EType::Remove, dynamic2.GetID(), static_id));
		CHECK_APPROX_EQUAL(dynamic1.GetPosition(), Vec3(-2, 1.5f, 0), 5.0e-3f);
		CHECK_APPROX_EQUAL(dynamic2.GetPosition(), Vec3(2, -1.5f - 3.0f * c.GetDeltaTime(), 0), 1.0e-4f);
	}
}
