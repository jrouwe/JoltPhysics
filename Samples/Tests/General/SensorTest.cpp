// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/General/SensorTest.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Utils/RagdollLoader.h>
#include <Utils/Log.h>
#include <Layers.h>
#include <Renderer/DebugRendererImp.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(SensorTest) 
{ 
	JPH_ADD_BASE_CLASS(SensorTest, Test) 
}

SensorTest::~SensorTest()
{
	// Destroy the ragdoll
	mRagdoll->RemoveFromPhysicsSystem();
	mRagdoll = nullptr;
}

void SensorTest::Initialize()
{
	// Floor
	CreateFloor();

	{
		// A static sensor that attrects dynamic bodies that enter its area
		BodyCreationSettings sensor_settings(new SphereShape(10.0f), Vec3(0, 10, 0), Quat::sIdentity(), EMotionType::Static, Layers::SENSOR);
		sensor_settings.mIsSensor = true;
		mSensorID[StaticAttractor] = mBodyInterface->CreateAndAddBody(sensor_settings, EActivation::DontActivate);
	}
	
	{
		// A static sensor that only detects active bodies
		BodyCreationSettings sensor_settings(new BoxShape(Vec3::sReplicate(5.0f)), Vec3(-10, 5, 0), Quat::sIdentity(), EMotionType::Static, Layers::SENSOR);
		sensor_settings.mIsSensor = true;
		mSensorID[StaticSensor] = mBodyInterface->CreateAndAddBody(sensor_settings, EActivation::DontActivate);
	}

	{
		// A kinematic sensor that also detects sleeping bodies
		BodyCreationSettings sensor_settings(new BoxShape(Vec3::sReplicate(5.0f)), Vec3(10, 5, 0), Quat::sIdentity(), EMotionType::Kinematic, Layers::SENSOR);
		sensor_settings.mIsSensor = true;
		mSensorID[KinematicSensor] = mBodyInterface->CreateAndAddBody(sensor_settings, EActivation::Activate);
	}

	// Dynamic bodies
	for (int i = 0; i < 10; ++i)
		mBodyInterface->CreateAndAddBody(BodyCreationSettings(new BoxShape(Vec3(0.1f, 0.5f, 0.2f)), Vec3(-15.0f + i * 3.0f, 25, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING), EActivation::Activate);

	// Load ragdoll
	Ref<RagdollSettings> ragdoll_settings = RagdollLoader::sLoad("Assets/Human.tof", EMotionType::Dynamic);
	if (ragdoll_settings == nullptr)
		FatalError("Could not load ragdoll");

	// Load animation
	Ref<SkeletalAnimation> animation;
	if (!ObjectStreamIn::sReadObject("Assets/Human/Dead_Pose1.tof", animation))
		FatalError("Could not open animation");

	// Create pose
	SkeletonPose ragdoll_pose;
	ragdoll_pose.SetSkeleton(ragdoll_settings->GetSkeleton());
	animation->Sample(0.0f, ragdoll_pose);
	SkeletonPose::JointState &root = ragdoll_pose.GetJoint(0);
	root.mTranslation = Vec3(0, 30, 0);
	ragdoll_pose.CalculateJointMatrices();

	// Create ragdoll
	mRagdoll = ragdoll_settings->CreateRagdoll(1, 0, mPhysicsSystem);
	mRagdoll->SetPose(ragdoll_pose);
	mRagdoll->AddToPhysicsSystem(EActivation::Activate);

	// Create kinematic body
	BodyCreationSettings kinematic_settings(new BoxShape(Vec3(0.25f, 0.5f, 1.0f)), Vec3(-20, 10, 0), Quat::sIdentity(), EMotionType::Kinematic, Layers::MOVING);
	Body &kinematic = *mBodyInterface->CreateBody(kinematic_settings);
	mKinematicBodyID = kinematic.GetID();
	mBodyInterface->AddBody(kinematic.GetID(), EActivation::Activate);
}

void SensorTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	// Update time
	mTime += inParams.mDeltaTime;

	// Move kinematic body
	Vec3 kinematic_pos = Vec3(-20.0f * cos(mTime), 10, 0);
	mBodyInterface->MoveKinematic(mKinematicBodyID, kinematic_pos, Quat::sIdentity(), inParams.mDeltaTime);

	// Draw if body is in sensor
	Color sensor_color[] = { Color::sRed, Color::sGreen, Color::sBlue };
	for (int sensor = 0; sensor < NumSensors; ++sensor)
		for (const BodyAndCount &body_and_count : mBodiesInSensor[sensor])
		{
			AABox bounds = mBodyInterface->GetTransformedShape(body_and_count.mBodyID).GetWorldSpaceBounds();
			bounds.ExpandBy(Vec3::sReplicate(0.01f * sensor));
			mDebugRenderer->DrawWireBox(bounds, sensor_color[sensor]);
		}

	// Apply forces to dynamic bodies in sensor
	lock_guard lock(mMutex);

	Vec3 center(0, 10, 0);
	float centrifugal_force = 10.0f;
	Vec3 gravity = mPhysicsSystem->GetGravity();
	
	for (const BodyAndCount &body_and_count : mBodiesInSensor[StaticAttractor])
	{
		BodyLockWrite body_lock(mPhysicsSystem->GetBodyLockInterface(), body_and_count.mBodyID);
		if (body_lock.Succeeded())
		{
			Body &body = body_lock.GetBody();
			if (body.IsKinematic())
				continue;

			// Calculate centrifugal acceleration
			Vec3 acceleration = center - body.GetPosition();
			float length = acceleration.Length();
			if (length > 0.0f)
				acceleration *= centrifugal_force / length;
			else
				acceleration = Vec3::sZero();

			// Draw acceleration
			mDebugRenderer->DrawArrow(body.GetPosition(), body.GetPosition() + acceleration, Color::sGreen, 0.1f);

			// Cancel gravity
			acceleration -= gravity;

			// Apply as force
			body.AddForce(acceleration / body.GetMotionProperties()->GetInverseMass());
		}
	}
}

void SensorTest::OnContactAdded(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings)
{
	for (int sensor = 0; sensor < NumSensors; ++sensor)
	{
		BodyID sensor_id = mSensorID[sensor];

		// Check which body is the sensor
		BodyID body_id;
		if (inBody1.GetID() == sensor_id)
			body_id = inBody2.GetID();
		else if (inBody2.GetID() == sensor_id)
			body_id = inBody1.GetID();
		else
			continue;

		lock_guard lock(mMutex);

		// Add to list and make sure that the list remains sorted for determinism (contacts can be added from multiple threads)
		BodyAndCount body_and_count { body_id, 1 };
		BodiesInSensor &bodies_in_sensor = mBodiesInSensor[sensor];
		BodiesInSensor::iterator b = lower_bound(bodies_in_sensor.begin(), bodies_in_sensor.end(), body_and_count);
		if (b != bodies_in_sensor.end() && b->mBodyID == body_id)
		{
			// This is the right body, increment reference
			b->mCount++;
			return;
		}
		bodies_in_sensor.insert(b, body_and_count);
	}
}

void SensorTest::OnContactRemoved(const SubShapeIDPair &inSubShapePair)
{
	for (int sensor = 0; sensor < NumSensors; ++sensor)
	{
		BodyID sensor_id = mSensorID[sensor];

		// Check which body is the sensor
		BodyID body_id;
		if (inSubShapePair.GetBody1ID() == sensor_id)
			body_id = inSubShapePair.GetBody2ID();
		else if (inSubShapePair.GetBody2ID() == sensor_id)
			body_id = inSubShapePair.GetBody1ID();
		else
			continue;

		lock_guard lock(mMutex);

		// Remove from list
		BodyAndCount body_and_count { body_id, 1 };
		BodiesInSensor &bodies_in_sensor = mBodiesInSensor[sensor];
		BodiesInSensor::iterator b = lower_bound(bodies_in_sensor.begin(), bodies_in_sensor.end(), body_and_count);
		if (b != bodies_in_sensor.end() && b->mBodyID == body_id)
		{
			// This is the right body, increment reference
			JPH_ASSERT(b->mCount > 0);
			b->mCount--;

			// When last reference remove from the list
			if (b->mCount == 0)
				bodies_in_sensor.erase(b);
			return;
		}
		JPH_ASSERT(false, "Body pair not found");
	}
}

void SensorTest::SaveState(StateRecorder &inStream) const
{
	inStream.Write(mTime);
	for (const BodiesInSensor &b : mBodiesInSensor)
		inStream.Write(b);
}

void SensorTest::RestoreState(StateRecorder &inStream)
{
	inStream.Read(mTime);
	for (BodiesInSensor &b : mBodiesInSensor)
		inStream.Read(b);
}
