// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include <Jolt/Core/BinaryHeap.h>

TEST_SUITE("BinaryHeapTest")
{
	TEST_CASE("TestBinaryHeap")
	{
		// Add some numbers
		Array<int> array;
		array.reserve(1100);
		for (int i = 0; i < 1000; ++i)
			array.push_back(i);

		// Ensure we have some duplicates
		for (int i = 0; i < 1000; i += 10)
			array.push_back(i);

		// Shuffle the array
		UnitTestRandom random(123);
		std::shuffle(array.begin(), array.end(), random);

		// Add the numbers to the heap
		Array<int> heap;
		for (int i : array)
		{
			heap.push_back(i);
			BinaryHeapPush(heap.begin(), heap.end(), std::less<int> { });
		}

		// Check that the heap is sorted
		int last = INT_MAX;
		int seen[1000] { 0 };
		while (!heap.empty())
		{
			BinaryHeapPop(heap.begin(), heap.end(), std::less<int> { });
			int current = heap.back();
			CHECK(++seen[current] <= (current % 10 == 0? 2 : 1));
			heap.pop_back();
			CHECK(current <= last);
			last = current;
		}
	}
}
