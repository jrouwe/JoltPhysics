// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include <Jolt/Core/JobSystemThreadPool.h>

JPH_SUPPRESS_WARNINGS_STD_BEGIN
#include <thread>
JPH_SUPPRESS_WARNINGS_STD_END

TEST_SUITE("JobSystemTest")
{
	TEST_CASE("TestJobSystemRunJobs")
	{
		// Create job system
		const int cMaxJobs = 127;
		const int cMaxBarriers = 10;
		const int cMaxThreads = 10;
		JobSystemThreadPool system(cMaxJobs + 1 /* Job system reserves 1 job, see documentation */, cMaxBarriers, cMaxThreads);

		// Create array of zeros
		atomic<uint32> values[cMaxJobs];
		for (int i = 0; i < cMaxJobs; ++i)
			values[i] = 0;

		// Create a barrier
		JobSystem::Barrier *barrier = system.CreateBarrier();

		// Create jobs that will increment all values
		for (int i = 0; i < cMaxJobs; ++i)
		{
			JobHandle handle = system.CreateJob("JobTest", Color::sRed, [&values, i] { values[i]++; });
			barrier->AddJob(handle);
		}

		// Wait for the barrier to complete
		system.WaitForJobs(barrier);

		// Destroy our barrier
		system.DestroyBarrier(barrier);

		// Test all values are 1
		for (int i = 0; i < cMaxJobs; ++i)
			CHECK(values[i] == 1);
	}

	TEST_CASE("TestJobSystemRunChain")
	{
		// Create job system
		const int cMaxJobs = 127;
		const int cMaxBarriers = 10;
		JobSystemThreadPool system(cMaxJobs + 1 /* Job system reserves 1 job, see documentation */, cMaxBarriers);

		// Create a barrier
		JobSystem::Barrier *barrier = system.CreateBarrier();

		// Counter that keeps track of order in which jobs ran
		atomic<uint32> counter = 1;

		// Create array of zeros
		atomic<uint32> values[cMaxJobs];
		for (int i = 0; i < cMaxJobs; ++i)
			values[i] = 0;

		// Create jobs that will set sequence number
		JobHandle handles[cMaxJobs];
		for (int i = 0; i < cMaxJobs; ++i)
		{
			handles[i] = system.CreateJob("JobTestChain", Color::sRed, [&values, &counter, &handles, i] {
				// Set sequence number
				values[i] = counter++;

				// Start previous job
				if (i > 0)
					handles[i - 1].RemoveDependency();
			}, 1);

			barrier->AddJob(handles[i]);
		}

		// Start the last job
		handles[cMaxJobs - 1].RemoveDependency();

		// Wait for the barrier to complete
		system.WaitForJobs(barrier);

		// Destroy our barrier
		system.DestroyBarrier(barrier);

		// Test jobs were executed in reverse order
		for (int i = cMaxJobs - 1; i >= 0; --i)
			CHECK(values[i] == cMaxJobs - i);
	}

	// Tests that jobs added to a barrier from within a job don't cause the barrier to release too early. See issue #2198.
	// Note that this only used to fail when explicitly putting a sleep inside JobSystemWithBarrier as specified in the issue.
	TEST_CASE("TestJobSystemCheckBarrierDoesNotReleaseWaitEarly")
	{
		static constexpr int cMaxJobs = 127;
		static constexpr int cMaxBarriers = 10;
		static constexpr int cNumThreads = 4;
		static constexpr int cNumIterations = 50;
		static constexpr uint32 cNumAddedJobs = 4;

		JobSystemThreadPool system(cMaxJobs + 1 /* Job system reserves 1 job, see documentation */, cMaxBarriers, cNumThreads);

		JobSystem::Barrier *barrier = system.CreateBarrier();

		for (int iteration = 0; iteration < cNumIterations; ++iteration)
		{
			// Keep track of whether the adder job is done and if its added jobs are done
			atomic<bool> adder_done = false;
			atomic<uint32> added_jobs_done = 0;

			// Job that adds jobs to the barrier. It has a dependency so that it is not executable while the main thread starts waiting,
			// this ensures that it runs on a worker thread rather than on the waiting thread
			JobHandle adder = system.CreateJob("Adder", Color::sRed, [&system, barrier, &adder_done, &added_jobs_done] {
				// Create jobs without dependencies, these are queued immediately and finish on other worker threads
				JobHandle handles[cNumAddedJobs];
				for (JobHandle &h : handles)
					h = system.CreateJob("Added", Color::sGreen, [&added_jobs_done] { added_jobs_done.fetch_add(1, std::memory_order_relaxed); });
				barrier->AddJobs(handles, cNumAddedJobs);

				// Signal that we're done, this is the last thing that this job does
				adder_done.store(true, std::memory_order_relaxed);
			}, 1);
			barrier->AddJob(adder);

			// Start the job after the main thread has had time to go to sleep in WaitForJobs
			#if !defined(JPH_CPU_WASM) || defined(__EMSCRIPTEN_PTHREADS__)
				std::thread starter([&adder]() {
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
					adder.RemoveDependency();
				});
			#else
				// If we're running without threads support we just start the thread now
				adder.RemoveDependency();
			#endif

			// Wait for the barrier to complete, this should not return until the adding job and all jobs it added have finished
			system.WaitForJobs(barrier);
			bool adder_finished = adder_done.load(std::memory_order_relaxed);
			uint32 added_finished = added_jobs_done.load(std::memory_order_relaxed);
			#if !defined(JPH_CPU_WASM) || defined(__EMSCRIPTEN_PTHREADS__)
				starter.join();
			#endif

			// Check that all jobs have finished, if not the barrier has released too early
			CHECK(adder_finished);
			CHECK(added_finished == cNumAddedJobs);
		}

		system.DestroyBarrier(barrier);
	}
}
