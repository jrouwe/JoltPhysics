// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Core/Memory.h>

namespace JPH {

/// STL allocator that takes care that memory is aligned to N bytes
template <typename T, size_t N>
class AlignedAllocator
{
public:
	using value_type = T;

	/// Pointer to type
	using pointer = T *;
	using const_pointer = const T *;

	/// Reference to type.
	/// Can be removed in C++20.
	using reference = T &;
	using const_reference = const T &;

	using size_type = size_t;
	using difference_type = ptrdiff_t;

	/// Constructor
	inline					AlignedAllocator() = default;

	/// Constructor from other allocator
	template <typename T2>
	inline explicit			AlignedAllocator(const AlignedAllocator<T2, N> &) { }

	/// Allocate memory
	inline pointer			allocate(size_type n)
	{
		return (pointer)AlignedAlloc(n * sizeof(value_type), N);
	}

	/// Free memory
	inline void				deallocate(pointer p, size_type)
	{
		AlignedFree(p);
	}

	/// Allocators are stateless so assumed to be equal
	inline bool				operator == (const AlignedAllocator<T, N>& other) const
	{
		return true;
	}

	inline bool				operator != (const AlignedAllocator<T, N>& other) const
	{
		return false;
	}

	/// Converting to allocator for other type
	template <typename T2>
	struct rebind
	{
		using other = AlignedAllocator<T2, N>;
	};
};

} // JPH