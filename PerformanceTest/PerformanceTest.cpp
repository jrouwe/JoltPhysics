// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

// Jolt includes
#include <Jolt.h>
#include <RegisterTypes.h>
#include <Core/TempAllocator.h>
#include <Core/JobSystemThreadPool.h>
#include <Physics/PhysicsSettings.h>
#include <Physics/PhysicsSystem.h>
#include <Physics/Ragdoll/Ragdoll.h>
#include <Physics/PhysicsScene.h>
#include <Physics/Collision/CastResult.h>
#include <Physics/Collision/RayCast.h>

// STL includes
#include <iostream>
#include <thread>
#include <chrono>

using namespace JPH;
using namespace std;

namespace Layers
{
	static constexpr uint8 NON_MOVING = 0;
	static constexpr uint8 MOVING = 1;
	static constexpr uint8 NUM_LAYERS = 2;
};

bool MyObjectCanCollide(ObjectLayer inObject1, ObjectLayer inObject2)
{
	switch (inObject1)
	{
	case Layers::NON_MOVING:
		return inObject2 == Layers::MOVING; // Non moving only collides with moving
	case Layers::MOVING:
		return true; // Moving collides with everything
	default:
		JPH_ASSERT(false);
		return false;
	}
};

namespace BroadPhaseLayers
{
	static constexpr BroadPhaseLayer NON_MOVING(0);
	static constexpr BroadPhaseLayer MOVING(1);
};

bool MyBroadPhaseCanCollide(ObjectLayer inLayer1, BroadPhaseLayer inLayer2)
{
	switch (inLayer1)
	{
	case Layers::NON_MOVING:
		return inLayer2 == BroadPhaseLayers::MOVING;
	case Layers::MOVING:
		return true;	
	default:
		JPH_ASSERT(false);
		return false;
	}
}

// Test configuration
const float cHorizontalSeparation = 4.0f;
const float cVerticalSeparation = 0.6f;
#ifdef _DEBUG
	const int cPileSize = 5;
	const int cNumRows = 2;
	const int cNumCols = 2;
#else
	const int cPileSize = 10;
	const int cNumRows = 4;
	const int cNumCols = 4;
#endif
const float cDeltaTime = 1.0f / 60.0f;

// Program entry point
int main(int argc, char** argv)
{
	// Register all Jolt physics types
	RegisterTypes();

	// Create temp allocator
	TempAllocatorImpl temp_allocator(10 * 1024 * 1024);

	// Load ragdoll
	Ref<RagdollSettings> ragdoll_settings;
	if (!ObjectStreamIn::sReadObject("Assets/Human.tof", ragdoll_settings))
	{
		cerr << "Unable to load ragdoll" << endl;
		return 1;
	}
	for (BodyCreationSettings &body : ragdoll_settings->mParts)
		body.mObjectLayer = Layers::MOVING;

	// Init ragdoll
	ragdoll_settings->GetSkeleton()->CalculateParentJointIndices();
	ragdoll_settings->Stabilize();
	ragdoll_settings->CalculateBodyIndexToConstraintIndex();
	ragdoll_settings->CalculateConstraintIndexToBodyIdxPair();

	// Load animation
	Ref<SkeletalAnimation> animation;
	if (!ObjectStreamIn::sReadObject("Assets/Human/dead_pose1.tof", animation))
	{
		cerr << "Unable to load animation" << endl;
		return 1;
	}

	// Sample pose
	SkeletonPose pose;
	pose.SetSkeleton(ragdoll_settings->GetSkeleton());
	animation->Sample(0.0f, pose);

	// Read the scene
	Ref<PhysicsScene> scene;
	if (!ObjectStreamIn::sReadObject("Assets/terrain2.bof", scene))
	{
		cerr << "Unable to load terrain" << endl;
		return 1;
	}
	for (BodyCreationSettings &body : scene->GetBodies())
		body.mObjectLayer = Layers::NON_MOVING;
	scene->FixInvalidScales();

	// Create mapping table from object layer to broadphase layer
	ObjectToBroadPhaseLayer object_to_broadphase;
	object_to_broadphase.resize(Layers::NUM_LAYERS);
	object_to_broadphase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
	object_to_broadphase[Layers::MOVING] = BroadPhaseLayers::MOVING;

	// Start profiling this thread
	JPH_PROFILE_THREAD_START("Main");

	// Trace header
	cout << "Motion Quality, Thread Count, Time (s), Hash" << endl;

	// Iterate motion qualities
	for (uint mq = 0; mq < 2; ++mq)
	{
		// Determine motion quality
		EMotionQuality motion_quality = mq == 0? EMotionQuality::Discrete : EMotionQuality::LinearCast;
		string motion_quality_str = mq == 0? "Discrete" : "LinearCast";

		// Set motion quality on ragdoll
		for (BodyCreationSettings &body : ragdoll_settings->mParts)
			body.mMotionQuality = motion_quality;

		// Test thread permutations
		for (uint num_threads = 0; num_threads < thread::hardware_concurrency(); ++num_threads)
		{
			// Create job system with desired number of threads
			JobSystemThreadPool job_system(cMaxPhysicsJobs, cMaxPhysicsBarriers, num_threads);

			// Create physics system
			PhysicsSystem physics_system;
			physics_system.Init(10240, 0, 65536, 10240, object_to_broadphase, MyBroadPhaseCanCollide, MyObjectCanCollide);

			// Add background geometry
			scene->CreateBodies(&physics_system);

			// Create ragdoll piles
			vector<Ref<Ragdoll>> ragdolls;
			mt19937 random;
			uniform_real_distribution<float> angle(0.0f, JPH_PI);
			CollisionGroup::GroupID group_id = 1;
			for (int row = 0; row < cNumRows; ++row)
				for (int col = 0; col < cNumCols; ++col)
				{
					// Determine start location of ray
					Vec3 start = Vec3(cHorizontalSeparation * (col - (cNumCols - 1) / 2.0f), 100, cHorizontalSeparation * (row - (cNumRows - 1) / 2.0f));

					// Cast ray down to terrain
					RayCastResult hit;
					Vec3 ray_direction(0, -200, 0);
					RayCast ray { start, ray_direction };
					if (physics_system.GetNarrowPhaseQuery().CastRay(ray, hit, SpecifiedBroadPhaseLayerFilter(BroadPhaseLayers::NON_MOVING), SpecifiedObjectLayerFilter(Layers::NON_MOVING)))
						start = start + hit.mFraction * ray_direction;

					for (int i = 0; i < cPileSize; ++i)
					{
						// Create ragdoll
						Ref<Ragdoll> ragdoll = ragdoll_settings->CreateRagdoll(group_id++, nullptr, &physics_system);
	
						// Override root
						SkeletonPose pose_copy = pose;
						SkeletonPose::JointState &root = pose_copy.GetJoint(0);
						root.mTranslation = start + Vec3(0, cVerticalSeparation * (i + 1), 0);
						root.mRotation = Quat::sRotation(Vec3::sAxisY(), angle(random)) * root.mRotation;
						pose_copy.CalculateJointMatrices();

						// Drive to pose
						ragdoll->SetPose(pose_copy);
						ragdoll->DriveToPoseUsingMotors(pose_copy);
						ragdoll->AddToPhysicsSystem(EActivation::Activate);

						// Keep reference
						ragdolls.push_back(ragdoll);
					}
				}

			chrono::nanoseconds total_duration(0);

			// Step the world for a fixed amount of iterations
			for (uint iterations = 0; iterations < 500; ++iterations)
			{
				JPH_PROFILE_NEXTFRAME();

				// Start measuring
				chrono::high_resolution_clock::time_point clock_start = chrono::high_resolution_clock::now();

				// Do a physics step
				physics_system.Update(cDeltaTime, 1, 1, &temp_allocator, &job_system);

				// Stop measuring
				chrono::high_resolution_clock::time_point clock_end = chrono::high_resolution_clock::now();
				total_duration += chrono::duration_cast<chrono::nanoseconds>(clock_end - clock_start);

				// Dump profile information every 100 iterations
				if (iterations % 100 == 0)
				{
					JPH_PROFILE_DUMP();
				}
			}

			// Calculate hash of all positions and rotations of the bodies
			size_t hash = 0;
			BodyInterface &bi = physics_system.GetBodyInterfaceNoLock();
			for (Ragdoll *ragdoll : ragdolls)
				for (BodyID id : ragdoll->GetBodyIDs())
				{
					Vec3 pos = bi.GetPosition(id);
					Quat rot = bi.GetRotation(id);
					hash_combine(hash, pos.GetX(), pos.GetY(), pos.GetZ(), rot.GetX(), rot.GetY(), rot.GetZ(), rot.GetW());
				}

			// Remove ragdolls
			for (Ragdoll *ragdoll : ragdolls)
				ragdoll->RemoveFromPhysicsSystem();

			// Trace stat line
			cout << motion_quality_str << ", " << num_threads + 1 << ", " << 1.0e-9 * total_duration.count() << ", " << hash << endl;
		}
	}

	// End profiling this thread
	JPH_PROFILE_THREAD_END();

	return 0;
}
