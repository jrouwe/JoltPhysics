// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include "PhysicsTestContext.h"
#include "Layers.h"
#include "LoggingContactListener.h"
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>

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
		BodyCreationSettings sensor_settings(new BoxShape(Vec3::sReplicate(1)), RVec3::sZero(), Quat::sIdentity(), EMotionType::Static, Layers::SENSOR);
		sensor_settings.mIsSensor = true;
		BodyID sensor_id = c.GetBodyInterface().CreateAndAddBody(sensor_settings, EActivation::DontActivate);

		// Dynamic body moving downwards
		Body &dynamic = c.CreateBox(RVec3(0, 2, 0), Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING, Vec3::sReplicate(0.5f));
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
		CHECK_APPROX_EQUAL(dynamic.GetPosition(), RVec3(0, -1.5f - 3.0f * c.GetDeltaTime(), 0), 1.0e-4f);
	}

	TEST_CASE("TestKinematicVsSensor")
	{
		PhysicsTestContext c;

		// Register listener
		LoggingContactListener listener;
		c.GetSystem()->SetContactListener(&listener);

		// Sensor
		BodyCreationSettings sensor_settings(new BoxShape(Vec3::sReplicate(1)), RVec3::sZero(), Quat::sIdentity(), EMotionType::Static, Layers::SENSOR);
		sensor_settings.mIsSensor = true;
		BodyID sensor_id = c.GetBodyInterface().CreateAndAddBody(sensor_settings, EActivation::DontActivate);

		// Kinematic body moving downwards
		Body &kinematic = c.CreateBox(RVec3(0, 2, 0), Quat::sIdentity(), EMotionType::Kinematic, EMotionQuality::Discrete, Layers::MOVING, Vec3::sReplicate(0.5f));
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
		CHECK_APPROX_EQUAL(kinematic.GetPosition(), RVec3(0, -1.5f - 3.0f * c.GetDeltaTime(), 0), 1.0e-4f);
	}

	TEST_CASE("TestKinematicVsKinematicSensor")
	{
		// Same as TestKinematicVsSensor but with the sensor being an active kinematic body

		PhysicsTestContext c;

		// Register listener
		LoggingContactListener listener;
		c.GetSystem()->SetContactListener(&listener);

		// Kinematic sensor
		BodyCreationSettings sensor_settings(new BoxShape(Vec3::sReplicate(1)), RVec3::sZero(), Quat::sIdentity(), EMotionType::Kinematic, Layers::SENSOR);
		sensor_settings.mIsSensor = true;
		BodyID sensor_id = c.GetBodyInterface().CreateAndAddBody(sensor_settings, EActivation::Activate);

		// Kinematic body moving downwards
		Body &kinematic = c.CreateBox(RVec3(0, 2, 0), Quat::sIdentity(), EMotionType::Kinematic, EMotionQuality::Discrete, Layers::MOVING, Vec3::sReplicate(0.5f));
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
		CHECK_APPROX_EQUAL(kinematic.GetPosition(), RVec3(0, -1.5f - 3.0f * c.GetDeltaTime(), 0), 1.0e-4f);
	}

	TEST_CASE("TestKinematicVsKinematicSensorReversed")
	{
		// Same as TestKinematicVsKinematicSensor but with bodies created in reverse order (this matters for Body::sFindCollidingPairsCanCollide because MotionProperties::mIndexInActiveBodies is swapped between the bodies)

		PhysicsTestContext c;

		// Register listener
		LoggingContactListener listener;
		c.GetSystem()->SetContactListener(&listener);

		// Kinematic body moving downwards
		Body &kinematic = c.CreateBox(RVec3(0, 2, 0), Quat::sIdentity(), EMotionType::Kinematic, EMotionQuality::Discrete, Layers::MOVING, Vec3::sReplicate(0.5f));
		kinematic.SetLinearVelocity(Vec3(0, -1, 0));

		// Kinematic sensor
		BodyCreationSettings sensor_settings(new BoxShape(Vec3::sReplicate(1)), RVec3::sZero(), Quat::sIdentity(), EMotionType::Kinematic, Layers::SENSOR);
		sensor_settings.mIsSensor = true;
		BodyID sensor_id = c.GetBodyInterface().CreateAndAddBody(sensor_settings, EActivation::Activate);

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
		CHECK_APPROX_EQUAL(kinematic.GetPosition(), RVec3(0, -1.5f - 3.0f * c.GetDeltaTime(), 0), 1.0e-4f);
	}

	TEST_CASE("TestDynamicSleepingVsStaticSensor")
	{
		PhysicsTestContext c;

		// Register listener
		LoggingContactListener listener;
		c.GetSystem()->SetContactListener(&listener);

		// Sensor
		BodyCreationSettings sensor_settings(new BoxShape(Vec3::sReplicate(1)), RVec3::sZero(), Quat::sIdentity(), EMotionType::Static, Layers::SENSOR);
		sensor_settings.mIsSensor = true;
		Body &sensor = *c.GetBodyInterface().CreateBody(sensor_settings);
		c.GetBodyInterface().AddBody(sensor.GetID(), EActivation::DontActivate);

		// Floor
		Body &floor = c.CreateFloor();

		// Dynamic body on floor (make them penetrate)
		Body &dynamic = c.CreateBox(RVec3(0, 0.5f - c.GetSystem()->GetPhysicsSettings().mMaxPenetrationDistance, 0), Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING, Vec3::sReplicate(0.5f), EActivation::DontActivate);

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
		BodyCreationSettings sensor_settings(new BoxShape(Vec3::sReplicate(1)), RVec3::sZero(), Quat::sIdentity(), EMotionType::Kinematic, Layers::SENSOR);
		sensor_settings.mIsSensor = true;
		Body &sensor = *c.GetBodyInterface().CreateBody(sensor_settings);
		c.GetBodyInterface().AddBody(sensor.GetID(), EActivation::Activate);

		// Floor
		Body &floor = c.CreateFloor();

		// Dynamic body on floor (make them penetrate)
		Body &dynamic = c.CreateBox(RVec3(0, 0.5f - c.GetSystem()->GetPhysicsSettings().mMaxPenetrationDistance, 0), Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING, Vec3::sReplicate(0.5f), EActivation::DontActivate);

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

	TEST_CASE("TestSensorVsSensor")
	{
		for (int test = 0; test < 2; ++test)
		{
			bool sensor_detects_sensor = test == 1;

			PhysicsTestContext c;

			// Register listener
			LoggingContactListener listener;
			c.GetSystem()->SetContactListener(&listener);

			// Depending on the iteration we either place the sensor in the moving layer which means it will collide with other sensors
			// or we put it in the sensor layer which means it won't collide with other sensors
			ObjectLayer layer = sensor_detects_sensor? Layers::MOVING : Layers::SENSOR;

			// Sensor 1
			BodyCreationSettings sensor_settings(new BoxShape(Vec3::sReplicate(1)), RVec3::sZero(), Quat::sIdentity(), EMotionType::Static, layer);
			sensor_settings.mIsSensor = true;
			BodyID sensor_id1 = c.GetBodyInterface().CreateAndAddBody(sensor_settings, EActivation::DontActivate);

			// Sensor 2 moving downwards
			sensor_settings.mMotionType = EMotionType::Kinematic;
			sensor_settings.mPosition = RVec3(0, 3, 0);
			sensor_settings.mIsSensor = true;
			sensor_settings.mLinearVelocity = Vec3(0, -2, 0);
			BodyID sensor_id2 = c.GetBodyInterface().CreateAndAddBody(sensor_settings, EActivation::Activate);

			// After a single step the sensors should not touch yet
			c.SimulateSingleStep();
			CHECK(listener.GetEntryCount() == 0);

			// After half a second the sensors should be touching
			c.Simulate(0.5f);
			if (sensor_detects_sensor)
				CHECK(listener.Contains(EType::Add, sensor_id1, sensor_id2));
			else
				CHECK(listener.GetEntryCount() == 0);
			listener.Clear();

			// The next step we require that the contact persists
			c.SimulateSingleStep();
			if (sensor_detects_sensor)
			{
				CHECK(listener.Contains(EType::Persist, sensor_id1, sensor_id2));
				CHECK(!listener.Contains(EType::Remove, sensor_id1, sensor_id2));
			}
			else
				CHECK(listener.GetEntryCount() == 0);
			listener.Clear();

			// After 2 more seconds we should have left the sensor at the bottom side
			c.Simulate(2.0f + c.GetDeltaTime());
			if (sensor_detects_sensor)
				CHECK(listener.Contains(EType::Remove, sensor_id1, sensor_id2));
			else
				CHECK(listener.GetEntryCount() == 0);
			CHECK_APPROX_EQUAL(c.GetBodyInterface().GetPosition(sensor_id2), sensor_settings.mPosition + sensor_settings.mLinearVelocity * (2.5f + 3.0f * c.GetDeltaTime()), 1.0e-4f);
		}
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
		BodyID static_id = c.GetBodyInterface().CreateAndAddBody(BodyCreationSettings(new BoxShape(Vec3(5, 1, 5)), RVec3::sZero(), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);

		// Dynamic body moving down that will do a normal collision
		Body &dynamic1 = c.CreateBox(RVec3(-2, 2, 0), Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING, Vec3::sReplicate(0.5f));
		dynamic1.SetAllowSleeping(false);
		dynamic1.SetLinearVelocity(Vec3(0, -1, 0));

		// Dynamic body moving down that will only see the static object as a sensor
		Body &dynamic2 = c.CreateBox(RVec3(2, 2, 0), Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING, Vec3::sReplicate(0.5f));
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
		CHECK_APPROX_EQUAL(dynamic1.GetPosition(), RVec3(-2, 1.5f, 0), 5.0e-3f);
		CHECK_APPROX_EQUAL(dynamic2.GetPosition(), RVec3(2, -1.5f - 3.0f * c.GetDeltaTime(), 0), 1.0e-4f);
	}

	TEST_CASE("TestContactListenerMakesSensorCCD")
	{
		PhysicsTestContext c;
		c.ZeroGravity();

		const float cPenetrationSlop = c.GetSystem()->GetPhysicsSettings().mPenetrationSlop;

		class SensorOverridingListener : public LoggingContactListener
		{
		public:
			virtual void		OnContactAdded(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings) override
			{
				LoggingContactListener::OnContactAdded(inBody1, inBody2, inManifold, ioSettings);

				JPH_ASSERT(ioSettings.mIsSensor == false);
				if (inBody1.GetID() == mBodyThatBecomesSensor || inBody2.GetID() == mBodyThatBecomesSensor)
					ioSettings.mIsSensor = true;
			}

			virtual void		OnContactPersisted(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings) override
			{
				LoggingContactListener::OnContactPersisted(inBody1, inBody2, inManifold, ioSettings);

				JPH_ASSERT(ioSettings.mIsSensor == false);
				if (inBody1.GetID() == mBodyThatBecomesSensor || inBody2.GetID() == mBodyThatBecomesSensor)
					ioSettings.mIsSensor = true;
			}

			BodyID				mBodyThatBecomesSensor;
		};

		// Register listener
		SensorOverridingListener listener;
		c.GetSystem()->SetContactListener(&listener);

		// Body that blocks the path
		BodyID static_id = c.GetBodyInterface().CreateAndAddBody(BodyCreationSettings(new BoxShape(Vec3(0.1f, 10, 10)), RVec3::sZero(), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);

		// Dynamic body moving to the static object that will do a normal CCD collision
		RVec3 dynamic1_pos(-0.5f, 2, 0);
		Vec3 initial_velocity(500, 0, 0);
		Body &dynamic1 = c.CreateBox(dynamic1_pos, Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::LinearCast, Layers::MOVING, Vec3::sReplicate(0.1f));
		dynamic1.SetAllowSleeping(false);
		dynamic1.SetLinearVelocity(initial_velocity);
		dynamic1.SetRestitution(1.0f);

		// Dynamic body moving through the static object that will become a sensor an thus pass through
		RVec3 dynamic2_pos(-0.5f, -2, 0);
		Body &dynamic2 = c.CreateBox(dynamic2_pos, Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::LinearCast, Layers::MOVING, Vec3::sReplicate(0.1f));
		dynamic2.SetAllowSleeping(false);
		dynamic2.SetLinearVelocity(initial_velocity);
		dynamic2.SetRestitution(1.0f);
		listener.mBodyThatBecomesSensor = dynamic2.GetID();

		// After a single step the we should have contact added callbacks for both bodies
		c.SimulateSingleStep();
		CHECK(listener.Contains(EType::Add, dynamic1.GetID(), static_id));
		CHECK(listener.Contains(EType::Add, dynamic2.GetID(), static_id));
		listener.Clear();
		CHECK_APPROX_EQUAL(dynamic1.GetPosition(), dynamic1_pos + RVec3(0.3f + cPenetrationSlop, 0, 0), 1.0e-4f); // Dynamic 1 should have moved to the surface of the static body
		CHECK_APPROX_EQUAL(dynamic2.GetPosition(), dynamic2_pos + initial_velocity * c.GetDeltaTime(), 1.0e-4f); // Dynamic 2 should have passed through the static body because it became a sensor

		// The next step the sensor should have its contact removed and the CCD body should have its contact persisted because it starts penetrating
		c.SimulateSingleStep();
		CHECK(listener.Contains(EType::Persist, dynamic1.GetID(), static_id));
		CHECK(listener.Contains(EType::Remove, dynamic2.GetID(), static_id));
		listener.Clear();

		// The next step all contacts have been removed
		c.SimulateSingleStep();
		CHECK(listener.Contains(EType::Remove, dynamic1.GetID(), static_id));
		listener.Clear();
	}

	TEST_CASE("TestSensorVsSubShapes")
	{
		PhysicsTestContext c;
		BodyInterface &bi = c.GetBodyInterface();

		// Register listener
		LoggingContactListener listener;
		c.GetSystem()->SetContactListener(&listener);

		// Create sensor
		BodyCreationSettings sensor_settings(new BoxShape(Vec3::sReplicate(5.0f)), RVec3(0, 10, 0), Quat::sIdentity(), EMotionType::Static, Layers::SENSOR);
		sensor_settings.mIsSensor = true;
		BodyID sensor_id = bi.CreateAndAddBody(sensor_settings, EActivation::DontActivate);

		// We will be testing if we receive callbacks from the individual sub shapes
		enum class EUserData
		{
			Bottom,
			Middle,
			Top,
		};

		// Create compound with 3 sub shapes
		Ref<StaticCompoundShapeSettings> shape_settings = new StaticCompoundShapeSettings();
		Ref<BoxShapeSettings> shape1 = new BoxShapeSettings(Vec3::sReplicate(0.4f));
		shape1->mUserData = (uint64)EUserData::Bottom;
		Ref<BoxShapeSettings> shape2 = new BoxShapeSettings(Vec3::sReplicate(0.4f));
		shape2->mUserData = (uint64)EUserData::Middle;
		Ref<BoxShapeSettings> shape3 = new BoxShapeSettings(Vec3::sReplicate(0.4f));
		shape3->mUserData = (uint64)EUserData::Top;
		shape_settings->AddShape(Vec3(0, -1.0f, 0), Quat::sIdentity(), shape1);
		shape_settings->AddShape(Vec3(0, 0.0f, 0), Quat::sIdentity(), shape2);
		shape_settings->AddShape(Vec3(0, 1.0f, 0), Quat::sIdentity(), shape3);
		BodyCreationSettings compound_body_settings(shape_settings, RVec3(0, 20, 0), Quat::sIdentity(), JPH::EMotionType::Dynamic, Layers::MOVING);
		compound_body_settings.mUseManifoldReduction = false; // Turn off manifold reduction for this body so that we can get proper callbacks for individual sub shapes
		JPH::BodyID compound_body = bi.CreateAndAddBody(compound_body_settings, JPH::EActivation::Activate);

		// Simulate until the body passes the origin
		while (bi.GetPosition(compound_body).GetY() > 0.0f)
			c.SimulateSingleStep();

		// The expected events
		struct Expected
		{

			EType		mType;
			EUserData	mUserData;
		};
		const Expected expected[] = {
			{ EType::Add, EUserData::Bottom },
			{ EType::Add, EUserData::Middle },
			{ EType::Add, EUserData::Top },
			{ EType::Remove, EUserData::Bottom },
			{ EType::Remove, EUserData::Middle },
			{ EType::Remove, EUserData::Top }
		};
		const Expected *next = expected;
		const Expected *end = expected + size(expected);

		// Loop over events that we received
		for (size_t e = 0; e < listener.GetEntryCount(); ++e)
		{
			const LoggingContactListener::LogEntry &entry = listener.GetEntry(e);

			// Only interested in adds/removes
			if (entry.mType != EType::Add && entry.mType != EType::Remove)
				continue;

			// Check if we have more expected events
			if (next >= end)
			{
				CHECK(false);
				break;
			}

			// Check if it is of expected type
			CHECK(entry.mType == next->mType);
			CHECK(entry.mBody1 == sensor_id);
			CHECK(entry.mManifold.mSubShapeID1 == SubShapeID());
			CHECK(entry.mBody2 == compound_body);
			EUserData user_data = (EUserData)bi.GetShape(compound_body)->GetSubShapeUserData(entry.mManifold.mSubShapeID2);
			CHECK(user_data == next->mUserData);

			// Next expected event
			++next;
		}

		// Check all expected events received
		CHECK(next == end);
	}

	TEST_CASE("TestSensorVsStatic")
	{
		PhysicsTestContext c;

		// Register listener
		LoggingContactListener listener;
		c.GetSystem()->SetContactListener(&listener);

		// Static body 1
		Body &static1 = c.CreateSphere(RVec3::sZero(), 1.0f, EMotionType::Static, EMotionQuality::Discrete, Layers::NON_MOVING, EActivation::DontActivate);

		// Sensor
		BodyCreationSettings sensor_settings(new BoxShape(Vec3::sReplicate(1)), RVec3::sZero(), Quat::sIdentity(), EMotionType::Kinematic, Layers::MOVING); // Put in layer that collides with static
		sensor_settings.mIsSensor = true;
		Body &sensor = *c.GetBodyInterface().CreateBody(sensor_settings);
		BodyID sensor_id = sensor.GetID();
		c.GetBodyInterface().AddBody(sensor_id, EActivation::Activate);

		// Static body 2 (created after sensor to force higher body ID)
		Body &static2 = c.CreateSphere(RVec3::sZero(), 1.0f, EMotionType::Static, EMotionQuality::Discrete, Layers::NON_MOVING, EActivation::DontActivate);

		// After a step we should not detect the static bodies
		c.SimulateSingleStep();
		CHECK(listener.GetEntryCount() == 0);
		listener.Clear();

		// Start detecting static
		sensor.SetSensorDetectsStatic(true);

		// After a single step we should detect both static bodies
		c.SimulateSingleStep();
		CHECK(listener.GetEntryCount() == 4); // Should also contain validates
		CHECK(listener.Contains(EType::Add, static1.GetID(), sensor_id));
		CHECK(listener.Contains(EType::Add, static2.GetID(), sensor_id));
		listener.Clear();

		// Stop detecting static
		sensor.SetSensorDetectsStatic(false);

		// After a single step we should stop detecting both static bodies
		c.SimulateSingleStep();
		CHECK(listener.GetEntryCount() == 2);
		CHECK(listener.Contains(EType::Remove, static1.GetID(), sensor_id));
		CHECK(listener.Contains(EType::Remove, static2.GetID(), sensor_id));
		listener.Clear();
	}
}
