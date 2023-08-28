// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include <Jolt/Core/JobSystemThreadPool.h>

TEST_SUITE("JobSystemTest")
{
	TEST_CASE("TestJobSystemRunJobs")
	{
		// Create job system
		const int cMaxJobs = 128;
		const int cMaxBarriers = 10;
		const int cMaxThreads = 10;
		JobSystemThreadPool system(cMaxJobs, cMaxBarriers, cMaxThreads);

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
		const int cMaxJobs = 128;
		const int cMaxBarriers = 10;
		JobSystemThreadPool system(cMaxJobs, cMaxBarriers);

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
}
