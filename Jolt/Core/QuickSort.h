// SPDX-FileCopyrightText: 2022 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Core/InsertionSort.h>

JPH_NAMESPACE_BEGIN

/// Implementation of the quick sort algorithm. The STL version implementation is not consistent across platforms.
template <typename Iterator, typename Compare>
inline void QuickSort(Iterator inBegin, Iterator inEnd, Compare inCompare)
{
	// Implementation based on https://en.wikipedia.org/wiki/Quicksort using Hoare's partition scheme

	// Loop so that we only need to do 1 recursive call instead of 2.
	for (;;)
	{
		// If there's less than 2 elements we're done
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
		Iterator pivot_iterator = inBegin + ((num_elements - 1) >> 1);
		auto pivot = *pivot_iterator;

		// Left and right iterators
		Iterator i = inBegin;
		Iterator j = inEnd;

		for (;;)
		{
			// Find the first element that is bigger than the pivot
			while (inCompare(*i, pivot))
				i++;

			// Find the last element that is smaller than the pivot
			do
				--j;
			while (inCompare(pivot, *j));

			// If the two iterators crossed, we're done
			if (i >= j)
				break;

			// Swap the elements
			swap(*i, *j);

			// Note that the first while loop in this function should
			// have been do i++ while (...) but since we cannot decrement
			// the iterator from inBegin we left that out, so we need to do
			// it here.
			++i;
		}

		// Include the middle element on the left side
		j++;

		// Check which partition is smaller
		if (j - inBegin < inEnd - j)
		{
			// Left side is smaller, recurse to left first
			QuickSort(inBegin, j, inCompare);

			// Loop again with the right side to avoid a call
			inBegin = j;
		}
		else
		{
			// Right side is smaller, recurse to right first
			QuickSort(j, inEnd, inCompare);

			// Loop again with the left side to avoid a call
			inEnd = j;
		}
	}
}

/// Implementation of quick sort algorithm without comparator.
template <typename Iterator>
inline void QuickSort(Iterator inBegin, Iterator inEnd)
{
	less<> compare;
	QuickSort(inBegin, inEnd, compare);
}

JPH_NAMESPACE_END
