// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"

#include <Jolt/Core/STLLocalAllocator.h>

TEST_SUITE("STLLocalAllocatorTest")
{
	/// The number of elements in the local buffer
	static constexpr size_t N = 20;

	template <class ArrayType, bool NonTrivial>
	static void sTestArray()
	{
		// Allocate so that we will run out of local memory and reallocate from heap at least once
		ArrayType arr;
		for (int i = 0; i < 64; ++i)
			arr.push_back(i);
		CHECK(arr.size() == 64);
		for (int i = 0; i < 64; ++i)
		{
			CHECK(arr[i] == i);
		#if !defined(JPH_USE_STD_VECTOR) && !defined(JPH_DISABLE_CUSTOM_ALLOCATOR)
			// We only have to move elements once we run out of the local buffer, this happens as we resize
			// from 16 to 32 elements, we'll reallocate again at 32 and 64
			if constexpr (NonTrivial)
				CHECK(arr[i].GetNonTriv() == (i < 16? 3 : (i < 32? 2 : 1)));
		#endif
		}
	#ifndef JPH_DISABLE_CUSTOM_ALLOCATOR
		CHECK(!arr.get_allocator().is_local(arr.data()));
	#endif

		// Check that we can copy the array to another array
		ArrayType arr2;
		arr2 = arr;
		for (int i = 0; i < 64; ++i)
		{
			CHECK(arr2[i] == i);
			if constexpr (NonTrivial)
				CHECK(arr2[i].GetNonTriv() == -999);
		}
	#ifndef JPH_DISABLE_CUSTOM_ALLOCATOR
		CHECK(!arr2.get_allocator().is_local(arr2.data()));
	#endif

		// Clear the array
		arr.clear();
		arr.shrink_to_fit();
		CHECK(arr.size() == 0);
	#ifndef JPH_USE_STD_VECTOR
		CHECK(arr.capacity() == 0);
		CHECK(arr.data() == nullptr);
	#endif

		// Allocate so we stay within the local buffer
		for (int i = 0; i < 10; ++i)
			arr.push_back(i);
		CHECK(arr.size() == 10);
		for (int i = 0; i < 10; ++i)
		{
			CHECK(arr[i] == i);
		#if !defined(JPH_USE_STD_VECTOR) && !defined(JPH_DISABLE_CUSTOM_ALLOCATOR)
			// We never need to move elements as they stay within the local buffer
			if constexpr (NonTrivial)
				CHECK(arr[i].GetNonTriv() == 1);
		#endif
		}
	#if !defined(JPH_USE_STD_VECTOR) && !defined(JPH_DISABLE_CUSTOM_ALLOCATOR)
		CHECK(arr.get_allocator().is_local(arr.data()));
	#endif

		// Check that we can copy the array to the local buffer
		ArrayType arr3;
		arr3 = arr;
		for (int i = 0; i < 10; ++i)
		{
			CHECK(arr3[i] == i);
			if constexpr (NonTrivial)
				CHECK(arr3[i].GetNonTriv() == -999);
		}
	#if !defined(JPH_USE_STD_VECTOR) && !defined(JPH_DISABLE_CUSTOM_ALLOCATOR)
		CHECK(arr3.get_allocator().is_local(arr3.data()));
	#endif

		// Check that if we reserve the memory, that we can fully fill the array in local memory
		ArrayType arr4;
		arr4.reserve(N);
		for (int i = 0; i < int(N); ++i)
			arr4.push_back(i);
		CHECK(arr4.size() == N);
		for (int i = 0; i < int(N); ++i)
		{
			CHECK(arr4[i] == i);
			if constexpr (NonTrivial)
				CHECK(arr4[i].GetNonTriv() == 1);
		}
	#ifndef JPH_DISABLE_CUSTOM_ALLOCATOR
		CHECK(arr4.get_allocator().is_local(arr4.data()));
	#endif
	}

	TEST_CASE("TestAllocation")
	{
		using Allocator = STLLocalAllocator<int, N>;
		using ArrayType = Array<int, Allocator>;
	#ifndef JPH_DISABLE_CUSTOM_ALLOCATOR
		static_assert(!Allocator::needs_aligned_allocate);
		static_assert(AllocatorHasReallocate<Allocator>::sValue);
	#endif

		sTestArray<ArrayType, false>();
	}

	TEST_CASE("TestAllocationAligned")
	{
		// Force the need for an aligned allocation
		struct alignas(64) Aligned
		{
						Aligned(int inValue)	: mValue(inValue) { }
			operator	int() const				{ return mValue; }

		private:
			int			mValue;
		};
		static_assert(std::is_trivially_copyable<Aligned>());

		using Allocator = STLLocalAllocator<Aligned, N>;
		using ArrayType = Array<Aligned, Allocator>;
	#ifndef JPH_DISABLE_CUSTOM_ALLOCATOR
		static_assert(Allocator::needs_aligned_allocate);
		static_assert(AllocatorHasReallocate<Allocator>::sValue);
	#endif

		sTestArray<ArrayType, false>();
	}

	TEST_CASE("TestAllocationNonTrivial")
	{
		// Force non trivial copy constructor
		struct NonTriv
		{
						NonTriv(int inValue)				: mValue(inValue) { }
						NonTriv(const NonTriv &inRHS)		: mValue(inRHS.mValue), mMakeNonTriv(-999) { }
						NonTriv(NonTriv &&inRHS)			: mValue(inRHS.mValue), mMakeNonTriv(inRHS.mMakeNonTriv + 1) { }
			NonTriv &	operator = (const NonTriv &inRHS)	{ mValue = inRHS.mValue; mMakeNonTriv = -9999; return *this; }
			operator	int() const							{ return mValue; }
			int			GetNonTriv() const					{ return mMakeNonTriv; }

		private:
			int			mValue;
			int			mMakeNonTriv = 0;
		};
		static_assert(!std::is_trivially_copyable<NonTriv>());

		using Allocator = STLLocalAllocator<NonTriv, N>;
		using ArrayType = Array<NonTriv, Allocator>;
	#ifndef JPH_DISABLE_CUSTOM_ALLOCATOR
		static_assert(!Allocator::needs_aligned_allocate);
		static_assert(AllocatorHasReallocate<Allocator>::sValue);
	#endif

		sTestArray<ArrayType, true>();
	}

	TEST_CASE("TestAllocationAlignedNonTrivial")
	{
		// Force non trivial copy constructor
		struct alignas(64) AlNonTriv
		{
						AlNonTriv(int inValue)				: mValue(inValue) { }
						AlNonTriv(const AlNonTriv &inRHS)	: mValue(inRHS.mValue), mMakeNonTriv(-999) { }
						AlNonTriv(AlNonTriv &&inRHS)		: mValue(inRHS.mValue), mMakeNonTriv(inRHS.mMakeNonTriv + 1) { }
			AlNonTriv &	operator = (const AlNonTriv &inRHS) { mValue = inRHS.mValue; mMakeNonTriv = -9999; return *this; }
			operator	int() const							{ return mValue; }
			int			GetNonTriv() const					{ return mMakeNonTriv; }

		private:
			int			mValue;
			int			mMakeNonTriv = 0;
		};
		static_assert(!std::is_trivially_copyable<AlNonTriv>());

		using Allocator = STLLocalAllocator<AlNonTriv, N>;
		using ArrayType = Array<AlNonTriv, Allocator>;
	#ifndef JPH_DISABLE_CUSTOM_ALLOCATOR
		static_assert(Allocator::needs_aligned_allocate);
		static_assert(AllocatorHasReallocate<Allocator>::sValue);
	#endif

		sTestArray<ArrayType, true>();
	}
}
