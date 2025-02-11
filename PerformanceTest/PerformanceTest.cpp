// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

// Jolt includes
#include <Jolt/Jolt.h>
#include <Jolt/ConfigurationString.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/NarrowPhaseStats.h>
#include <Jolt/Physics/StateRecorderImpl.h>
#include <Jolt/Physics/DeterminismLog.h>
#ifdef JPH_DEBUG_RENDERER
	#include <Jolt/Renderer/DebugRendererRecorder.h>
	#include <Jolt/Core/StreamWrapper.h>
#endif // JPH_DEBUG_RENDERER
#ifdef JPH_PLATFORM_ANDROID
#include <android/log.h>
#include <android_native_app_glue.h>
#endif // JPH_PLATFORM_ANDROID

// STL includes
JPH_SUPPRESS_WARNINGS_STD_BEGIN
#include <iostream>
#include <thread>
#include <chrono>
#include <memory>
#include <cstdarg>
#include <filesystem>
JPH_SUPPRESS_WARNINGS_STD_END

using namespace JPH;
using namespace JPH::literals;
using namespace std;

// Disable common warnings triggered by Jolt
JPH_SUPPRESS_WARNINGS

// Local includes
#include "RagdollScene.h"
#include "ConvexVsMeshScene.h"
#include "PyramidScene.h"
#include "LargeMeshScene.h"
#include "CharacterVirtualScene.h"
#include "MaxBodiesScene.h"

// Time step for physics
constexpr float cDeltaTime = 1.0f / 60.0f;

static void TraceImpl(const char *inFMT, ...)
{
	// Format the message
	va_list list;
	va_start(list, inFMT);
	char buffer[1024];
	vsnprintf(buffer, sizeof(buffer), inFMT, list);
	va_end(list);

	// Print to the TTY
#ifndef JPH_PLATFORM_ANDROID
	cout << buffer << endl;
#else
	__android_log_write(ANDROID_LOG_INFO, "Jolt", buffer);
#endif
}

// Program entry point
int main(int argc, char** argv)
{
	// Install callbacks
	Trace = TraceImpl;

	// Register allocation hook
	RegisterDefaultAllocator();

	// Helper function that creates the default scene
#ifdef JPH_OBJECT_STREAM
	auto create_ragdoll_scene = []{ return unique_ptr<PerformanceTestScene>(new RagdollScene(4, 10, 0.6f)); };
#else
	auto create_ragdoll_scene = []{ return unique_ptr<PerformanceTestScene>(new ConvexVsMeshScene); };
#endif // JPH_OBJECT_STREAM

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
	bool record_state = false;
	bool validate_state = false;
	unique_ptr<PerformanceTestScene> scene;
	const char *validate_hash = nullptr;
	int repeat = 1;
	for (int argidx = 1; argidx < argc; ++argidx)
	{
		const char *arg = argv[argidx];

		if (strncmp(arg, "-s=", 3) == 0)
		{
			// Parse scene
			if (strcmp(arg + 3, "Ragdoll") == 0)
				scene = create_ragdoll_scene();
#ifdef JPH_OBJECT_STREAM
			else if (strcmp(arg + 3, "RagdollSinglePile") == 0)
				scene = unique_ptr<PerformanceTestScene>(new RagdollScene(1, 160, 0.4f));
#endif // JPH_OBJECT_STREAM
			else if (strcmp(arg + 3, "ConvexVsMesh") == 0)
				scene = unique_ptr<PerformanceTestScene>(new ConvexVsMeshScene);
			else if (strcmp(arg + 3, "Pyramid") == 0)
				scene = unique_ptr<PerformanceTestScene>(new PyramidScene);
			else if (strcmp(arg + 3, "LargeMesh") == 0)
				scene = unique_ptr<PerformanceTestScene>(new LargeMeshScene);
			else if (strcmp(arg + 3, "CharacterVirtual") == 0)
				scene = unique_ptr<PerformanceTestScene>(new CharacterVirtualScene);
			else if (strcmp(arg + 3, "MaxBodies") == 0)
				scene = unique_ptr<MaxBodiesScene>(new MaxBodiesScene);
			else
			{
				Trace("Invalid scene");
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
				Trace("Invalid quality");
				return 1;
			}
		}
		else if (strncmp(arg, "-t=max", 6) == 0)
		{
			// Default to number of threads on the system
			specified_threads = thread::hardware_concurrency();
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
		else if (strcmp(arg, "-rs") == 0)
		{
			record_state = true;
		}
		else if (strcmp(arg, "-vs") == 0)
		{
			validate_state = true;
		}
		else if (strncmp(arg, "-validate_hash=", 15) == 0)
		{
			validate_hash = arg + 15;
		}
		else if (strncmp(arg, "-repeat=", 8) == 0)
		{
			// Parse repeat count
			repeat = atoi(arg + 8);
		}
		else if (strcmp(arg, "-h") == 0)
		{
			// Print usage
			Trace("Usage:\n"
				  "-s=<scene>: Select scene (Ragdoll, RagdollSinglePile, ConvexVsMesh, Pyramid)\n"
				  "-i=<num physics steps>: Number of physics steps to simulate (default 500)\n"
				  "-q=<quality>: Test only with specified quality (Discrete, LinearCast)\n"
				  "-t=<num threads>: Test only with N threads (default is to iterate over 1 .. num hardware threads)\n"
				  "-t=max: Test with the number of threads available on the system\n"
				  "-p: Write out profiles\n"
				  "-r: Record debug renderer output for JoltViewer\n"
				  "-f: Record per frame timings\n"
				  "-no_sleep: Disable sleeping\n"
				  "-rs: Record state\n"
				  "-vs: Validate state\n"
				  "-validate_hash=<hash>: Validate hash (return 0 if successful, 1 if failed)\n"
				  "-repeat=<num>: Repeat all tests <num> times");
			return 0;
		}
	}

	// Create a factory
	Factory::sInstance = new Factory();

	// Register all Jolt physics types
	RegisterTypes();

	// Show used instruction sets
	Trace(GetConfigurationString());

	// If no scene was specified use the default scene
	if (scene == nullptr)
		scene = create_ragdoll_scene();

	// Output scene we're running
	Trace("Running scene: %s", scene->GetName());

	// Create temp allocator
	TempAllocatorImpl temp_allocator(scene->GetTempAllocatorSizeMB() * 1024 * 1024);

	// Find the asset path
	bool found = false;
	filesystem::path asset_path(argv[0]);
	filesystem::path root_path = asset_path.root_path();
	while (asset_path != root_path)
	{
		asset_path = asset_path.parent_path();
		if (filesystem::exists(asset_path / "Assets"))
		{
			found = true;
			break;
		}
	}
	if (!found) // Note that argv[0] can be a relative path like './PerformanceTest' so we also scan up using '..'
		for (int i = 0; i < 5; ++i)
		{
			asset_path /= "..";
			if (filesystem::exists(asset_path / "Assets"))
			{
				found = true;
				break;
			}
		}
	if (!found)
		asset_path = "Assets";
	else
		asset_path /= "Assets";
	asset_path /= "";

	// Load the scene
	if (!scene->Load(String(asset_path.string())))
		return 1;

	// Create mapping table from object layer to broadphase layer
	BPLayerInterfaceImpl broad_phase_layer_interface;

	// Create class that filters object vs broadphase layers
	ObjectVsBroadPhaseLayerFilterImpl object_vs_broadphase_layer_filter;

	// Create class that filters object vs object layers
	ObjectLayerPairFilterImpl object_vs_object_layer_filter;

	// Start profiling this program
	JPH_PROFILE_START("Main");

	// Trace header
	Trace("Motion Quality, Thread Count, Steps / Second, Hash");

	// Repeat test
	for (int r = 0; r < repeat; ++r)
	{
		// Iterate motion qualities
		for (uint mq = 0; mq < 2; ++mq)
		{
			// Skip quality if another was specified
			if (specified_quality != -1 && mq != (uint)specified_quality)
				continue;

			// Determine motion quality
			EMotionQuality motion_quality = mq == 0? EMotionQuality::Discrete : EMotionQuality::LinearCast;
			String motion_quality_str = mq == 0? "Discrete" : "LinearCast";

			// Determine which thread counts to test
			Array<uint> thread_permutations;
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
				physics_system.Init(scene->GetMaxBodies(), 0, scene->GetMaxBodyPairs(), scene->GetMaxContactConstraints(), broad_phase_layer_interface, object_vs_broadphase_layer_filter, object_vs_object_layer_filter);

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
				String tag = ToLower(motion_quality_str) + "_th" + ConvertToString(num_threads + 1);

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

				ofstream record_state_file;
				ifstream validate_state_file;
				if (record_state)
					record_state_file.open(("state_" + ToLower(motion_quality_str) + ".bin").c_str(), ofstream::out | ofstream::binary | ofstream::trunc);
				else if (validate_state)
					validate_state_file.open(("state_" + ToLower(motion_quality_str) + ".bin").c_str(), ifstream::in | ifstream::binary);

				chrono::nanoseconds total_duration(0);

				// Step the world for a fixed amount of iterations
				for (uint iterations = 0; iterations < max_iterations; ++iterations)
				{
					JPH_PROFILE_NEXTFRAME();
					JPH_DET_LOG("Iteration: " << iterations);

					// Start measuring
					chrono::high_resolution_clock::time_point clock_start = chrono::high_resolution_clock::now();

					// Update the test
					scene->UpdateTest(physics_system, temp_allocator, cDeltaTime);

					// Do a physics step
					physics_system.Update(cDeltaTime, 1, &temp_allocator, &job_system);

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

					if (record_state)
					{
						// Record state
						StateRecorderImpl recorder;
						physics_system.SaveState(recorder);

						// Write to file
						string data = recorder.GetData();
						uint32 size = uint32(data.size());
						record_state_file.write((char *)&size, sizeof(size));
						record_state_file.write(data.data(), size);
					}
					else if (validate_state)
					{
						// Read state
						uint32 size = 0;
						validate_state_file.read((char *)&size, sizeof(size));
						string data;
						data.resize(size);
						validate_state_file.read(data.data(), size);

						// Copy to validator
						StateRecorderImpl validator;
						validator.WriteBytes(data.data(), size);

						// Validate state
						validator.SetValidating(true);
						physics_system.RestoreState(validator);
					}

				#ifdef JPH_ENABLE_DETERMINISM_LOG
					const BodyLockInterface &bli = physics_system.GetBodyLockInterfaceNoLock();
					BodyIDVector body_ids;
					physics_system.GetBodies(body_ids);
					for (BodyID id : body_ids)
					{
						BodyLockRead lock(bli, id);
						const Body &body = lock.GetBody();
						if (!body.IsStatic())
							JPH_DET_LOG(id << ": p: " << body.GetPosition() << " r: " << body.GetRotation() << " v: " << body.GetLinearVelocity() << " w: " << body.GetAngularVelocity());
					}
				#endif // JPH_ENABLE_DETERMINISM_LOG
				}

				// Calculate hash of all positions and rotations of the bodies
				uint64 hash = HashBytes(nullptr, 0); // Ensure we start with the proper seed
				BodyInterface &bi = physics_system.GetBodyInterfaceNoLock();
				BodyIDVector body_ids;
				physics_system.GetBodies(body_ids);
				for (BodyID id : body_ids)
				{
					RVec3 pos = bi.GetPosition(id);
					hash = HashBytes(&pos, 3 * sizeof(Real), hash);
					Quat rot = bi.GetRotation(id);
					hash = HashBytes(&rot, sizeof(Quat), hash);
				}

				// Let the scene hash its own state
				scene->UpdateHash(hash);

				// Convert hash to string
				stringstream hash_stream;
				hash_stream << "0x" << hex << hash << dec;
				string hash_str = hash_stream.str();

				// Stop test scene
				scene->StopTest(physics_system);

				// Trace stat line
				Trace("%s, %d, %f, %s", motion_quality_str.c_str(), num_threads + 1, double(max_iterations) / (1.0e-9 * total_duration.count()), hash_str.c_str());

				// Check hash code
				if (validate_hash != nullptr && hash_str != validate_hash)
				{
					Trace("Fail hash validation. Was: %s, expected: %s", hash_str.c_str(), validate_hash);
					return 1;
				}
			}
		}
	}

#ifdef JPH_TRACK_NARROWPHASE_STATS
	NarrowPhaseStat::sReportStats();
#endif // JPH_TRACK_NARROWPHASE_STATS

	// Unregisters all types with the factory and cleans up the default material
	UnregisterTypes();

	// Destroy the factory
	delete Factory::sInstance;
	Factory::sInstance = nullptr;

	// End profiling this program
	JPH_PROFILE_END();

	return 0;
}

#ifdef JPH_PLATFORM_ANDROID

// Main entry point for android
void android_main(struct android_app *ioApp)
{
	// Run the regular main function
	const char *args[] = { "Unused", "-s=ConvexVsMesh", "-t=max" };
	main(size(args), (char **)args);
}

#endif // JPH_PLATFORM_ANDROID
