// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Core/STLAllocator.h>

#ifdef JPH_USE_STD_VECTOR

JPH_SUPPRESS_WARNINGS_STD_BEGIN
#include <vector>
JPH_SUPPRESS_WARNINGS_STD_END

JPH_NAMESPACE_BEGIN

template <class T, class Allocator = STLAllocator<T>> using Array = std::vector<T, Allocator>;

JPH_NAMESPACE_END

#else

JPH_NAMESPACE_BEGIN

/// Simple replacement for std::vector
///
/// Major differences:
/// - Memory is not initialized to zero (this was causing a lot of page faults)
/// - When resizing, we memcpy all elements to the new memory location instead of moving them using a move constructor (this is cheap and works for the classes in Jolt)
/// - Iterators are simple pointers (for now)
/// - Array capacity can only grow, only shrinks if shrink_to_fit is called
/// - No exception safety
/// - Not all functions have been implemented
template <class T, class Allocator = STLAllocator<T>>
class [[nodiscard]] Array : private Allocator
{
public:
	using value_type = T;
	using size_type = size_t;
	using pointer = T *;
	using const_pointer = const T *;
	using reference = T &;
	using const_reference = const T &;

	using const_iterator = const T *;
	using iterator = T *;

	/// Default constructor
						Array() = default;

	/// Constructor with allocator
	explicit			Array(const Allocator &inAllocator) :
		Allocator(inAllocator)
	{
	}

	/// Constructor with length
	explicit			Array(size_type inLength, const Allocator &inAllocator = { }) :
		Allocator(inAllocator)
	{
		resize(inLength);
	}

	/// Constructor with length and value
						Array(size_type inLength, const T &inValue, const Allocator &inAllocator = { }) :
		Allocator(inAllocator)
	{
		reserve(inLength);

		for (T *element = mElements, *element_end = mElements + inLength; element < element_end; ++element)
			::new (element) T(inValue);
		mSize = inLength;
	}

	/// Constructor from initializer list
						Array(std::initializer_list<T> inList, const Allocator &inAllocator = { }) :
		Allocator(inAllocator)
	{
		reserve(size_type(inList.size()));

		for (typename std::initializer_list<T>::iterator i = inList.begin(); i != inList.end(); ++i)
			::new (&mElements[mSize++]) T(*i);
	}

	/// Constructor from iterator
						Array(const_iterator inBegin, const_iterator inEnd, const Allocator &inAllocator = { }) :
		Allocator(inAllocator)
	{
		reserve(size_type(inEnd - inBegin));

		for (const_iterator element = inBegin; element < inEnd; ++element)
			::new (&mElements[mSize++]) T(*element);
	}

	/// Copy constructor
						Array(const Array<T, Allocator> &inRHS) :
		Allocator(inRHS.get_allocator())
	{
		reserve(inRHS.mSize);

		while (mSize < inRHS.mSize)
		{
			::new (&mElements[mSize]) T(inRHS[mSize]);
			++mSize;
		}
	}

	/// Move constructor
						Array(Array<T, Allocator> &&inRHS)
	{
		destroy();

		get_allocator() = std::move(inRHS.get_allocator());

		mSize = inRHS.mSize;
		mCapacity = inRHS.mCapacity;
		mElements = inRHS.mElements;

		inRHS.mSize = 0;
		inRHS.mCapacity = 0;
		inRHS.mElements = nullptr;

	}

	/// Destruct all elements
						~Array()
	{
		destroy();
	}

	/// Get the allocator
	Allocator &			get_allocator()
	{
		return *this;
	}

	const Allocator &	get_allocator() const
	{
		return *this;
	}

	/// Destruct all elements and set length to zero
	void				clear()
	{
		if constexpr (!is_trivially_destructible<T>())
			for (T *element = mElements, *end = element + mSize; element < end; ++element)
				element->~T();
		mSize = 0;
	}

	/// Add element to the back of the array
	void				push_back(const T &inValue)
	{
		JPH_ASSERT(&inValue < mElements || &inValue >= mElements + mSize, "Can't pass an element from the array to push_back");

		grow();

		T *element = mElements + mSize++;
		::new (element) T(inValue);
	}

	void				push_back(T &&inValue)
	{
		grow();

		T *element = mElements + mSize++;
		::new (element) T(std::move(inValue));
	}

	/// Construct element at the back of the array
	template <class... A>
	T &					emplace_back(A &&... inValue)
	{
		grow();

		T *element = mElements + mSize++;
		::new (element) T(std::forward<A>(inValue)...);
		return *element;
	}

	/// Remove element from the back of the array
	void				pop_back()
	{
		JPH_ASSERT(mSize > 0);
		mElements[--mSize].~T();
	}

	/// Returns true if there are no elements in the array
	bool				empty() const
	{
		return mSize == 0;
	}

	/// Returns amount of elements in the array
	size_type			size() const
	{
		return mSize;
	}

	/// Returns maximum amount of elements the array can hold
	size_type			capacity() const
	{
		return mCapacity;
	}

	/// Reserve array space
	void				reserve(size_type inNewSize)
	{
		if (mCapacity < inNewSize)
		{
			pointer pointer = get_allocator().allocate(inNewSize);
			if (mElements != nullptr)
			{
				memcpy(pointer, mElements, mSize * sizeof(T));
				get_allocator().deallocate(mElements, mCapacity);
			}
			mElements = pointer;
			mCapacity = inNewSize;
		}
	}

	/// Resize array to new length
	void				resize(size_type inNewSize)
	{
		if constexpr (!is_trivially_destructible<T>())
			for (T *element = mElements + inNewSize, *element_end = mElements + mSize; element < element_end; ++element)
				element->~T();

		reserve(inNewSize);

		if constexpr (!is_trivially_constructible<T>())
			for (T *element = mElements + mSize, *element_end = mElements + inNewSize; element < element_end; ++element)
				::new (element) T;
		mSize = inNewSize;
	}

	/// Resize array to new length and initialize all elements with inValue
	void				resize(size_type inNewSize, const T &inValue)
	{
		JPH_ASSERT(&inValue < mElements || &inValue >= mElements + mSize, "Can't pass an element from the array to resize");

		if constexpr (!is_trivially_destructible<T>())
			for (T *element = mElements + inNewSize, *element_end = mElements + mSize; element < element_end; ++element)
				element->~T();

		reserve(inNewSize);

		for (T *element = mElements + mSize, *element_end = mElements + inNewSize; element < element_end; ++element)
			::new (element) T(inValue);
		mSize = inNewSize;
	}

	/// Reduce the capacity of the array to match its size
	void				shrink_to_fit()
	{
		if (mCapacity > mSize)
		{
			pointer pointer = get_allocator().allocate(mSize);
			memcpy(pointer, mElements, mSize * sizeof(T));
			get_allocator().deallocate(mElements, mCapacity);
			mElements = pointer;
			mCapacity = mSize;
		}
	}

	/// Replace the contents of this array with inBegin .. inEnd
	template <class Iterator>
	void				assign(Iterator inBegin, Iterator inEnd)
	{
		clear();

		reserve(size_type(std::distance(inBegin, inEnd)));

		for (Iterator element = inBegin; element != inEnd; ++element)
			::new (&mElements[mSize++]) T(*element);
	}

	/// Replace the contents of this array with inList
	void				assign(std::initializer_list<T> inList)
	{
		clear();

		reserve(size_type(inList.size()));

		for (typename std::initializer_list<T>::iterator i = inList.begin(); i != inList.end(); ++i)
			::new (&mElements[mSize++]) T(*i);
	}

	/// Swap the contents of two arrays
	void				swap(Array<T, Allocator> &inRHS)
	{
		std::swap(get_allocator(), inRHS.get_allocator());
		std::swap(mSize, inRHS.mSize);
		std::swap(mCapacity, inRHS.mCapacity);
		std::swap(mElements, inRHS.mElements);
	}

	template <class Iterator>
	void				insert(const_iterator inPos, Iterator inBegin, Iterator inEnd)
	{
		// After grow() inPos may be invalid
		size_type first_element = inPos - mElements;

		size_type num_elements = size_type(std::distance(inBegin, inEnd));
		grow(num_elements);

		T *element_begin = mElements + first_element;
		T *element_end = element_begin + num_elements;
		memmove(element_end, element_begin, (mSize - first_element) * sizeof(T));

		for (T *element = element_begin; element < element_end; ++element, ++inBegin)
			::new (element) T(*inBegin);

		mSize += num_elements;
	}

	void				insert(const_iterator inPos, const T &inValue)
	{
		JPH_ASSERT(&inValue < mElements || &inValue >= mElements + mSize, "Can't pass an element from the array to insert");

		// After grow() inPos may be invalid
		size_type first_element = inPos - mElements;

		grow();

		T *element = mElements + first_element;
		memmove(element + 1, element, (mSize - first_element) * sizeof(T));

		::new (element) T(inValue);
		mSize++;
	}

	/// Iterators
	const_iterator		begin() const
	{
		return mElements;
	}

	const_iterator		end() const
	{
		return mElements + mSize;
	}

	const_iterator		cbegin() const
	{
		return mElements;
	}

	const_iterator		cend() const
	{
		return mElements + mSize;
	}

	iterator			begin()
	{
		return mElements;
	}

	iterator			end()
	{
		return mElements + mSize;
	}

	const T *			data() const
	{
		return mElements;
	}

	T *					data()
	{
		return mElements;
	}

	/// Access element
	T &					operator [] (size_type inIdx)
	{
		JPH_ASSERT(inIdx < mSize);
		return mElements[inIdx];
	}

	const T &			operator [] (size_type inIdx) const
	{
		JPH_ASSERT(inIdx < mSize);
		return mElements[inIdx];
	}

	/// Access element
	T &					at(size_type inIdx)
	{
		JPH_ASSERT(inIdx < mSize);
		return mElements[inIdx];
	}

	const T &			at(size_type inIdx) const
	{
		JPH_ASSERT(inIdx < mSize);
		return mElements[inIdx];
	}

	/// First element in the array
	const T &			front() const
	{
		JPH_ASSERT(mSize > 0);
		return mElements[0];
	}

	T &					front()
	{
		JPH_ASSERT(mSize > 0);
		return mElements[0];
	}

	/// Last element in the array
	const T &			back() const
	{
		JPH_ASSERT(mSize > 0);
		return mElements[mSize - 1];
	}

	T &					back()
	{
		JPH_ASSERT(mSize > 0);
		return mElements[mSize - 1];
	}

	/// Remove one element from the array
	void				erase(const_iterator inIter)
	{
		size_type p = size_type(inIter - begin());
		JPH_ASSERT(p < mSize);
		mElements[p].~T();
		if (p + 1 < mSize)
			memmove(mElements + p, mElements + p + 1, (mSize - p - 1) * sizeof(T));
		--mSize;
	}

	/// Remove multiple element from the array
	void				erase(const_iterator inBegin, const_iterator inEnd)
	{
		size_type p = size_type(inBegin - begin());
		size_type n = size_type(inEnd - inBegin);
		JPH_ASSERT(inEnd <= end());
		for (size_type i = 0; i < n; ++i)
			mElements[p + i].~T();
		if (p + n < mSize)
			memmove(mElements + p, mElements + p + n, (mSize - p - n) * sizeof(T));
		mSize -= n;
	}

	/// Assignment operator
	Array<T, Allocator> &	operator = (const Array<T, Allocator> &inRHS)
	{
		if (static_cast<const void *>(this) != static_cast<const void *>(&inRHS))
			assign(inRHS.begin(), inRHS.end());

		return *this;
	}

	/// Assignment operator
	Array<T, Allocator> &	operator = (std::initializer_list<T> inRHS)
	{
		assign(inRHS);

		return *this;
	}

	/// Comparing arrays
	bool				operator == (const Array<T, Allocator> &inRHS) const
	{
		if (mSize != inRHS.mSize)
			return false;
		for (size_type i = 0; i < mSize; ++i)
			if (!(mElements[i] == inRHS.mElements[i]))
				return false;
		return true;
	}

	bool				operator != (const Array<T, Allocator> &inRHS) const
	{
		if (mSize != inRHS.mSize)
			return true;
		for (size_type i = 0; i < mSize; ++i)
			if (mElements[i] != inRHS.mElements[i])
				return true;
		return false;
	}

private:
	/// Grow the array by at least inAmount elements
	inline void			grow(size_type inAmount = 1)
	{
		size_type min_size = mSize + inAmount;
		if (min_size > mCapacity)
		{
			size_type new_capacity = max(min_size, mCapacity * 2);
			reserve(new_capacity);
		}
	}

	/// Destroy all elements and free memory
	inline void			destroy()
	{
		if (mElements != nullptr)
		{
			clear();

			get_allocator().deallocate(mElements, mCapacity);
			mElements = nullptr;
			mCapacity = 0;
		}
	}

	size_type			mSize = 0;
	size_type			mCapacity = 0;
	T *					mElements = nullptr;
};

JPH_NAMESPACE_END

JPH_SUPPRESS_WARNING_PUSH
JPH_CLANG_SUPPRESS_WARNING("-Wc++98-compat")

namespace std
{
	/// Declare std::hash for Array
	template <class T, class Allocator>
	struct hash<JPH::Array<T, Allocator>>
	{
		size_t operator () (const JPH::Array<T, Allocator> &inRHS) const
		{
			std::size_t ret = 0;

			// Hash length first
            JPH::HashCombine(ret, inRHS.size());

			// Then hash elements
			for (const T &t : inRHS)
	            JPH::HashCombine(ret, t);

            return ret;
		}
	};
}

JPH_SUPPRESS_WARNING_POP

#endif // JPH_USE_STD_VECTOR
