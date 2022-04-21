// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/Profiler.h>
#include <Jolt/Core/FPException.h>

JPH_SUPPRESS_WARNINGS_STD_BEGIN
#include <algorithm>
JPH_SUPPRESS_WARNINGS_STD_END

#ifdef JPH_PLATFORM_WINDOWS
	JPH_SUPPRESS_WARNING_PUSH
	JPH_MSVC_SUPPRESS_WARNING(5039) // winbase.h(13179): warning C5039: 'TpSetCallbackCleanupGroup': pointer or reference to potentially throwing function passed to 'extern "C"' function under -EHc. Undefined behavior may occur if this function throws an exception.
	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>
	JPH_SUPPRESS_WARNING_POP
#endif

JPH_NAMESPACE_BEGIN

JobSystemThreadPool::Semaphore::Semaphore()
{
#ifdef JPH_PLATFORM_WINDOWS
	mSemaphore = CreateSemaphore(nullptr, 0, INT_MAX, nullptr);
#endif
}

JobSystemThreadPool::Semaphore::~Semaphore()
{
#ifdef JPH_PLATFORM_WINDOWS
	CloseHandle(mSemaphore);
#endif
}

void JobSystemThreadPool::Semaphore::Release(uint inNumber)
{
	JPH_ASSERT(inNumber > 0);

#ifdef JPH_PLATFORM_WINDOWS
	int old_value = mCount.fetch_add(inNumber);
	if (old_value < 0)
	{
		int new_value = old_value + (int)inNumber;
		int num_to_release = min(new_value, 0) - old_value;
		::ReleaseSemaphore(mSemaphore, num_to_release, nullptr);
	}
#else
	lock_guard lock(mLock);
	mCount += (int)inNumber;
	if (inNumber > 1)
		mWaitVariable.notify_all();
	else
		mWaitVariable.notify_one();
#endif
}

void JobSystemThreadPool::Semaphore::Acquire(uint inNumber)
{
	JPH_ASSERT(inNumber > 0);

#ifdef JPH_PLATFORM_WINDOWS
	int old_value = mCount.fetch_sub(inNumber);
	int new_value = old_value - (int)inNumber;
	if (new_value < 0)
	{
		int num_to_acquire = min(old_value, 0) - new_value;
		for (int i = 0; i < num_to_acquire; ++i)
			WaitForSingleObject(mSemaphore, INFINITE);
	}
#else
	unique_lock lock(mLock);
	mCount -= (int)inNumber;
	mWaitVariable.wait(lock, [this]() { return mCount >= 0; });
#endif
}

JobSystemThreadPool::BarrierImpl::BarrierImpl()
{
	for (atomic<Job *> &j : mJobs)
		j = nullptr;
}

JobSystemThreadPool::BarrierImpl::~BarrierImpl()
{
	JPH_ASSERT(IsEmpty());
}

void JobSystemThreadPool::BarrierImpl::AddJob(const JobHandle &inJob)
{
	JPH_PROFILE_FUNCTION();

	bool release_semaphore = false;

	// Set the barrier on the job, this returns true if the barrier was successfully set (otherwise the job is already done and we don't need to add it to our list)
	Job *job = inJob.GetPtr();
	if (job->SetBarrier(this))
	{
		// If the job can be executed we want to release the semaphore an extra time to allow the waiting thread to start executing it
		mNumToAcquire++;
		if (job->CanBeExecuted())
		{
			release_semaphore = true;
			mNumToAcquire++;
		}

		// Add the job to our job list
		job->AddRef();
		uint write_index = mJobWriteIndex++;
		while (write_index - mJobReadIndex >= cMaxJobs)
		{
			JPH_ASSERT(false, "Barrier full, stalling!");
			this_thread::sleep_for(100us);
		}
		mJobs[write_index & (cMaxJobs - 1)] = job;
	}

	// Notify waiting thread that a new executable job is available
	if (release_semaphore)
		mSemaphore.Release();
}

void JobSystemThreadPool::BarrierImpl::AddJobs(const JobHandle *inHandles, uint inNumHandles)
{
	JPH_PROFILE_FUNCTION();

	bool release_semaphore = false;

	for (const JobHandle *handle = inHandles, *handles_end = inHandles + inNumHandles; handle < handles_end; ++handle)
	{
		// Set the barrier on the job, this returns true if the barrier was successfully set (otherwise the job is already done and we don't need to add it to our list)
		Job *job = handle->GetPtr();
		if (job->SetBarrier(this))
		{
			// If the job can be executed we want to release the semaphore an extra time to allow the waiting thread to start executing it
			mNumToAcquire++;
			if (!release_semaphore && job->CanBeExecuted())
			{
				release_semaphore = true;
				mNumToAcquire++;
			}

			// Add the job to our job list
			job->AddRef();
			uint write_index = mJobWriteIndex++;
			while (write_index - mJobReadIndex >= cMaxJobs)
			{
				JPH_ASSERT(false, "Barrier full, stalling!");
				this_thread::sleep_for(100us);
			}
			mJobs[write_index & (cMaxJobs - 1)] = job;
		}
	}

	// Notify waiting thread that a new executable job is available
	if (release_semaphore)
		mSemaphore.Release();
}

void JobSystemThreadPool::BarrierImpl::OnJobFinished(Job *inJob)
{
	JPH_PROFILE_FUNCTION();

	mSemaphore.Release();
}

void JobSystemThreadPool::BarrierImpl::Wait()
{
	while (mNumToAcquire > 0)
	{
		{
			JPH_PROFILE("Execute Jobs");

			// Go through all jobs
			bool has_executed;
			do
			{
				has_executed = false;

				// Loop through the jobs and erase jobs from the beginning of the list that are done
				while (mJobReadIndex < mJobWriteIndex)
				{				
					atomic<Job *> &job = mJobs[mJobReadIndex & (cMaxJobs - 1)];
					Job *job_ptr = job.load();
					if (job_ptr == nullptr || !job_ptr->IsDone())
						break;

					// Job is finished, release it
					job_ptr->Release();
					job = nullptr;
					++mJobReadIndex;
				}

				// Loop through the jobs and execute the first executable job
				for (uint index = mJobReadIndex; index < mJobWriteIndex; ++index)
				{
					const atomic<Job *> &job = mJobs[index & (cMaxJobs - 1)];
					Job *job_ptr = job.load();
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

		// Wait for another thread to wake us when either there is more work to do or when all jobs have completed
		int num_to_acquire = max(1, mSemaphore.GetValue()); // When there have been multiple releases, we acquire them all at the same time to avoid needlessly spinning on executing jobs
		mSemaphore.Acquire(num_to_acquire);
		mNumToAcquire -= num_to_acquire;
	}

	// All jobs should be done now, release them
	while (mJobReadIndex < mJobWriteIndex)
	{				
		atomic<Job *> &job = mJobs[mJobReadIndex & (cMaxJobs - 1)];
		Job *job_ptr = job.load();
		JPH_ASSERT(job_ptr != nullptr && job_ptr->IsDone());
		job_ptr->Release();
		job = nullptr;
		++mJobReadIndex;
	}
}

void JobSystemThreadPool::Init(uint inMaxJobs, uint inMaxBarriers, int inNumThreads)
{
	JPH_ASSERT(mBarriers == nullptr); // Already initialized?

	// Init freelist of barriers
	mMaxBarriers = inMaxBarriers;
	mBarriers = new BarrierImpl [inMaxBarriers];

	// Init freelist of jobs
	mJobs.Init(inMaxJobs, inMaxJobs);

	// Init queue
	for (atomic<Job *> &j : mQueue)
		j = nullptr;

	// Start the worker threads
	StartThreads(inNumThreads);
}

JobSystemThreadPool::JobSystemThreadPool(uint inMaxJobs, uint inMaxBarriers, int inNumThreads)
{
	Init(inMaxJobs, inMaxBarriers, inNumThreads);
}

void JobSystemThreadPool::StartThreads(int inNumThreads)
{
	// Auto detect number of threads
	if (inNumThreads < 0)
		inNumThreads = thread::hardware_concurrency() - 1;

	// If no threads are requested we're done
	if (inNumThreads == 0)
		return;

	// Don't quit the threads
	mQuit = false;

	// Allocate heads
	mHeads = new atomic<uint> [inNumThreads];
	for (int i = 0; i < inNumThreads; ++i)
		mHeads[i] = 0;

	// Start running threads
	JPH_ASSERT(mThreads.empty());
	mThreads.reserve(inNumThreads);
	for (int i = 0; i < inNumThreads; ++i)
	{
		// Name the thread
		char name[64];
		snprintf(name, sizeof(name), "Worker %d", int(i + 1));

		// Create thread
		mThreads.emplace_back([this, name, i] { ThreadMain(name, i); });
	}
}

JobSystemThreadPool::~JobSystemThreadPool()
{
	// Stop all worker threads
	StopThreads();

	// Ensure that none of the barriers are used
#ifdef JPH_ENABLE_ASSERTS
	for (const BarrierImpl *b = mBarriers, *b_end = mBarriers + mMaxBarriers; b < b_end; ++b)
		JPH_ASSERT(!b->mInUse);
#endif // JPH_ENABLE_ASSERTS
	delete [] mBarriers;
}

void JobSystemThreadPool::StopThreads()
{
	if (mThreads.empty())
		return;

	// Signal threads that we want to stop and wake them up
	mQuit = true;
	mSemaphore.Release((uint)mThreads.size());

	// Wait for all threads to finish
	for (thread &t : mThreads)
		if (t.joinable())
			t.join();

	// Delete all threads
	mThreads.clear();

	// Ensure that there are no lingering jobs in the queue
	for (uint head = 0; head != mTail; ++head)
	{
		// Fetch job
		Job *job_ptr = mQueue[head & (cQueueLength - 1)].exchange(nullptr);
		if (job_ptr != nullptr)
		{
			// And execute it
			job_ptr->Execute();
			job_ptr->Release();
		}
	}

	// Destroy heads and reset tail
	delete [] mHeads;
	mHeads = nullptr;
	mTail = 0;
}

JobHandle JobSystemThreadPool::CreateJob(const char *inJobName, ColorArg inColor, const JobFunction &inJobFunction, uint32 inNumDependencies)
{
	JPH_PROFILE_FUNCTION();

	// Loop until we can get a job from the free list
	uint32 index;
	for (;;)
	{
		index = mJobs.ConstructObject(inJobName, inColor, this, inJobFunction, inNumDependencies);
		if (index != AvailableJobs::cInvalidObjectIndex)
			break;
		JPH_ASSERT(false, "No jobs available!");
		this_thread::sleep_for(100us);
	}
	Job *job = &mJobs.Get(index);
	
	// Construct handle to keep a reference, the job is queued below and may immediately complete
	JobHandle handle(job);
	
	// If there are no dependencies, queue the job now
	if (inNumDependencies == 0)
		QueueJob(job);

	// Return the handle
	return handle;
}

void JobSystemThreadPool::FreeJob(Job *inJob)
{
	mJobs.DestructObject(inJob);
}

JobSystem::Barrier *JobSystemThreadPool::CreateBarrier()
{
	JPH_PROFILE_FUNCTION();

	// Find the first unused barrier
	for (uint32 index = 0; index < mMaxBarriers; ++index)
	{
		bool expected = false;
		if (mBarriers[index].mInUse.compare_exchange_strong(expected, true))
			return &mBarriers[index];
	}

	return nullptr;
}

void JobSystemThreadPool::DestroyBarrier(Barrier *inBarrier)
{
	JPH_PROFILE_FUNCTION();

	// Check that no jobs are in the barrier
	JPH_ASSERT(static_cast<BarrierImpl *>(inBarrier)->IsEmpty());

	// Flag the barrier as unused
	bool expected = true;
	static_cast<BarrierImpl *>(inBarrier)->mInUse.compare_exchange_strong(expected, false);
	JPH_ASSERT(expected);
}

void JobSystemThreadPool::WaitForJobs(Barrier *inBarrier)
{
	JPH_PROFILE_FUNCTION();

	// Let our barrier implementation wait for the jobs
	static_cast<BarrierImpl *>(inBarrier)->Wait();
}

uint JobSystemThreadPool::GetHead() const
{
	// Find the minimal value across all threads
	uint head = mTail;
	for (size_t i = 0; i < mThreads.size(); ++i)
		head = min(head, mHeads[i].load());
	return head;
}

void JobSystemThreadPool::QueueJobInternal(Job *inJob)
{
	// Add reference to job because we're adding the job to the queue
	inJob->AddRef();

	// Need to read head first because otherwise the tail can already have passed the head
	// We read the head outside of the loop since it involves iterating over all threads and we only need to update
	// it if there's not enough space in the queue.
	uint head = GetHead();

	for (;;)
	{
		// Check if there's space in the queue
		uint old_value = mTail;
		if (old_value - head >= cQueueLength)
		{
			// We calculated the head outside of the loop, update head (and we also need to update tail to prevent it from passing head)
			head = GetHead();
			old_value = mTail;
	
			// Second check if there's space in the queue
			if (old_value - head >= cQueueLength)
			{
				// Wake up all threads in order to ensure that they can clear any nullptrs they may not have processed yet
				mSemaphore.Release((uint)mThreads.size()); 

				// Sleep a little (we have to wait for other threads to update their head pointer in order for us to be able to continue)
				this_thread::sleep_for(100us);
				continue;
			}
		}

		// Write the job pointer if the slot is empty
		Job *expected_job = nullptr;
		bool success = mQueue[old_value & (cQueueLength - 1)].compare_exchange_strong(expected_job, inJob);

		// Regardless of who wrote the slot, we will update the tail (if the successful thread got scheduled out 
		// after writing the pointer we still want to be able to continue)
		mTail.compare_exchange_strong(old_value, old_value + 1);

		// If we successfully added our job we're done
		if (success)
			break;
	}
}

void JobSystemThreadPool::QueueJob(Job *inJob)
{
	JPH_PROFILE_FUNCTION();

	// If we have no worker threads, we can't queue the job either. We assume in this case that the job will be added to a barrier and that the barrier will execute the job when it's Wait() function is called.
	if (mThreads.empty())
		return;

	// Queue the job
	QueueJobInternal(inJob);

	// Wake up thread
	mSemaphore.Release();
}

void JobSystemThreadPool::QueueJobs(Job **inJobs, uint inNumJobs)
{
	JPH_PROFILE_FUNCTION();

	JPH_ASSERT(inNumJobs > 0);

	// If we have no worker threads, we can't queue the job either. We assume in this case that the job will be added to a barrier and that the barrier will execute the job when it's Wait() function is called.
	if (mThreads.empty())
		return;

	// Queue all jobs
	for (Job **job = inJobs, **job_end = inJobs + inNumJobs; job < job_end; ++job)
		QueueJobInternal(*job);

	// Wake up threads
	mSemaphore.Release(min(inNumJobs, (uint)mThreads.size()));
}

#ifdef JPH_PLATFORM_WINDOWS

// Sets the current thread name in MSVC debugger
static void SetThreadName(const char *inName)
{
	#pragma pack(push, 8)

	struct THREADNAME_INFO
	{
		DWORD	dwType;			// Must be 0x1000.
		LPCSTR	szName;			// Pointer to name (in user addr space).
		DWORD	dwThreadID;		// Thread ID (-1=caller thread).
		DWORD	dwFlags;		// Reserved for future use, must be zero.
	};

	#pragma pack(pop)

	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = inName;
	info.dwThreadID = (DWORD)-1;
	info.dwFlags = 0;

	__try
	{
		RaiseException(0x406D1388, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR *)&info);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
	}
}

#endif

void JobSystemThreadPool::ThreadMain([[maybe_unused]] const char *inName, int inThreadIndex)
{
#ifdef JPH_PLATFORM_WINDOWS
	SetThreadName(inName);
#endif

	// Enable floating point exceptions
	FPExceptionsEnable enable_exceptions;
	JPH_UNUSED(enable_exceptions);

	JPH_PROFILE_THREAD_START(inName);

	atomic<uint> &head = mHeads[inThreadIndex];

	while (!mQuit)
	{
		// Wait for jobs
		mSemaphore.Acquire();

		{
			JPH_PROFILE("Executing Jobs");

			// Loop over the queue
			while (head != mTail)
			{
				// Exchange any job pointer we find with a nullptr
				atomic<Job *> &job = mQueue[head & (cQueueLength - 1)];
				if (job.load() != nullptr)
				{
					Job *job_ptr = job.exchange(nullptr);
					if (job_ptr != nullptr)
					{
						// And execute it
						job_ptr->Execute();
						job_ptr->Release();
					}
				}
				head++;
			}
		}
	}

	JPH_PROFILE_THREAD_END();
}

JPH_NAMESPACE_END
