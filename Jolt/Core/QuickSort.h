// SPDX-FileCopyrightText: 2022 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Core/InsertionSort.h>

JPH_NAMESPACE_BEGIN

/// Implementation of the quick sort algorithm. The STL version implementation is not consistent across platforms.
template <typename Iterator, typename Compare>
inline void QuickSort(Iterator inBegin, Iterator inEnd, Compare inCompare)
{
	// If there's not enough elements we're done
	auto num_elements = inEnd - inBegin;
	if (num_elements < 2)
		return;

	// Fall back to insertion sort if there are too few elements
	if (num_elements <= 32)
	{
		InsertionSort(inBegin, inEnd, inCompare);
		return;
	}

	// Determine pivot
	Iterator pivot = inBegin + num_elements / 2;

	// Move pivot to the beginning
	Iterator store = inBegin;
	swap(*store, *pivot);

	// Partition the array
	for (Iterator it = inBegin + 1; it != inEnd; ++it)
	{
		if (inCompare(*it, *inBegin))
		{
			++store;
			swap(*store, *it);
		}
	}

	// Move pivot to the right place
	swap(*inBegin, *store);

	// Recurse
	QuickSort(inBegin, store, inCompare);
	QuickSort(store + 1, inEnd, inCompare);
}

/// Implementation of quick sort algorithm without comparator.
template <typename Iterator>
inline void QuickSort(Iterator inBegin, Iterator inEnd)
{
	less<> compare;
	QuickSort(inBegin, inEnd, compare);
}

JPH_NAMESPACE_END
