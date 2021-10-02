// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/General/SensorTest.h>
#include <Physics/Collision/Shape/BoxShape.h>
#include <Physics/Collision/Shape/SphereShape.h>
#include <Physics/Body/BodyCreationSettings.h>
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

	// Sensor
	BodyCreationSettings sensor_settings(new SphereShape(10.0f), Vec3(0, 10, 0), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);
	sensor_settings.mIsSensor = true;
	Body &sensor = *mBodyInterface->CreateBody(sensor_settings);
	JPH_IF_DEBUG(sensor.SetDebugName("Sensor");)
	mSensorID = sensor.GetID();
	mBodyInterface->AddBody(mSensorID, EActivation::DontActivate);

	// Dynamic bodies
	for (int i = 0; i < 10; ++i)
	{
		Body &body = *mBodyInterface->CreateBody(BodyCreationSettings(new BoxShape(Vec3(0.1f, 0.5f, 0.2f)), Vec3(-15.0f + i * 3.0f, 25, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
		JPH_IF_DEBUG(body.SetDebugName("Body " + ConvertToString(i)));
		mBodyInterface->AddBody(body.GetID(), EActivation::Activate);
	}

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
	mRagdoll = ragdoll_settings->CreateRagdoll(1, nullptr, mPhysicsSystem);
	mRagdoll->SetPose(ragdoll_pose);
	mRagdoll->AddToPhysicsSystem(EActivation::Activate);

	// Create kinematic body
	BodyCreationSettings kinematic_settings(new BoxShape(Vec3(0.25f, 0.5f, 1.0f)), Vec3(-15, 10, 0), Quat::sIdentity(), EMotionType::Kinematic, Layers::MOVING);
	Body &kinematic = *mBodyInterface->CreateBody(kinematic_settings);
	mKinematicBodyID = kinematic.GetID();
	mBodyInterface->AddBody(kinematic.GetID(), EActivation::Activate);
}

void SensorTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	// Update time
	mTime += inParams.mDeltaTime;

	// Move kinematic body
	Vec3 kinematic_pos = Vec3(-15.0f * cos(mTime), 10, 0);
	mBodyInterface->MoveKinematic(mKinematicBodyID, kinematic_pos, Quat::sIdentity(), inParams.mDeltaTime);

	// Draw if the kinematic body is in the sensor
	if (mKinematicBodyInSensor)
		mDebugRenderer->DrawWireBox(mBodyInterface->GetTransformedShape(mKinematicBodyID).GetWorldSpaceBounds(), Color::sRed);

	// Apply forces to dynamic bodies in sensor
	lock_guard lock(mMutex);

	Vec3 center(0, 10, 0);
	float centrifugal_force = 10.0f;
	Vec3 gravity = mPhysicsSystem->GetGravity();
	
	for (const BodyAndCount &body_and_count : mBodiesInSensor)
	{
		BodyLockWrite body_lock(mPhysicsSystem->GetBodyLockInterface(), body_and_count.mBodyID);
		if (body_lock.Succeeded())
		{
			Body &body = body_lock.GetBody();

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
	// Check which body is the sensor
	BodyID body_id;
	if (inBody1.GetID() == mSensorID)
		body_id = inBody2.GetID();
	else if (inBody2.GetID() == mSensorID)
		body_id = inBody1.GetID();
	else
		return;

	lock_guard lock(mMutex);

	if (body_id == mKinematicBodyID)
	{
		JPH_ASSERT(!mKinematicBodyInSensor);
		mKinematicBodyInSensor = true;
	}
	else
	{
		// Add to list and make sure that the list remains sorted for determinism (contacts can be added from multiple threads)
		BodyAndCount body_and_count { body_id, 1 };
		BodiesInSensor::iterator b = lower_bound(mBodiesInSensor.begin(), mBodiesInSensor.end(), body_and_count);
		if (b != mBodiesInSensor.end() && b->mBodyID == body_id)
		{
			// This is the right body, increment reference
			b->mCount++;
			return;
		}
		mBodiesInSensor.insert(b, body_and_count);
	}
}

void SensorTest::OnContactRemoved(const SubShapeIDPair &inSubShapePair)
{
	// Check which body is the sensor
	BodyID body_id;
	if (inSubShapePair.GetBody1ID() == mSensorID)
		body_id = inSubShapePair.GetBody2ID();
	else if (inSubShapePair.GetBody2ID() == mSensorID)
		body_id = inSubShapePair.GetBody1ID();
	else
		return;

	lock_guard lock(mMutex);

	if (body_id == mKinematicBodyID)
	{
		JPH_ASSERT(mKinematicBodyInSensor);
		mKinematicBodyInSensor = false;
	}
	else
	{
		// Remove from list
		BodyAndCount body_and_count { body_id, 1 };
		BodiesInSensor::iterator b = lower_bound(mBodiesInSensor.begin(), mBodiesInSensor.end(), body_and_count);
		if (b != mBodiesInSensor.end() && b->mBodyID == body_id)
		{
			// This is the right body, increment reference
			JPH_ASSERT(b->mCount > 0);
			b->mCount--;

			// When last reference remove from the list
			if (b->mCount == 0)
				mBodiesInSensor.erase(b);
			return;
		}
		JPH_ASSERT(false, "Body pair not found");
	}
}

void SensorTest::SaveState(StateRecorder &inStream) const
{
	inStream.Write(mTime);
	inStream.Write(mBodiesInSensor);
	inStream.Write(mKinematicBodyInSensor);
}

void SensorTest::RestoreState(StateRecorder &inStream)
{
	inStream.Read(mTime);
	inStream.Read(mBodiesInSensor);
	inStream.Read(mKinematicBodyInSensor);
}
