// SPDX-FileCopyrightText: 2022 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

JPH_NAMESPACE_BEGIN

/// Implementation of the QuickSort algorithm. The STL version implementation is not consistent across platforms.
template <typename Iterator, typename Compare>
inline void QuickSort(Iterator inBegin, Iterator inEnd, Compare inCompare)
{
	// If there's not enough elements we're done
	if (inEnd - inBegin < 2)
		return;

	// Determine pivot
	Iterator pivot = inBegin + (inEnd - inBegin) / 2;

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

/// Implementation of QuickSort algorithm without comparator.
template <typename Iterator>
inline void QuickSort(Iterator inBegin, Iterator inEnd)
{
	less<> compare;
	QuickSort(inBegin, inEnd, compare);
}

JPH_NAMESPACE_END
