// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Core/JobSystemWithBarrier.h>
#include <Jolt/Core/Profiler.h>

JPH_SUPPRESS_WARNINGS_STD_BEGIN
#include <thread>
JPH_SUPPRESS_WARNINGS_STD_END

JPH_NAMESPACE_BEGIN

JobSystemWithBarrier::BarrierImpl::BarrierImpl()
{
	for (atomic<Job *> &j : mJobs)
		j.store(nullptr, memory_order_relaxed);
}

JobSystemWithBarrier::BarrierImpl::~BarrierImpl()
{
	JPH_ASSERT(IsEmpty());
}

void JobSystemWithBarrier::BarrierImpl::AddJob(const JobHandle &inJob)
{
	JPH_PROFILE_FUNCTION();

	// First increment mNumToAcquire so that the barrier knows to wait for it. If we do this after SetBarrier, it is possible for
	// the job to finish and call OnJobFinished before we increment mNumToAcquire, which would cause WaitForJobs to return too early.
	mNumToAcquire.fetch_add(1, memory_order_relaxed);

	// Set the barrier on the job, this returns true if the barrier was successfully set (otherwise the job is already done and we don't need to add it to our list)
	Job *job = inJob.GetPtr();
	if (job->SetBarrier(this))
	{
		bool release_semaphore = false;

		// If the job can be executed we want to release the semaphore an extra time to allow the waiting thread to start executing it
		if (job->CanBeExecuted())
		{
			release_semaphore = true;
			mNumToAcquire.fetch_add(1, memory_order_relaxed);
		}

		// Add the job to our job list
		job->AddRef();
		uint write_index = mJobWriteIndex.fetch_add(1, memory_order_relaxed);
		while (write_index - mJobReadIndex.load(memory_order_acquire) >= cMaxJobs)
		{
			JPH_ASSERT(false, "Barrier full, stalling!");
			std::this_thread::sleep_for(std::chrono::microseconds(100));
		}
		mJobs[write_index & (cMaxJobs - 1)].store(job, memory_order_release);

		// Notify waiting thread that a new executable job is available
		if (release_semaphore)
			mSemaphore.Release();
	}
	else
	{
		// Job was already done, we didn't add it to the barrier so we need to decrement mNumToAcquire again.
		// Note that this could cause Wait() to hang forever if the job finished in the mean time and the waiting
		// thread looped and is waiting for the semaphore again. However, AddJob can only be called either before
		// Wait is called or from a job that is part of the barrier, in which case that job is guaranteed
		// to wake up the semaphore when it finishes.
		mNumToAcquire.fetch_sub(1, memory_order_relaxed);
	}
}

void JobSystemWithBarrier::BarrierImpl::AddJobs(const JobHandle *inHandles, uint inNumHandles)
{
	JPH_PROFILE_FUNCTION();

	bool release_semaphore = false;

	for (const JobHandle *handle = inHandles, *handles_end = inHandles + inNumHandles; handle < handles_end; ++handle)
	{
		// First increment mNumToAcquire so that the barrier knows to wait for it. If we do this after SetBarrier, it is possible for
		// the job to finish and call OnJobFinished before we increment mNumToAcquire, which would cause WaitForJobs to return too early.
		mNumToAcquire.fetch_add(1, memory_order_relaxed);

		// Set the barrier on the job, this returns true if the barrier was successfully set (otherwise the job is already done and we don't need to add it to our list)
		Job *job = handle->GetPtr();
		if (job->SetBarrier(this))
		{
			// If the job can be executed we want to release the semaphore an extra time to allow the waiting thread to start executing it
			if (!release_semaphore && job->CanBeExecuted())
			{
				release_semaphore = true;
				mNumToAcquire.fetch_add(1, memory_order_relaxed);
			}

			// Add the job to our job list
			job->AddRef();
			uint write_index = mJobWriteIndex.fetch_add(1, memory_order_relaxed);
			while (write_index - mJobReadIndex.load(memory_order_acquire) >= cMaxJobs)
			{
				JPH_ASSERT(false, "Barrier full, stalling!");
				std::this_thread::sleep_for(std::chrono::microseconds(100));
			}
			mJobs[write_index & (cMaxJobs - 1)].store(job, memory_order_release);
		}
		else
		{
			// Job was already done, we didn't add it to the barrier so we need to decrement mNumToAcquire again.
			// Note that this could cause Wait() to hang forever if the job finished in the mean time and the waiting
			// thread looped and is waiting for the semaphore again. However, AddJob can only be called either before
			// Wait is called or from a job that is part of the barrier, in which case that job is guaranteed
			// to wake up the semaphore when it finishes.
			mNumToAcquire.fetch_sub(1, memory_order_relaxed);
		}
	}

	// Notify waiting thread that a new executable job is available
	if (release_semaphore)
		mSemaphore.Release();
}

void JobSystemWithBarrier::BarrierImpl::OnJobFinished(Job *inJob)
{
	JPH_PROFILE_FUNCTION();

	mSemaphore.Release();
}

void JobSystemWithBarrier::BarrierImpl::Wait()
{
	while (mNumToAcquire.load(memory_order_relaxed) > 0)
	{
		{
			JPH_PROFILE("Execute Jobs");

			// Go through all jobs
			bool has_executed;
			do
			{
				has_executed = false;

				// Loop through the jobs and erase jobs from the beginning of the list that are done
				// Note that we use != instead of < for all index comparisons since the indices can wrap around
				while (mJobReadIndex.load(memory_order_relaxed) != mJobWriteIndex.load(memory_order_relaxed))
				{
					atomic<Job *> &job = mJobs[mJobReadIndex.load(memory_order_relaxed) & (cMaxJobs - 1)];
					Job *job_ptr = job.load(memory_order_acquire); // Acquire so that we see the contents of the job
					if (job_ptr == nullptr || !job_ptr->IsDone())
						break;

					// Job is finished, release it
					job_ptr->Release();
					job.store(nullptr, memory_order_relaxed);
					mJobReadIndex.fetch_add(1, memory_order_release); // Release so that the nullptr store above is ordered before a writer reusing this slot
				}

				// Loop through the jobs and execute the first executable job
				for (uint index = mJobReadIndex.load(memory_order_relaxed), end_index = mJobWriteIndex.load(memory_order_relaxed); index != end_index; ++index)
				{
					const atomic<Job *> &job = mJobs[index & (cMaxJobs - 1)];
					Job *job_ptr = job.load(memory_order_acquire); // Acquire so that we see the contents of the job
					if (job_ptr != nullptr && job_ptr->CanBeExecuted())
					{
						// This will only execute the job if it has not already executed
						job_ptr->Execute();
						has_executed = true;
						break;
					}
				}

			} while (has_executed);
		}

		// Wait for another thread to wake us when either there is more work to do or when all jobs have completed.
		// When there have been multiple releases, we acquire them all at the same time to avoid needlessly spinning on executing jobs.
		// Note that using GetValue is inherently unsafe since we can read a stale value, but this is not an issue here as this is the only
		// place where we acquire the semaphore. Other threads only release it, so we can only read a value that is lower or equal to the actual value.
		int num_to_acquire = max(1, mSemaphore.GetValue());
		mSemaphore.Acquire(num_to_acquire);
		mNumToAcquire.fetch_sub(num_to_acquire, memory_order_relaxed);
	}

	JPH_ASSERT(mSemaphore.GetValue() == 0);
	JPH_ASSERT(mNumToAcquire.load(memory_order_relaxed) == 0);

	// All jobs should be done now, release them
	while (mJobReadIndex.load(memory_order_relaxed) != mJobWriteIndex.load(memory_order_relaxed))
	{
		atomic<Job *> &job = mJobs[mJobReadIndex.load(memory_order_relaxed) & (cMaxJobs - 1)];
		Job *job_ptr = job.load(memory_order_acquire); // Acquire so that we see the contents of the job
		JPH_ASSERT(job_ptr != nullptr && job_ptr->IsDone());
		job_ptr->Release();
		job.store(nullptr, memory_order_relaxed);
		mJobReadIndex.fetch_add(1, memory_order_release); // Release so that the nullptr store above is ordered before a writer reusing this slot
	}
}

void JobSystemWithBarrier::Init(uint inMaxBarriers)
{
	JPH_ASSERT(mBarriers == nullptr); // Already initialized?

	// Init freelist of barriers
	mMaxBarriers = inMaxBarriers;
	mBarriers = new BarrierImpl [inMaxBarriers];
}

JobSystemWithBarrier::JobSystemWithBarrier(uint inMaxBarriers)
{
	Init(inMaxBarriers);
}

JobSystemWithBarrier::~JobSystemWithBarrier()
{
	// Ensure that none of the barriers are used
#ifdef JPH_ENABLE_ASSERTS
	for (const BarrierImpl *b = mBarriers, *b_end = mBarriers + mMaxBarriers; b < b_end; ++b)
		JPH_ASSERT(!b->mInUse);
#endif // JPH_ENABLE_ASSERTS
	delete [] mBarriers;
}

JobSystem::Barrier *JobSystemWithBarrier::CreateBarrier()
{
	JPH_PROFILE_FUNCTION();

	// Find the first unused barrier
	for (uint32 index = 0; index < mMaxBarriers; ++index)
	{
		bool expected = false;
		if (mBarriers[index].mInUse.compare_exchange_strong(expected, true, memory_order_acquire, memory_order_relaxed))
			return &mBarriers[index];
	}

	return nullptr;
}

void JobSystemWithBarrier::DestroyBarrier(Barrier *inBarrier)
{
	JPH_PROFILE_FUNCTION();

	// Check that no jobs are in the barrier
	JPH_ASSERT(static_cast<BarrierImpl *>(inBarrier)->IsEmpty());

	// Flag the barrier as unused
	bool expected = true;
	static_cast<BarrierImpl *>(inBarrier)->mInUse.compare_exchange_strong(expected, false, memory_order_release, memory_order_relaxed);
	JPH_ASSERT(expected);
}

void JobSystemWithBarrier::WaitForJobs(Barrier *inBarrier)
{
	JPH_PROFILE_FUNCTION();

	// Let our barrier implementation wait for the jobs
	static_cast<BarrierImpl *>(inBarrier)->Wait();
}

JPH_NAMESPACE_END
