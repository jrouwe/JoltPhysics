// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

namespace JPH {

/// A mutex array protects a number of resources with a limited amount of mutexes.
/// It uses hashing to find the mutex of a particular object.
/// The idea is that if the amount of threads is much smaller than the amount of mutexes
/// that there is a relatively small chance that two different objects map to the same mutex.
template <class MutexType, int NumMutexesArg>
class MutexArray
{
public:
	/// Number of mutexes used to protect the underlying resources.
	static constexpr int	NumMutexes = NumMutexesArg;

	/// Convert an object index to a mutex index
	inline uint32			GetMutexIndex(uint32 inObjectIndex) const
	{
		std::hash<uint32> hasher;
		static_assert(IsPowerOf2(NumMutexes), "Number of mutexes must be power of 2");
		return hasher(inObjectIndex) & (NumMutexes - 1);
	}

	/// Get the mutex belonging to a certain object by index
	inline MutexType &		GetMutexByObjectIndex(uint32 inObjectIndex)
	{
		return mMutexStorage[GetMutexIndex(inObjectIndex)].mMutex;
	}

	/// Get a mutex by index in the array
	inline MutexType &		GetMutexByIndex(uint32 inMutexIndex)
	{
		return mMutexStorage[inMutexIndex].mMutex;
	}

	/// Lock all mutexes
	void					LockAll()
	{
		JPH_PROFILE_FUNCTION();

		for (MutexStorage &m : mMutexStorage)
			m.mMutex.lock();
	}

	/// Unlock all mutexes
	void					UnlockAll()
	{
		JPH_PROFILE_FUNCTION();

		for (MutexStorage &m : mMutexStorage)
			m.mMutex.unlock();
	}

private:
	/// Align the mutex to a cache line to ensure there is no false sharing (this is platform dependent, we do this to be safe)
	struct alignas(JPH_CACHE_LINE_SIZE) MutexStorage
	{
		MutexType			mMutex;
	};

	MutexStorage			mMutexStorage[NumMutexes];
};

} // JPH

