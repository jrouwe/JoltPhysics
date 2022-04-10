// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Core/Mutex.h>

JPH_NAMESPACE_BEGIN

/// This is the list of locks used by the physics engine, they need to be locked in a particular order (from top of the list to bottom of the list) in order to prevent deadlocks
enum class EPhysicsLockTypes
{
	BroadPhaseQuery			= 1 << 0,
	PerBody					= 1 << 1,
	BodiesList				= 1 << 2,
	BroadPhaseUpdate		= 1 << 3,
	ConstraintsList			= 1 << 4,
	ActiveBodiesList		= 1 << 5,
};

/// Helpers to safely lock the different mutexes that are part of the physics system while preventing deadlock
/// Class that keeps track per thread which lock are taken and if the order of locking is correct
class PhysicsLock
{
public:
#ifdef JPH_ENABLE_ASSERTS
	/// Call before taking the lock
	static inline void			sCheckLock(EPhysicsLockTypes inType)
	{
		JPH_ASSERT((uint32)inType > sLockedMutexes, "A lock of same or higher priority was already taken, this can create a deadlock!");
		sLockedMutexes = sLockedMutexes | (uint32)inType;
	}

	/// Call after releasing the lock
	static inline void			sCheckUnlock(EPhysicsLockTypes inType)
	{
		JPH_ASSERT((sLockedMutexes & (uint32)inType) != 0, "Mutex was not locked!");
		sLockedMutexes = sLockedMutexes & ~(uint32)inType;
	}
#endif // !JPH_ENABLE_ASSERTS

	template <class LockType>
	static inline void			sLock(LockType &inMutex, [[maybe_unused]] EPhysicsLockTypes inType)
	{
		JPH_IF_ENABLE_ASSERTS(sCheckLock(inType);)
		inMutex.lock();
	}

	template <class LockType>
	static inline void			sUnlock(LockType &inMutex, [[maybe_unused]] EPhysicsLockTypes inType)
	{
		JPH_IF_ENABLE_ASSERTS(sCheckUnlock(inType);)
		inMutex.unlock();
	}

	template <class LockType>
	static inline void			sLockShared(LockType &inMutex, [[maybe_unused]] EPhysicsLockTypes inType)
	{
		JPH_IF_ENABLE_ASSERTS(sCheckLock(inType);)
		inMutex.lock_shared();
	}

	template <class LockType>
	static inline void			sUnlockShared(LockType &inMutex, [[maybe_unused]] EPhysicsLockTypes inType)
	{
		JPH_IF_ENABLE_ASSERTS(sCheckUnlock(inType);)
		inMutex.unlock_shared();
	}

#ifdef JPH_ENABLE_ASSERTS
private:
	static thread_local uint32	sLockedMutexes;
#endif // !JPH_ENABLE_ASSERTS
};

/// Helper class that is similar to std::unique_lock
template <class LockType>
class UniqueLock : public NonCopyable
{
public:
								UniqueLock(LockType &inLock, EPhysicsLockTypes inType)		: mLock(inLock), mType(inType) { PhysicsLock::sLock(mLock, mType); }
								~UniqueLock()												{ PhysicsLock::sUnlock(mLock, mType); }

private:
	LockType &					mLock;
	EPhysicsLockTypes			mType;
};

/// Helper class that is similar to std::shared_lock
template <class LockType>
class SharedLock : public NonCopyable
{
public:
								SharedLock(LockType &inLock, EPhysicsLockTypes inType)		: mLock(inLock), mType(inType) { PhysicsLock::sLockShared(mLock, mType); }
								~SharedLock()												{ PhysicsLock::sUnlockShared(mLock, mType); }

private:
	LockType &					mLock;
	EPhysicsLockTypes			mType;
};

JPH_NAMESPACE_END
