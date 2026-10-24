// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Core/JobSystemWithBarrier.h>
#include <Jolt/Core/FixedSizeFreeList.h>
#include <Jolt/Core/Semaphore.h>

JPH_SUPPRESS_WARNINGS_STD_BEGIN
#include <thread>
JPH_SUPPRESS_WARNINGS_STD_END

JPH_NAMESPACE_BEGIN

// Things we're using from STL
using std::thread;

/// Implementation of a JobSystem using a thread pool
///
/// Note that this is considered an example implementation. It is expected that when you integrate
/// the physics engine into your own project that you'll provide your own implementation of the
/// JobSystem built on top of whatever job system your project uses.
class JPH_EXPORT JobSystemThreadPool final : public JobSystemWithBarrier
{
public:
	JPH_OVERRIDE_NEW_DELETE

	/// Creates a thread pool.
	/// @see JobSystemThreadPool::Init
							JobSystemThreadPool(uint inMaxJobs, uint inMaxBarriers, int inNumThreads = -1);
							JobSystemThreadPool() = default;
	virtual					~JobSystemThreadPool() override;

	/// Functions to call when a thread is initialized or exits, must be set before calling Init()
	using InitExitFunction = function<void(int)>;
	void					SetThreadInitFunction(const InitExitFunction &inInitFunction)	{ mThreadInitFunction = inInitFunction; }
	void					SetThreadExitFunction(const InitExitFunction &inExitFunction)	{ mThreadExitFunction = inExitFunction; }

	/// Initialize the thread pool
	/// @param inMaxJobs Max number of jobs that can be allocated at any time (note that the actual number of jobs that can be queued is inMaxJobs - 1 because the queue always contains a dummy node)
	/// @param inMaxBarriers Max number of barriers that can be allocated at any time
	/// @param inNumThreads Number of threads to start (the number of concurrent jobs is 1 more because the main thread will also run jobs while waiting for a barrier to complete). Use -1 to auto detect the amount of CPU's.
	void					Init(uint inMaxJobs, uint inMaxBarriers, int inNumThreads = -1);

	// See JobSystem
	virtual int				GetMaxConcurrency() const override				{ return int(mThreads.size()) + 1; }
	virtual JobHandle		CreateJob(const char *inName, ColorArg inColor, const JobFunction &inJobFunction, uint32 inNumDependencies = 0) override;
	virtual void			CreateJobs(const char *inName, ColorArg inColor, const JobFunction &inJobFunction, JobHandle *outJobs, uint32 inNumJobs, uint32 inNumDependencies = 0) override;

	/// Change the max concurrency after initialization
	void					SetNumThreads(int inNumThreads)					{ StopThreads(); StartThreads(inNumThreads); }

protected:
	// See JobSystem
	virtual void			QueueJob(Job *inJob) override;
	virtual void			QueueJobs(Job **inJobs, uint inNumJobs) override;
	virtual void			FreeJob(Job *inJob) override;

private:
	/// A job that is also a node of the queue. The queue is a Michael-Scott multi producer multi consumer queue.
	/// See 'Simple, Fast, and Practical Non-Blocking and Blocking Concurrent Queue Algorithms'.
	/// Pointers are stored as an index in mJobs combined with a modification count (to prevent the ABA problem) packed in a uint64.
	class Node : public Job
	{
	public:
							Node(const char *inJobName, ColorArg inColor, JobSystem *inJobSystem, const JobFunction &inJobFunction, uint32 inNumDependencies)
			: Job(inJobName, inColor, inJobSystem, inJobFunction, inNumDependencies)
		{
			// Work around TSAN false positive. We can read mNext from a freed and then recyled node. So instead of constructing, we atomically intialize mNext to 0
			mNext.store(0, std::memory_order_relaxed);
		}

		atomic<uint64>		mNext;											///< Index to the next node in the queue and counter
	};

	static constexpr uint32	cNullIndex = 0xffffffff;
	static uint64			sMakePointer(uint32 inIndex, uint32 inCount)	{ return (uint64(inCount) << 32) | inIndex; }
	static uint32			sGetIndex(uint64 inPointer)						{ return uint32(inPointer); }
	static uint32			sGetCount(uint64 inPointer)						{ return uint32(inPointer >> 32); }

	/// Allocate a job
	Job *					AllocateJob(const char *inName, ColorArg inColor, const JobFunction &inJobFunction, uint32 inNumDependencies);

	/// Add a job to the queue
	void					Enqueue(Job *inJob);

	/// Take a job from the queue, returns nullptr when the queue is empty. The caller must Release() the returned job after executing it.
	Job *					Dequeue();

	/// Start/stop the worker threads
	void					StartThreads(int inNumThreads);
	void					StopThreads();

	/// Entry point for a thread
	void					ThreadMain(int inThreadIndex);

	/// Execute all jobs in the queue
	void					ExecuteJobs();

	/// Functions to call when initializing or exiting a thread
	InitExitFunction		mThreadInitFunction = [](int) { };
	InitExitFunction		mThreadExitFunction = [](int) { };

	/// Array of jobs (fixed size)
	using AvailableJobs = FixedSizeFreeList<Node>;
	AvailableJobs			mJobs;

	/// Threads running jobs
	Array<thread>			mThreads;

	// Head and tail of the queue, these are index + counters (see sMakePointer)
	alignas(JPH_CACHE_LINE_SIZE) atomic<uint64> mHead;
	alignas(JPH_CACHE_LINE_SIZE) atomic<uint64> mTail;

	// Semaphore used to signal worker threads that there is new work
	Semaphore				mSemaphore;

	/// Boolean to indicate that we want to stop the job system
	atomic<bool>			mQuit = false;
};

JPH_NAMESPACE_END
