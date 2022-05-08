// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

// Jolt includes
#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/NarrowPhaseStats.h>
#ifdef JPH_DEBUG_RENDERER
	#include <Jolt/Renderer/DebugRendererRecorder.h>
	#include <Jolt/Core/StreamWrapper.h>
#endif // JPH_DEBUG_RENDERER

// STL includes
#include <iostream>
#include <thread>
#include <chrono>
#include <memory>
#include <cstdarg>

using namespace JPH;
using namespace std;

// Disable common warnings triggered by Jolt
JPH_SUPPRESS_WARNINGS

// Local includes
#include "RagdollScene.h"
#include "ConvexVsMeshScene.h"

// Time step for physics
constexpr float cDeltaTime = 1.0f / 60.0f;

static void TraceImpl(const char *inFMT, ...)
{ 
	// Format the message
	va_list list;
	va_start(list, inFMT);
	char buffer[1024];
	vsnprintf(buffer, sizeof(buffer), inFMT, list);

	// Print to the TTY
	cout << buffer << endl;
}

// Program entry point
int main(int argc, char** argv)
{
	// Parse command line parameters
	int specified_quality = -1;
	int specified_threads = -1;
	uint max_iterations = 500;
	bool disable_sleep = false;
	bool enable_profiler = false;
#ifdef JPH_DEBUG_RENDERER
	bool enable_debug_renderer = false;
#endif // JPH_DEBUG_RENDERER
	bool enable_per_frame_recording = false;
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
		else if (strncmp(arg, "-i=", 3) == 0)
		{
			// Parse max iterations
			max_iterations = (uint)atoi(arg + 3);
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
		else if (strcmp(arg, "-no_sleep") == 0)
		{
			disable_sleep = true;
		}
		else if (strcmp(arg, "-p") == 0)
		{
			enable_profiler = true;
		}
	#ifdef JPH_DEBUG_RENDERER
		else if (strcmp(arg, "-r") == 0)
		{
			enable_debug_renderer = true;
		}
	#endif // JPH_DEBUG_RENDERER
		else if (strcmp(arg, "-f") == 0)
		{
			enable_per_frame_recording = true;
		}
		else if (strcmp(arg, "-h") == 0)
		{
			// Print usage
			cerr << "Usage:" << endl
				 << "-s=<scene>: Select scene (Ragdoll, ConvexVsMesh)" << endl
				 << "-i=<num physics steps>: Number of physics steps to simulate (default 500)" << endl
				 << "-q=<quality>: Test only with specified quality (Discrete, LinearCast)" << endl
				 << "-t=<num threads>: Test only with N threads (default is to iterate over 1 .. num hardware threads)" << endl
				 << "-p: Write out profiles" << endl
				 << "-r: Record debug renderer output for JoltViewer" << endl
				 << "-f: Record per frame timings" << endl
				 << "-no_sleep: Disable sleeping" << endl;
			return 0;
		}
	}

	// Install callbacks
	Trace = TraceImpl;

	// Create a factory
	Factory::sInstance = new Factory();

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
	BPLayerInterfaceImpl broad_phase_layer_interface;

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
			physics_system.Init(10240, 0, 65536, 10240, broad_phase_layer_interface, BroadPhaseCanCollide, ObjectCanCollide);

			// Start test scene
			scene->StartTest(physics_system, motion_quality);

			// Disable sleeping if requested
			if (disable_sleep)
			{
				const BodyLockInterface &bli = physics_system.GetBodyLockInterfaceNoLock();
				BodyIDVector body_ids;
				physics_system.GetBodies(body_ids);
				for (BodyID id : body_ids)
				{
					BodyLockWrite lock(bli, id);
					if (lock.Succeeded())
					{
						Body &body = lock.GetBody();
						if (!body.IsStatic())
							body.SetAllowSleeping(false);
					}
				}
			}

			// Optimize the broadphase to prevent an expensive first frame
			physics_system.OptimizeBroadPhase();

			// A tag used to identify the test
			string tag = ToLower(motion_quality_str) + "_th" + ConvertToString(num_threads + 1);
					     
		#ifdef JPH_DEBUG_RENDERER
			// Open renderer output
			ofstream renderer_file;
			if (enable_debug_renderer)
				renderer_file.open(("performance_test_" + tag + ".jor").c_str(), ofstream::out | ofstream::binary | ofstream::trunc);
			StreamOutWrapper renderer_stream(renderer_file);
			DebugRendererRecorder renderer(renderer_stream);
		#endif // JPH_DEBUG_RENDERER

			// Open per frame timing output
			ofstream per_frame_file;
			if (enable_per_frame_recording)
			{
				per_frame_file.open(("per_frame_" + tag + ".csv").c_str(), ofstream::out | ofstream::trunc);
				per_frame_file << "Frame, Time (ms)" << endl;
			}

			chrono::nanoseconds total_duration(0);

			// Step the world for a fixed amount of iterations
			for (uint iterations = 0; iterations < max_iterations; ++iterations)
			{
				JPH_PROFILE_NEXTFRAME();

				// Start measuring
				chrono::high_resolution_clock::time_point clock_start = chrono::high_resolution_clock::now();

				// Do a physics step
				physics_system.Update(cDeltaTime, 1, 1, &temp_allocator, &job_system);

				// Stop measuring
				chrono::high_resolution_clock::time_point clock_end = chrono::high_resolution_clock::now();
				chrono::nanoseconds duration = chrono::duration_cast<chrono::nanoseconds>(clock_end - clock_start);
				total_duration += duration;

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

				// Record time taken this iteration
				if (enable_per_frame_recording)
					per_frame_file << iterations << ", " << (1.0e-6 * duration.count()) << endl;

				// Dump profile information every 100 iterations
				if (enable_profiler && iterations % 100 == 0)
				{
					JPH_PROFILE_DUMP(tag + "_it" + ConvertToString(iterations));
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
			cout << motion_quality_str << ", " << num_threads + 1 << ", " << double(max_iterations) / (1.0e-9 * total_duration.count()) << ", " << hash << endl;
		}
	}

#ifdef JPH_TRACK_NARROWPHASE_STATS
	NarrowPhaseStat::sReportStats();
#endif // JPH_TRACK_NARROWPHASE_STATS

	// Destroy the factory
	delete Factory::sInstance;
	Factory::sInstance = nullptr;

	// End profiling this thread
	JPH_PROFILE_THREAD_END();

	return 0;
}
