// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Core/JobSystem.h>
#include <Jolt/Core/FixedSizeFreeList.h>

JPH_SUPPRESS_WARNINGS_STD_BEGIN
#include <thread>
#include <mutex>
#include <condition_variable>
JPH_SUPPRESS_WARNINGS_STD_END

JPH_NAMESPACE_BEGIN

/// Implementation of a JobSystem using a thread pool
/// 
/// Note that this is considered an example implementation. It is expected that when you integrate
/// the physics engine into your own project that you'll provide your own implementation of the
/// JobSystem built on top of whatever job system your project uses.
class JobSystemThreadPool final : public JobSystem
{
public:
	/// Creates a thread pool.
	/// @see JobSystemThreadPool::Init
							JobSystemThreadPool(uint inMaxJobs, uint inMaxBarriers, int inNumThreads = -1);
							JobSystemThreadPool() = default;
	virtual					~JobSystemThreadPool() override;

	/// Initialize the thread pool
	/// @param inMaxJobs Max number of jobs that can be allocated at any time
	/// @param inMaxBarriers Max number of barriers that can be allocated at any time
	/// @param inNumThreads Number of threads to start (the number of concurrent jobs is 1 more because the main thread will also run jobs while waiting for a barrier to complete). Use -1 to autodetect the amount of CPU's.
	void					Init(uint inMaxJobs, uint inMaxBarriers, int inNumThreads = -1);

	// See JobSystem
	virtual int				GetMaxConcurrency() const override				{ return int(mThreads.size()) + 1; }
	virtual JobHandle		CreateJob(const char *inName, ColorArg inColor, const JobFunction &inJobFunction, uint32 inNumDependencies = 0) override;
	virtual Barrier *		CreateBarrier() override;
	virtual void			DestroyBarrier(Barrier *inBarrier) override;
	virtual void			WaitForJobs(Barrier *inBarrier) override;

	/// Change the max concurrency after initialization
	void					SetNumThreads(int inNumThreads)					{ StopThreads(); StartThreads(inNumThreads); }
	
protected:
	// See JobSystem
	virtual void			QueueJob(Job *inJob) override;
	virtual void			QueueJobs(Job **inJobs, uint inNumJobs) override;
	virtual void			FreeJob(Job *inJob) override;

private:
	/// When we switch to C++20 we can use counting_semaphore to unify this
	class Semaphore
	{
	public:
		/// Constructor
		inline				Semaphore();
		inline				~Semaphore();

		/// Release the semaphore, signalling the thread waiting on the barrier that there may be work
		inline void			Release(uint inNumber = 1);

		/// Acquire the semaphore inNumber times
		inline void			Acquire(uint inNumber = 1);

		/// Get the current value of the semaphore
		inline int			GetValue() const								{ return mCount; }

	private:
#ifdef JPH_PLATFORM_WINDOWS
		// On windows we use a semaphore object since it is more efficient than a lock and a condition variable
		alignas(JPH_CACHE_LINE_SIZE) atomic<int> mCount { 0 };				///< We increment mCount for every release, to acquire we decrement the count. If the count is negative we know that we are waiting on the actual semaphore.
		void *				mSemaphore;										///< The semaphore is an expensive construct so we only acquire/release it if we know that we need to wait/have waiting threads
#else
		// Other platforms: Emulate a semaphore using a mutex, condition variable and count
		mutex				mLock;
		condition_variable	mWaitVariable;
		int					mCount = 0;
#endif
	};

	class BarrierImpl : public Barrier
	{
	public:
		/// Constructor
							BarrierImpl();
		virtual				~BarrierImpl() override;

		// See Barrier
		virtual void		AddJob(const JobHandle &inJob) override;
		virtual void		AddJobs(const JobHandle *inHandles, uint inNumHandles) override;

		/// Check if there are any jobs in the job barrier
		inline bool			IsEmpty() const									{ return mJobReadIndex == mJobWriteIndex; }

		/// Wait for all jobs in this job barrier, while waiting, execute jobs that are part of this barrier on the current thread
		void				Wait();

		/// Flag to indicate if a barrier has been handed out
		atomic<bool>		mInUse { false };

	protected:
		/// Called by a Job to mark that it is finished
		virtual void		OnJobFinished(Job *inJob) override;

		/// Jobs queue for the barrier
		static constexpr uint cMaxJobs = 1024;
		static_assert(IsPowerOf2(cMaxJobs));								// We do bit operations and require max jobs to be a power of 2
		atomic<Job *> 		mJobs[cMaxJobs];								///< List of jobs that are part of this barrier, nullptrs for empty slots
		alignas(JPH_CACHE_LINE_SIZE) atomic<uint> mJobReadIndex { 0 };		///< First job that could be valid (modulo cMaxJobs), can be nullptr if other thread is still working on adding the job
		alignas(JPH_CACHE_LINE_SIZE) atomic<uint> mJobWriteIndex { 0 };		///< First job that can be written (modulo cMaxJobs)
		atomic<int>			mNumToAcquire { 0 };							///< Number of times the semaphore has been released, the barrier should acquire the semaphore this many times (written at the same time as mJobWriteIndex so ok to put in same cache line)
		Semaphore			mSemaphore;										///< Semaphore used by finishing jobs to signal the barrier that they're done
	};

	/// Start/stop the worker threads
	void					StartThreads(int inNumThreads);
	void					StopThreads();
	
	/// Entry point for a thread
	void					ThreadMain(const char *inName, int inThreadIndex);

	/// Get the head of the thread that has processed the least amount of jobs
	inline uint				GetHead() const;

	/// Internal helper function to queue a job
	inline void				QueueJobInternal(Job *inJob);

	/// Array of jobs (fixed size)
	using AvailableJobs = FixedSizeFreeList<Job>;
	AvailableJobs			mJobs;

	/// Array of barriers (we keep them constructed all the time since constructing a semaphore/mutex is not cheap)
	uint					mMaxBarriers = 0;								///< Max amount of barriers
	BarrierImpl *			mBarriers = nullptr;							///< List of the actual barriers

	/// Threads running jobs
	vector<thread>			mThreads;

	// The job queue
	static constexpr uint32 cQueueLength = 1024;
	static_assert(IsPowerOf2(cQueueLength));								// We do bit operations and require queue length to be a power of 2
	atomic<Job *>			mQueue[cQueueLength];

	// Head and tail of the queue, do this value modulo cQueueLength - 1 to get the element in the mQueue array
	atomic<uint> *			mHeads = nullptr;								///< Per executing thread the head of the current queue
	alignas(JPH_CACHE_LINE_SIZE) atomic<uint> mTail = 0;					///< Tail (write end) of the queue

	// Semaphore used to signal worker threads that there is new work
	Semaphore				mSemaphore;

	/// Boolean to indicate that we want to stop the job system
	atomic<bool>			mQuit = false;
};

JPH_NAMESPACE_END
