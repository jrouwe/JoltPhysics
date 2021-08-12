// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <mutex>
#include <shared_mutex>
#include <thread>
#include <Core/Profiler.h>

namespace JPH {

#if defined(JPH_ENABLE_ASSERTS) || defined(JPH_PROFILE_ENABLED) || defined(JPH_EXTERNAL_PROFILE)

/// Very simple wrapper around std::mutex which tracks lock contention in the profiler 
/// and asserts that locks/unlocks take place on the same thread
class Mutex : public mutex
{
public:
	inline bool		try_lock()
	{
		JPH_ASSERT(mLockedThreadID != this_thread::get_id());
		if (mutex::try_lock())
		{
			JPH_IF_ENABLE_ASSERTS(mLockedThreadID = this_thread::get_id();)
			return true;
		}
		return false;
	}

	inline void		lock()
	{
		if (!try_lock())
		{
			JPH_PROFILE("Lock", 0xff00ffff);
			mutex::lock();
			JPH_IF_ENABLE_ASSERTS(mLockedThreadID = this_thread::get_id();)
		}
	}

	inline void		unlock()
	{
		JPH_ASSERT(mLockedThreadID == this_thread::get_id());
		JPH_IF_ENABLE_ASSERTS(mLockedThreadID = thread::id();)
		mutex::unlock();
	}

#ifdef JPH_ENABLE_ASSERTS
	inline bool		is_locked()
	{
		return mLockedThreadID != thread::id();
	}
#endif // JPH_ENABLE_ASSERTS

private:
	JPH_IF_ENABLE_ASSERTS(thread::id mLockedThreadID;)
};

/// Very simple wrapper around std::shared_mutex which tracks lock contention in the profiler
/// and asserts that locks/unlocks take place on the same thread
class SharedMutex : public shared_mutex
{
public:
	inline bool		try_lock()
	{
		JPH_ASSERT(mLockedThreadID != this_thread::get_id());
		if (shared_mutex::try_lock())
		{
			JPH_IF_ENABLE_ASSERTS(mLockedThreadID = this_thread::get_id();)
			return true;
		}
		return false;
	}

	inline void		lock()
	{
		if (!try_lock())
		{
			JPH_PROFILE("Lock", 0xff00ffff);
			shared_mutex::lock();
			JPH_IF_ENABLE_ASSERTS(mLockedThreadID = this_thread::get_id();)
		}
	}

	inline void		unlock()
	{
		JPH_ASSERT(mLockedThreadID == this_thread::get_id());
		JPH_IF_ENABLE_ASSERTS(mLockedThreadID = thread::id();)
		shared_mutex::unlock();
	}

#ifdef JPH_ENABLE_ASSERTS
	inline bool		is_locked()
	{
		return mLockedThreadID != thread::id();
	}
#endif // JPH_ENABLE_ASSERTS

	inline void		lock_shared()
	{
		if (!try_lock_shared())
		{
			JPH_PROFILE("LockShared", 0xff00ffff);
			shared_mutex::lock_shared();
		}
	}

private:
	JPH_IF_ENABLE_ASSERTS(thread::id mLockedThreadID;)
};

#else

using Mutex = mutex;
using SharedMutex = shared_mutex;

#endif

} // JPH