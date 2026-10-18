// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/Profiler.h>
#include <Jolt/Core/FPException.h>
#include <Jolt/Core/IncludeWindows.h>

#ifdef JPH_PLATFORM_LINUX
	#include <sys/prctl.h>
#endif

JPH_NAMESPACE_BEGIN

void JobSystemThreadPool::Init(uint inMaxJobs, uint inMaxBarriers, int inNumThreads)
{
	JobSystemWithBarrier::Init(inMaxBarriers);

	// Init freelist of jobs
	mJobs.Init(inMaxJobs, inMaxJobs);

	// Start the worker threads
	StartThreads(inNumThreads);
}

JobSystemThreadPool::JobSystemThreadPool(uint inMaxJobs, uint inMaxBarriers, int inNumThreads)
{
	Init(inMaxJobs, inMaxBarriers, inNumThreads);
}

void JobSystemThreadPool::StartThreads([[maybe_unused]] int inNumThreads)
{
#if !defined(JPH_CPU_WASM) || defined(__EMSCRIPTEN_PTHREADS__) // If we're running without threads support we cannot create threads and we ignore the inNumThreads parameter
	// Auto detect number of threads
	if (inNumThreads < 0)
		inNumThreads = thread::hardware_concurrency() - 1;

	// If no threads are requested we're done
	if (inNumThreads == 0)
		return;

	// Don't quit the threads
	mQuit.store(false, std::memory_order_relaxed);

	// Reset the queue: create the dummy node that is the only node in the queue, the queue holds a reference to it
	Node *dummy = &mJobs.Get(mJobs.ConstructObject("Dummy", Color::sBlack, this, JobFunction(), 0));
	dummy->AddRef();
	dummy->mNext.store(sMakePointer(cNullIndex, 0), std::memory_order_relaxed);
	uint64 dummy_pointer = sMakePointer(mJobs.GetObjectIndex(dummy), 0);
	mHead.store(dummy_pointer, std::memory_order_relaxed);
	mTail.store(dummy_pointer, std::memory_order_relaxed);

	// Start running threads
	JPH_ASSERT(mThreads.empty());
	mThreads.reserve(inNumThreads);
	for (int i = 0; i < inNumThreads; ++i)
		mThreads.emplace_back([this, i] { ThreadMain(i); });
#endif
}

JobSystemThreadPool::~JobSystemThreadPool()
{
	// Stop all worker threads
	StopThreads();
}

void JobSystemThreadPool::StopThreads()
{
	if (mThreads.empty())
		return;

	// Signal threads that we want to stop and wake them up
	mQuit.store(true, std::memory_order_relaxed);
	mSemaphore.Release((uint)mThreads.size());

	// Wait for all threads to finish
	for (thread &t : mThreads)
		if (t.joinable())
			t.join();

	// Delete all threads
	mThreads.clear();

	// Ensure that there are no lingering jobs in the queue
	ExecuteJobs();

	// Destruct the dummy job
	uint64 dummy = mTail.load(std::memory_order_relaxed);
	mJobs.DestructObject(sGetIndex(dummy));
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
		std::this_thread::sleep_for(std::chrono::microseconds(100));
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
	mJobs.DestructObject(static_cast<Node *>(inJob));
}

void JobSystemThreadPool::Enqueue(Job *inJob)
{
	Node *node = static_cast<Node *>(inJob);
	uint32 node_index = mJobs.GetObjectIndex(node);

	// Add references to the job: one for the queue (held until the node is no longer the dummy node) and one for the thread that will execute it
	node->AddRef(2);

	// Set the next pointer to null, the count must be different from any previous use of this node
	node->mNext.store(sMakePointer(cNullIndex, sGetCount(mTail.load(std::memory_order_relaxed))), std::memory_order_relaxed);

	for (;;)
	{
		uint64 tail = mTail.load(std::memory_order_acquire);
		Node &tail_node = mJobs.Get(sGetIndex(tail));
		uint64 next = tail_node.mNext.load(std::memory_order_acquire); // Node could be deleted but since we used a fixed freelist, the memory is still accessible
		if (tail == mTail.load(std::memory_order_relaxed)) // Are tail and next consistent?
		{
			if (sGetIndex(next) == cNullIndex) // Was tail pointing to the last node?
			{
				// Try to link the node at the end of the list
				if (tail_node.mNext.compare_exchange_weak(next, sMakePointer(node_index, sGetCount(next) + 1), std::memory_order_release, std::memory_order_relaxed))
				{
					// Enqueue is done, try to swing tail to the inserted node
					mTail.compare_exchange_strong(tail, sMakePointer(node_index, sGetCount(tail) + 1), std::memory_order_release, std::memory_order_relaxed);
					return;
				}
			}
			else
			{
				// Tail was not pointing to the last node, try to swing it to the next node
				mTail.compare_exchange_weak(tail, sMakePointer(sGetIndex(next), sGetCount(tail) + 1), std::memory_order_release, std::memory_order_relaxed);
			}
		}
	}
}

JobSystem::Job *JobSystemThreadPool::Dequeue()
{
	for (;;)
	{
		uint64 head = mHead.load(std::memory_order_acquire);
		uint64 tail = mTail.load(std::memory_order_acquire);
		uint64 next = mJobs.Get(sGetIndex(head)).mNext.load(std::memory_order_acquire); // Node could be deleted but since we used a fixed freelist, the memory is still accessible
		if (head == mHead.load(std::memory_order_relaxed)) // Are head, tail and next consistent?
		{
			if (sGetIndex(head) == sGetIndex(tail)) // Is queue empty or tail falling behind?
			{
				if (sGetIndex(next) == cNullIndex)
					return nullptr; // Queue is empty

				// Tail is falling behind, try to advance it
				mTail.compare_exchange_weak(tail, sMakePointer(sGetIndex(next), sGetCount(tail) + 1), std::memory_order_release, std::memory_order_relaxed);
			}
			else
			{
				// Try to swing head to the next node, this makes it the new dummy node
				if (mHead.compare_exchange_weak(head, sMakePointer(sGetIndex(next), sGetCount(head) + 1), std::memory_order_release, std::memory_order_relaxed))
				{
					// This node is no longer in the queue so we can release the queue's reference to it
					mJobs.Get(sGetIndex(head)).Release();

					// The caller owns the other reference to the job that was taken in Enqueue
					return &mJobs.Get(sGetIndex(next));
				}
			}
		}
	}
}

void JobSystemThreadPool::QueueJob(Job *inJob)
{
	JPH_PROFILE_FUNCTION();

	// If we have no worker threads, we can't queue the job either. We assume in this case that the job will be added to a barrier and that the barrier will execute the job when it's Wait() function is called.
	if (mThreads.empty())
		return;

	Enqueue(inJob);

	// Wake up thread
	mSemaphore.Release();
}

void JobSystemThreadPool::QueueJobs(Job **inJobs, uint inNumJobs)
{
	JPH_PROFILE_FUNCTION();

	// If we have no worker threads, we can't queue the job either. We assume in this case that the job will be added to a barrier and that the barrier will execute the job when it's Wait() function is called.
	if (mThreads.empty())
		return;

	for (Job **job = inJobs, **job_end = inJobs + inNumJobs; job < job_end; ++job)
		Enqueue(*job);

	// Wake up threads
	mSemaphore.Release(min(inNumJobs, (uint)mThreads.size()));
}

void JobSystemThreadPool::ExecuteJobs()
{
	JPH_PROFILE("Executing Jobs");

	while (Job *job = Dequeue())
	{
		// Execute the associated job
		job->Execute();
		job->Release();
	}
}

#if defined(JPH_PLATFORM_WINDOWS)

#if !defined(JPH_COMPILER_MINGW) // MinGW doesn't support __try/__except)
	// Sets the current thread name in MSVC debugger
	static void RaiseThreadNameException(const char *inName)
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
#endif // !JPH_COMPILER_MINGW

	static void SetThreadName(const char* inName)
	{
		JPH_SUPPRESS_WARNING_PUSH

		// Suppress casting warning, it's fine here as GetProcAddress doesn't really return a FARPROC
		JPH_CLANG_SUPPRESS_WARNING("-Wcast-function-type") // error : cast from 'FARPROC' (aka 'long long (*)()') to 'SetThreadDescriptionFunc' (aka 'long (*)(void *, const wchar_t *)') converts to incompatible function type
		JPH_CLANG_SUPPRESS_WARNING("-Wcast-function-type-strict") // error : cast from 'FARPROC' (aka 'long long (*)()') to 'SetThreadDescriptionFunc' (aka 'long (*)(void *, const wchar_t *)') converts to incompatible function type
		JPH_MSVC_SUPPRESS_WARNING(4191) // reinterpret_cast' : unsafe conversion from 'FARPROC' to 'SetThreadDescriptionFunc'. Calling this function through the result pointer may cause your program to fail

		using SetThreadDescriptionFunc = HRESULT(WINAPI*)(HANDLE hThread, PCWSTR lpThreadDescription);
		static SetThreadDescriptionFunc SetThreadDescription = reinterpret_cast<SetThreadDescriptionFunc>(GetProcAddress(GetModuleHandleW(L"Kernel32.dll"), "SetThreadDescription"));

		JPH_SUPPRESS_WARNING_POP

		if (SetThreadDescription)
		{
			wchar_t name_buffer[64] = { 0 };
			if (MultiByteToWideChar(CP_UTF8, 0, inName, -1, name_buffer, sizeof(name_buffer) / sizeof(wchar_t) - 1) == 0)
				return;

			SetThreadDescription(GetCurrentThread(), name_buffer);
		}
#if !defined(JPH_COMPILER_MINGW)
		else if (IsDebuggerPresent())
			RaiseThreadNameException(inName);
#endif // !JPH_COMPILER_MINGW
	}
#elif defined(JPH_PLATFORM_LINUX)
	static void SetThreadName(const char *inName)
	{
		JPH_ASSERT(strlen(inName) < 16); // String will be truncated if it is longer
		prctl(PR_SET_NAME, inName, 0, 0, 0);
	}
#endif // JPH_PLATFORM_LINUX

void JobSystemThreadPool::ThreadMain(int inThreadIndex)
{
	// Name the thread
	char name[64];
	snprintf(name, sizeof(name), "Worker %d", int(inThreadIndex + 1));

#if defined(JPH_PLATFORM_WINDOWS) || defined(JPH_PLATFORM_LINUX)
	SetThreadName(name);
#endif // JPH_PLATFORM_WINDOWS || JPH_PLATFORM_LINUX

	// Enable floating point exceptions
	FPExceptionsEnable enable_exceptions;
	JPH_UNUSED(enable_exceptions);

	JPH_PROFILE_THREAD_START(name);

	// Call the thread init function
	mThreadInitFunction(inThreadIndex);

	while (!mQuit.load(std::memory_order_relaxed))
	{
		// Wait for jobs
		mSemaphore.Acquire();

		ExecuteJobs();
	}

	// Call the thread exit function
	mThreadExitFunction(inThreadIndex);

	JPH_PROFILE_THREAD_END();
}

JPH_NAMESPACE_END
