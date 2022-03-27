// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/General/MultithreadedTest.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>
#include <Jolt/Skeleton/Skeleton.h>
#include <Jolt/Skeleton/SkeletalAnimation.h>
#include <Jolt/Skeleton/SkeletonPose.h>
#include <Jolt/Physics/Ragdoll/Ragdoll.h>
#include <Utils/RagdollLoader.h>
#include <Utils/Log.h>
#include <Renderer/DebugRendererImp.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(MultithreadedTest) 
{ 
	JPH_ADD_BASE_CLASS(MultithreadedTest, Test) 
}

MultithreadedTest::~MultithreadedTest()
{
	// Quit the threads
	mIsQuitting = true;
	mBoxSpawnerThread.join();
	mRagdollSpawnerThread.join();
	mCasterThread.join();
}

void MultithreadedTest::Initialize() 
{
	// Floor
	CreateFloor();
		
	// Start threads
	mBoxSpawnerThread = thread([this]() { BoxSpawner(); });
	mRagdollSpawnerThread = thread([this]() { RagdollSpawner(); });
	mCasterThread = thread([this]() { CasterMain(); });
}

void MultithreadedTest::Execute(default_random_engine &ioRandom, const char *inName, function<void()> inFunction)
{
	uniform_real_distribution<float> chance(0, 1);
	if (chance(ioRandom) < 0.5f)
	{
		// Execute as a job and wait for it
		JobHandle handle = mJobSystem->CreateJob(inName, Color::sGreen, inFunction);
		while (!handle.IsDone())
			this_thread::sleep_for(1ms);
	}
	else
	{
		// Execute in this separate thread (not part of the job system)
		JPH_PROFILE(inName);
		inFunction();
	}
}

void MultithreadedTest::BoxSpawner()
{
	JPH_PROFILE_THREAD_START("BoxSpawner");

#ifdef _DEBUG
	const int cMaxObjects = 100;
#else
	const int cMaxObjects = 1000;
#endif

	default_random_engine random;

	vector<BodyID> bodies;

	while (!mIsQuitting)
	{
		// Ensure there are enough objects at all times
		if (bodies.size() < cMaxObjects)
		{
			BodyID body_id;
			Execute(random, "AddBody", [this, &body_id, &random]() {
				uniform_real_distribution<float> from_y(0, 10);
				uniform_real_distribution<float> from_xz(-5, 5);
				Vec3 position = Vec3(from_xz(random), 1.0f + from_y(random), from_xz(random));
				Quat orientation = Quat::sRandom(random);
				Vec3 velocity = Vec3::sRandom(random);
				Body &body = *mBodyInterface->CreateBody(BodyCreationSettings(new BoxShape(Vec3(0.5f, 0.2f, 0.3f)), position, orientation, EMotionType::Dynamic, Layers::MOVING));
				body.SetLinearVelocity(velocity);
				body_id = body.GetID();
				mBodyInterface->AddBody(body_id, EActivation::Activate);
			});

			Execute(random, "Remove/AddBody", [this, body_id]() {
				// Undo/redo add to trigger more race conditions
				mBodyInterface->RemoveBody(body_id);
				mBodyInterface->AddBody(body_id, EActivation::Activate);
			});

			bodies.push_back(body_id);
		}

		uniform_real_distribution<float> chance(0, 1);
		if (bodies.size() > 0 && chance(random) < 0.5f)
		{
			// Pick random body
			uniform_int_distribution<size_t> element(0, bodies.size() - 1);
			size_t index = element(random);
			BodyID body_id = bodies[index];
			bodies.erase(bodies.begin() + index);

			Execute(random, "Remove/DestroyBody", [this, body_id]() {
				// Remove it
				mBodyInterface->RemoveBody(body_id);
				mBodyInterface->DestroyBody(body_id);
			});
		}

		this_thread::sleep_for(1ms);
	}

	JPH_PROFILE_THREAD_END();
}

void MultithreadedTest::RagdollSpawner()
{
	JPH_PROFILE_THREAD_START("RagdollSpawner");

#ifdef _DEBUG
	const int cMaxRagdolls = 10;
#else
	const int cMaxRagdolls = 50;
#endif

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

	default_random_engine random;
	uniform_real_distribution<float> from_y(0, 10);
	uniform_real_distribution<float> from_xz(-5, 5);

	CollisionGroup::GroupID group_id = 1;

	vector<Ref<Ragdoll>> ragdolls;

	while (!mIsQuitting)
	{
		// Ensure there are enough objects at all times
		if (ragdolls.size() < cMaxRagdolls)
		{
			// Create ragdoll
			Ref<Ragdoll> ragdoll = ragdoll_settings->CreateRagdoll(group_id++, 0, mPhysicsSystem);
	
			// Override root
			SkeletonPose::JointState &root = ragdoll_pose.GetJoint(0);
			root.mTranslation = Vec3(from_xz(random), 1.0f + from_y(random), from_xz(random));
			root.mRotation = Quat::sRandom(random);
			ragdoll_pose.CalculateJointMatrices();

			// Drive to pose
			ragdoll->SetPose(ragdoll_pose);
			ragdoll->DriveToPoseUsingMotors(ragdoll_pose);

			Execute(random, "Activate", [ragdoll]() {
				// Activate the ragdoll
				ragdoll->AddToPhysicsSystem(EActivation::Activate);
			});

			Execute(random, "Deactivate/Activate", [ragdoll]() {
				// Undo/redo add to trigger more race conditions
				ragdoll->RemoveFromPhysicsSystem();
				ragdoll->AddToPhysicsSystem(EActivation::Activate);
			});

			ragdolls.push_back(ragdoll);
		}

		uniform_real_distribution<float> chance(0, 1);
		if (ragdolls.size() > 0 && chance(random) < 0.1f)
		{
			// Pick random body
			uniform_int_distribution<size_t> element(0, ragdolls.size() - 1);
			size_t index = element(random);
			Ref<Ragdoll> ragdoll = ragdolls[index];
			ragdolls.erase(ragdolls.begin() + index);

			Execute(random, "Deactivate", [ragdoll]() {
				// Deactivate it
				ragdoll->RemoveFromPhysicsSystem();
			});
		}

		this_thread::sleep_for(1ms);
	}

	for (Ragdoll *r : ragdolls)
		r->RemoveFromPhysicsSystem();

	JPH_PROFILE_THREAD_END();
}

void MultithreadedTest::CasterMain()
{
	JPH_PROFILE_THREAD_START("CasterMain");

	default_random_engine random;

	vector<BodyID> bodies;

	while (!mIsQuitting)
	{
		Execute(random, "CastRay", [this, &random]() {
			// Cast a random ray
			uniform_real_distribution<float> from_y(0, 10);
			uniform_real_distribution<float> from_xz(-5, 5);
			Vec3 from = Vec3(from_xz(random), from_y(random), from_xz(random));
			Vec3 to = Vec3(from_xz(random), from_y(random), from_xz(random));
			RayCast ray { from, to - from };
			RayCastResult hit;
			if (mPhysicsSystem->GetNarrowPhaseQuery().CastRay(ray, hit, SpecifiedBroadPhaseLayerFilter(BroadPhaseLayers::MOVING), SpecifiedObjectLayerFilter(Layers::MOVING)))
			{
				// Draw hit position
				Vec3 hit_position_world = ray.mOrigin + hit.mFraction * ray.mDirection;
				mDebugRenderer->DrawMarker(hit_position_world, Color::sYellow, 0.2f);

				BodyLockRead lock(mPhysicsSystem->GetBodyLockInterface(), hit.mBodyID);
				if (lock.SucceededAndIsInBroadPhase())
				{
					// Draw normal
					const Body &hit_body = lock.GetBody();
					Mat44 inv_com = hit_body.GetInverseCenterOfMassTransform();
					Vec3 normal = inv_com.Multiply3x3Transposed(hit_body.GetShape()->GetSurfaceNormal(hit.mSubShapeID2, inv_com * hit_position_world)).Normalized();
					mDebugRenderer->DrawArrow(hit_position_world, hit_position_world + normal, Color::sGreen, 0.1f);
				}
			}
		});
	}

	JPH_PROFILE_THREAD_END();
}
