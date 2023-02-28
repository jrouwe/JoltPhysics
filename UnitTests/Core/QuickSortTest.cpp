// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"

#include <Jolt/Core/QuickSort.h>

TEST_SUITE("QuickSortTest")
{
	TEST_CASE("TestOrderedArray")
	{
		Array<int> array;
		for (int i = 0; i < 100; i++)
			array.push_back(i);

		QuickSort(array.begin(), array.end());

		for (int i = 0; i < 100; i++)
			CHECK(array[i] == i);
	}

	TEST_CASE("TestOrderedArrayComparator")
	{
		Array<int> array;
		for (int i = 0; i < 100; i++)
			array.push_back(i);

		QuickSort(array.begin(), array.end(), greater<int> {});

		for (int i = 0; i < 100; i++)
			CHECK(array[i] == 99 - i);
	}

	TEST_CASE("TestReversedArray")
	{
		Array<int> array;
		for (int i = 0; i < 100; i++)
			array.push_back(99 - i);

		QuickSort(array.begin(), array.end());

		for (int i = 0; i < 100; i++)
			CHECK(array[i] == i);
	}

	TEST_CASE("TestRandomArray")
	{
		UnitTestRandom random;

		Array<UnitTestRandom::result_type> array;
		for (int i = 0; i < 1000; i++)
		{
			UnitTestRandom::result_type value = random();

			// Insert value at beginning
			array.insert(array.begin(), value);

			// Insert value at end
			array.push_back(value);
		}

		QuickSort(array.begin(), array.end());

		for (Array<int>::size_type i = 0; i < array.size() - 2; i += 2)
		{
			// We inserted the same value twice so these elements should be the same
			CHECK(array[i] == array[i + 1]);

			// The next element should be bigger or equal
			CHECK(array[i] <= array[i + 2]);
		}
	}

	TEST_CASE("TestEmptyArray")
	{
		Array<int> array;
		QuickSort(array.begin(), array.end());
		CHECK(array.empty());
	}

	TEST_CASE("Test1ElementArray")
	{
		Array<int> array { 1 };
		QuickSort(array.begin(), array.end());
		CHECK(array[0] == 1);
	}

	TEST_CASE("Test2ElementArray")
	{
		Array<int> array { 2, 1 };
		QuickSort(array.begin(), array.end());
		CHECK(array[0] == 1);
		CHECK(array[1] == 2);
	}
}
