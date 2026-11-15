// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include "PhysicsTestContext.h"
#include "Layers.h"
#include <Jolt/Physics/Ragdoll/Ragdoll.h>
#include <Jolt/Physics/Constraints/HingeConstraint.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Skeleton/Skeleton.h>
#include <Jolt/Skeleton/SkeletonPose.h>

JPH_SUPPRESS_WARNINGS_STD_BEGIN
#include <thread>
#include <chrono>
JPH_SUPPRESS_WARNINGS_STD_END

#if !defined(JPH_CPU_WASM) || defined(__EMSCRIPTEN_PTHREADS__)

// Stress test that adds / removes bodies and ragdolls and does collision queries from multiple threads while the simulation is running.
// This test is mainly useful when running under ThreadSanitizer as it checks for race conditions. It mimics the MultithreadedTest sample.
TEST_SUITE("MultithreadedTests")
{
	// Ragdoll consisting of a vertical chain of capsules connected by hinges
	static constexpr int cNumRagdollParts = 5;
	static constexpr float cCapsuleHalfHeight = 0.2f;
	static constexpr float cCapsuleRadius = 0.1f;
	static constexpr float cCapsuleSpacing = 2.0f * (cCapsuleHalfHeight + cCapsuleRadius) + 0.1f; // Leave a small gap so that the capsules don't collide in the rest pose

#ifdef JPH_DEBUG
	static constexpr uint cMaxBoxes = 50;
	static constexpr uint cMaxRagdolls = 5;
#else
	static constexpr uint cMaxBoxes = 500;
	static constexpr uint cMaxRagdolls = 25;
#endif

	static Ref<RagdollSettings> sCreateCapsuleChain()
	{
		Ref<Skeleton> skeleton = new Skeleton;
		skeleton->AddJoint("Part0");
		for (int p = 1; p < cNumRagdollParts; ++p)
			skeleton->AddJoint("Part" + ConvertToString(p), p - 1);

		Ref<RagdollSettings> settings = new RagdollSettings;
		settings->mSkeleton = skeleton;
		settings->mParts.resize(cNumRagdollParts);
		for (int p = 0; p < cNumRagdollParts; ++p)
		{
			RagdollSettings::Part &part = settings->mParts[p];
			part.SetShape(new CapsuleShape(cCapsuleHalfHeight, cCapsuleRadius));
			part.mPosition = RVec3(0, p * cCapsuleSpacing, 0);
			part.mMotionType = EMotionType::Dynamic;
			part.mObjectLayer = Layers::MOVING;

			if (p > 0)
			{
				// Connect to the previous capsule with a hinge
				Ref<HingeConstraintSettings> hinge = new HingeConstraintSettings;
				hinge->mPoint1 = hinge->mPoint2 = RVec3(0, (p - 0.5f) * cCapsuleSpacing, 0);
				hinge->mHingeAxis1 = hinge->mHingeAxis2 = Vec3::sAxisZ();
				hinge->mNormalAxis1 = hinge->mNormalAxis2 = Vec3::sAxisX();
				hinge->mLimitsMin = -0.5f * JPH_PI;
				hinge->mLimitsMax = 0.5f * JPH_PI;
				part.mToParent = hinge;
			}
		}

		settings->Stabilize();
		settings->CalculateBodyIndexToConstraintIndex();
		settings->CalculateConstraintIndexToBodyIdxPair();

		return settings;
	}

	// Execute inFunction either as a job on the job system or directly on the calling thread
	static void sExecute(JobSystem *inJobSystem, UnitTestRandom &ioRandom, const char *inName, const function<void()> &inFunction)
	{
		uniform_real_distribution<float> chance(0, 1);
		if (chance(ioRandom) < 0.5f)
		{
			// Execute as a job and wait for it
			JobHandle handle = inJobSystem->CreateJob(inName, Color::sGreen, inFunction);
			while (!handle.IsDone())
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		else
		{
			// Execute on this thread (not part of the job system)
			inFunction();
		}
	}

	static void sBoxSpawner(PhysicsTestContext &ioContext, const atomic<bool> &inIsQuitting)
	{
		BodyInterface &bi = ioContext.GetBodyInterface();
		JobSystem *job_system = ioContext.GetJobSystem();
		UnitTestRandom random(1);
		uniform_real_distribution<float> from_y(0, 10);
		uniform_real_distribution<float> from_xz(-5, 5);
		uniform_real_distribution<float> chance(0, 1);

		Array<BodyID> bodies;

		while (!inIsQuitting)
		{
			// Ensure there are enough objects at all times
			if (bodies.size() < cMaxBoxes)
			{
				BodyID body_id;
				sExecute(job_system, random, "AddBody", [&bi, &body_id, &random, &from_y, &from_xz]() {
					RVec3 position(from_xz(random), 1.0f + from_y(random), from_xz(random));
					Quat orientation = Quat::sRandom(random);
					Vec3 velocity = Vec3::sRandom(random);
					Body *body = bi.CreateBody(BodyCreationSettings(new BoxShape(Vec3(0.5f, 0.2f, 0.3f)), position, orientation, EMotionType::Dynamic, Layers::MOVING));
					if (body != nullptr)
					{
						body->SetLinearVelocity(velocity);
						body_id = body->GetID();
						bi.AddBody(body_id, EActivation::Activate);
					}
				});
				if (body_id.IsInvalid())
					break; // Out of bodies

				sExecute(job_system, random, "Remove/AddBody", [&bi, body_id]() {
					// Undo/redo add to trigger more race conditions
					bi.RemoveBody(body_id);
					bi.AddBody(body_id, EActivation::Activate);
				});

				bodies.push_back(body_id);
			}

			if (!bodies.empty() && chance(random) < 0.5f)
			{
				// Pick random body
				uniform_int_distribution<size_t> element(0, bodies.size() - 1);
				size_t index = element(random);
				BodyID body_id = bodies[index];
				bodies.erase(bodies.begin() + index);

				sExecute(job_system, random, "Remove/DestroyBody", [&bi, body_id]() {
					bi.RemoveBody(body_id);
					bi.DestroyBody(body_id);
				});
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		// Clean up
		for (const BodyID &body_id : bodies)
		{
			bi.RemoveBody(body_id);
			bi.DestroyBody(body_id);
		}
	}

	static void sRagdollSpawner(PhysicsTestContext &ioContext, const atomic<bool> &inIsQuitting)
	{
		PhysicsSystem *system = ioContext.GetSystem();
		JobSystem *job_system = ioContext.GetJobSystem();
		UnitTestRandom random(2);
		uniform_real_distribution<float> from_y(0, 10);
		uniform_real_distribution<float> from_xz(-5, 5);
		uniform_real_distribution<float> chance(0, 1);

		Ref<RagdollSettings> ragdoll_settings = sCreateCapsuleChain();

		// Get the rest pose of the ragdoll
		SkeletonPose ragdoll_pose;
		ragdoll_pose.SetSkeleton(ragdoll_settings->GetSkeleton());
		{
			Ref<Ragdoll> temp_ragdoll = ragdoll_settings->CreateRagdoll(0, 0, system);
			temp_ragdoll->GetPose(ragdoll_pose);
			ragdoll_pose.CalculateJointStates();
		}

		CollisionGroup::GroupID group_id = 1;

		Array<Ref<Ragdoll>> ragdolls;

		while (!inIsQuitting)
		{
			// Ensure there are enough ragdolls at all times
			if (ragdolls.size() < cMaxRagdolls)
			{
				Ref<Ragdoll> ragdoll = ragdoll_settings->CreateRagdoll(group_id++, 0, system);
				if (ragdoll == nullptr)
					break; // Out of bodies

				// Randomize root
				SkeletonPose::JointState &root = ragdoll_pose.GetJoint(0);
				root.mRotation = Quat::sRandom(random);
				ragdoll_pose.SetRootOffset(RVec3(from_xz(random), 1.0f + from_y(random), from_xz(random)));
				ragdoll_pose.CalculateJointMatrices();

				// Drive to pose
				ragdoll->SetPose(ragdoll_pose);
				ragdoll->DriveToPoseUsingMotors(ragdoll_pose);

				sExecute(job_system, random, "Activate", [ragdoll]() {
					ragdoll->AddToPhysicsSystem(EActivation::Activate);
				});

				sExecute(job_system, random, "Deactivate/Activate", [ragdoll]() {
					// Undo/redo add to trigger more race conditions
					ragdoll->RemoveFromPhysicsSystem();
					ragdoll->AddToPhysicsSystem(EActivation::Activate);
				});

				ragdolls.push_back(ragdoll);
			}

			if (!ragdolls.empty() && chance(random) < 0.1f)
			{
				// Pick random ragdoll
				uniform_int_distribution<size_t> element(0, ragdolls.size() - 1);
				size_t index = element(random);
				Ref<Ragdoll> ragdoll = ragdolls[index];
				ragdolls.erase(ragdolls.begin() + index);

				sExecute(job_system, random, "Deactivate", [ragdoll]() {
					ragdoll->RemoveFromPhysicsSystem();
				});
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		// Clean up
		for (Ragdoll *r : ragdolls)
			r->RemoveFromPhysicsSystem();
	}

	static void sCaster(PhysicsTestContext &ioContext, const atomic<bool> &inIsQuitting)
	{
		PhysicsSystem *system = ioContext.GetSystem();
		JobSystem *job_system = ioContext.GetJobSystem();
		UnitTestRandom random(3);
		uniform_real_distribution<float> from_y(0, 10);
		uniform_real_distribution<float> from_xz(-5, 5);

		while (!inIsQuitting)
		{
			sExecute(job_system, random, "CastRay", [system, &random, &from_y, &from_xz]() {
				// Cast a random ray
				RVec3 from(from_xz(random), from_y(random), from_xz(random));
				RVec3 to(from_xz(random), from_y(random), from_xz(random));
				RRayCast ray { from, Vec3(to - from) };
				RayCastResult hit;
				if (system->GetNarrowPhaseQuery().CastRay(ray, hit, SpecifiedBroadPhaseLayerFilter(BroadPhaseLayers::MOVING), SpecifiedObjectLayerFilter(Layers::MOVING)))
				{
					// Get the surface normal at the hit
					RVec3 hit_position_world = ray.GetPointOnRay(hit.mFraction);
					BodyLockRead lock(system->GetBodyLockInterface(), hit.mBodyID);
					if (lock.SucceededAndIsInBroadPhase())
					{
						const Body &hit_body = lock.GetBody();
						RMat44 inv_com = hit_body.GetInverseCenterOfMassTransform();
						Vec3 normal = inv_com.Multiply3x3Transposed(hit_body.GetShape()->GetSurfaceNormal(hit.mSubShapeID2, Vec3(inv_com * hit_position_world)));
						CHECK(normal.IsNormalized(1.0e-3f));
					}
				}
			});
		}
	}

	TEST_CASE("TestMultithreadedAddRemoveAndQuery")
	{
		PhysicsTestContext c(1.0f / 60.0f, 1, 4, 2048, 4096, 4096); // Note that we may run out of contact constraints, this is not a problem for this test
		c.CreateFloor();

		// Start threads that add / remove bodies and do queries
		atomic<bool> is_quitting = false;
		std::thread box_spawner([&c, &is_quitting]() { sBoxSpawner(c, is_quitting); });
		std::thread ragdoll_spawner([&c, &is_quitting]() { sRagdollSpawner(c, is_quitting); });
		std::thread caster([&c, &is_quitting]() { sCaster(c, is_quitting); });

		// Simulate for 2 seconds while the threads are running.
		// We step in real time (like the sample does), otherwise the simulation runs much faster than the threads can add objects.
		constexpr int cNumSteps = 120;
		auto start = std::chrono::steady_clock::now();
		std::chrono::duration<float> delta_time(c.GetDeltaTime());
		for (int step = 1; step <= cNumSteps; ++step)
		{
			c.SimulateSingleStep();
			std::this_thread::sleep_until(start + std::chrono::duration_cast<std::chrono::steady_clock::duration>(step * delta_time));
		}

		// Stop the threads
		is_quitting = true;
		box_spawner.join();
		ragdoll_spawner.join();
		caster.join();

		// Everything should have been cleaned up, only the floor should remain
		CHECK(c.GetSystem()->GetNumBodies() == 1);
		CHECK(c.GetSystem()->GetConstraints().empty());
	}
}

#endif // !defined(JPH_CPU_WASM) || defined(__EMSCRIPTEN_PTHREADS__)
