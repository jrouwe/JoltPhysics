// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

JPH_NAMESPACE_BEGIN

/// STL allocator that forwards to our allocation functions
template <typename T>
class STLAllocator
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
	inline					STLAllocator() = default;

	/// Constructor from other allocator
	template <typename T2>
	inline explicit			STLAllocator(const STLAllocator<T2> &) { }

	/// Allocate memory
	inline pointer			allocate(size_type inN)
	{
		if constexpr (alignof(T) > (JPH_CPU_ADDRESS_BITS == 32? 8 : 16))
			return (pointer)AlignedAlloc(inN * sizeof(value_type), alignof(T));
		else
			return (pointer)Alloc(inN * sizeof(value_type));
	}

	/// Free memory
	inline void				deallocate(pointer inPointer, size_type)
	{
		if constexpr (alignof(T) > (JPH_CPU_ADDRESS_BITS == 32? 8 : 16))
			AlignedFree(inPointer);
		else
			Free(inPointer);
	}

	/// Allocators are stateless so assumed to be equal
	inline bool				operator == (const STLAllocator<T> &) const
	{
		return true;
	}

	inline bool				operator != (const STLAllocator<T> &) const
	{
		return false;
	}

	/// Converting to allocator for other type
	template <typename T2>
	struct rebind
	{
		using other = STLAllocator<T2>;
	};
};

// Declare STL containers that use our allocator
template <class T> using Array = vector<T, STLAllocator<T>>;
using String = basic_string<char, char_traits<char>, STLAllocator<char>>;
using IStringStream = basic_istringstream<char, char_traits<char>, STLAllocator<char>>;

JPH_NAMESPACE_END
