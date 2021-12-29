// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

// Jolt includes
#include <Jolt.h>
#include <RegisterTypes.h>
#include <Core/TempAllocator.h>
#include <Core/JobSystemThreadPool.h>
#include <Physics/PhysicsSettings.h>
#include <Physics/PhysicsSystem.h>
#ifdef JPH_DEBUG_RENDERER
	#include <Renderer/DebugRendererRecorder.h>
	#include <Core/StreamWrapper.h>
#endif // JPH_DEBUG_RENDERER

// STL includes
#include <iostream>
#include <thread>
#include <chrono>
#include <memory>

using namespace JPH;
using namespace std;

// Local includes
#include "RagdollScene.h"
#include "ConvexVsMeshScene.h"

// Time step for physics
constexpr float cDeltaTime = 1.0f / 60.0f;

// Number of iterations to run the test
constexpr uint cMaxIterations = 500;

// Program entry point
int main(int argc, char** argv)
{
	// Parse command line parameters
	int specified_quality = -1;
	int specified_threads = -1;
	bool enable_profiler = false;
	bool enable_debug_renderer = false;
	unique_ptr<PerformanceTestScene> scene;
	for (int argidx = 1; argidx < argc; ++argidx)
	{
		const char *arg = argv[argidx];

		if (strncmp(arg, "-s=", 3) == 0)
		{
			// Parse scene
			if (strcmp(arg + 3, "Ragdoll") == 0)
				scene = unique_ptr<PerformanceTestScene>(new RagdollScene);
			else if (strcmp(arg + 3, "ConvexVsMesh") == 0)
				scene = unique_ptr<PerformanceTestScene>(new ConvexVsMeshScene);
			else
			{
				cerr << "Invalid scene" << endl;
				return 1;
			}
		}
		else if (strncmp(arg, "-q=", 3) == 0)
		{
			// Parse quality
			if (strcmp(arg + 3, "Discrete") == 0)
				specified_quality = 0;
			else if (strcmp(arg + 3, "LinearCast") == 0)
				specified_quality = 1;
			else
			{
				cerr << "Invalid quality" << endl;
				return 1;
			}
		}
		else if (strncmp(arg, "-t=", 3) == 0)
		{
			// Parse threads
			specified_threads = atoi(arg + 3);
		}
		else if (strcmp(arg, "-p") == 0)
		{
			enable_profiler = true;
		}
		else if (strcmp(arg, "-r") == 0)
		{
			enable_debug_renderer = true;
		}
		else if (strcmp(arg, "-h") == 0)
		{
			// Print usage
			cerr << "Usage: PerformanceTest [-s=<scene>] [-q=<quality>] [-t=<threads>] [-p] [-r]" << endl
				 << "-s: Select scene (Ragdoll, ConvexVsMesh)" << endl
				 << "-q: Test only with specified quality (Discrete, LinearCast)" << endl
				 << "-t: Test only with N threads" << endl
				 << "-p: Write out profiles" << endl
				 << "-r: Record debug renderer output for JoltViewer" << endl;
			return 0;
		}
	}

	// Register all Jolt physics types
	RegisterTypes();

	// Create temp allocator
	TempAllocatorImpl temp_allocator(10 * 1024 * 1024);

	// Load the scene
	if (scene == nullptr)
		scene = unique_ptr<PerformanceTestScene>(new RagdollScene);
	if (!scene->Load())
		return 1;

	// Output scene we're running
	cout << "Running scene: " << scene->GetName() << endl;

	// Create mapping table from object layer to broadphase layer
	ObjectToBroadPhaseLayer object_to_broadphase = GetObjectToBroadPhaseLayer();

	// Start profiling this thread
	JPH_PROFILE_THREAD_START("Main");

	// Trace header
	cout << "Motion Quality, Thread Count, Steps / Second, Hash" << endl;

	// Iterate motion qualities
	for (uint mq = 0; mq < 2; ++mq)
	{
		// Skip quality if another was specified
		if (specified_quality != -1 && mq != (uint)specified_quality)
			continue;

		// Determine motion quality
		EMotionQuality motion_quality = mq == 0? EMotionQuality::Discrete : EMotionQuality::LinearCast;
		string motion_quality_str = mq == 0? "Discrete" : "LinearCast";

		// Determine which thread counts to test
		vector<uint> thread_permutations;
		if (specified_threads > 0)
			thread_permutations.push_back((uint)specified_threads - 1);
		else
			for (uint num_threads = 0; num_threads < thread::hardware_concurrency(); ++num_threads)
				thread_permutations.push_back(num_threads);

		// Test thread permutations
		for (uint num_threads : thread_permutations)
		{
			// Create job system with desired number of threads
			JobSystemThreadPool job_system(cMaxPhysicsJobs, cMaxPhysicsBarriers, num_threads);

			// Create physics system
			PhysicsSystem physics_system;
			physics_system.Init(10240, 0, 65536, 10240, object_to_broadphase, BroadPhaseCanCollide, ObjectCanCollide);

			// Start test scene
			scene->StartTest(physics_system, motion_quality);

		#ifdef JPH_DEBUG_RENDERER
			// Open output
			ofstream renderer_file;
			if (enable_debug_renderer)
				renderer_file.open(("performance_test_" + ToLower(motion_quality_str) + "_th" + ConvertToString(num_threads + 1) + ".JoltRecording").c_str(), ofstream::out | ofstream::binary | ofstream::trunc);
			StreamOutWrapper renderer_stream(renderer_file);
			DebugRendererRecorder renderer(renderer_stream);
		#endif // JPH_DEBUG_RENDERER

			chrono::nanoseconds total_duration(0);

			// Step the world for a fixed amount of iterations
			for (uint iterations = 0; iterations < cMaxIterations; ++iterations)
			{
				JPH_PROFILE_NEXTFRAME();

				// Start measuring
				chrono::high_resolution_clock::time_point clock_start = chrono::high_resolution_clock::now();

				// Do a physics step
				physics_system.Update(cDeltaTime, 1, 1, &temp_allocator, &job_system);

				// Stop measuring
				chrono::high_resolution_clock::time_point clock_end = chrono::high_resolution_clock::now();
				total_duration += chrono::duration_cast<chrono::nanoseconds>(clock_end - clock_start);

			#ifdef JPH_DEBUG_RENDERER
				if (enable_debug_renderer)
				{
					// Draw the state of the world
					BodyManager::DrawSettings settings;
					physics_system.DrawBodies(settings, &renderer);

					// Mark end of frame
					renderer.EndFrame();
				}
			#endif // JPH_DEBUG_RENDERER

				// Dump profile information every 100 iterations
				if (enable_profiler && iterations % 100 == 0)
				{
					JPH_PROFILE_DUMP(ToLower(motion_quality_str) + "_th" + ConvertToString(num_threads + 1) + "_it" + ConvertToString(iterations));
				}
			}

			// Calculate hash of all positions and rotations of the bodies
			size_t hash = 0;
			BodyInterface &bi = physics_system.GetBodyInterfaceNoLock();
			BodyIDVector body_ids;
			physics_system.GetBodies(body_ids);
			for (BodyID id : body_ids)
			{
				Vec3 pos = bi.GetPosition(id);
				Quat rot = bi.GetRotation(id);
				hash_combine(hash, pos.GetX(), pos.GetY(), pos.GetZ(), rot.GetX(), rot.GetY(), rot.GetZ(), rot.GetW());
			}

			// Stop test scene
			scene->StopTest(physics_system);

			// Trace stat line
			cout << motion_quality_str << ", " << num_threads + 1 << ", " << double(cMaxIterations) / (1.0e-9 * total_duration.count()) << ", " << hash << endl;
		}
	}

	// End profiling this thread
	JPH_PROFILE_THREAD_END();

	return 0;
}
