// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"

#include <Jolt/Core/QuickSort.h>

TEST_SUITE("QuickSortTest")
{
	TEST_CASE("TestOrderedArray")
	{
		Array<int> array;
		for (int i = 0; i < 10; i++)
			array.push_back(i);

		QuickSort(array.begin(), array.end(), less<int> {});

		for (int i = 0; i < 10; i++)
			CHECK(array[i] == i);
	}

	TEST_CASE("TestReversedArray")
	{
		Array<int> array;
		for (int i = 0; i < 10; i++)
			array.push_back(9 - i);

		QuickSort(array.begin(), array.end(), less<int> {});

		for (int i = 0; i < 10; i++)
			CHECK(array[i] == i);
	}

	TEST_CASE("TestRandomArray")
	{
		UnitTestRandom random;

		Array<int> array;
		for (int i = 0; i < 1000; i++)
		{
			int value = random();

			// Insert value at beginning
			array.insert(array.begin(), value);

			// Insert value at end
			array.push_back(value);
		}

		QuickSort(array.begin(), array.end(), less<int> {});

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
		QuickSort(array.begin(), array.end(), less<int> {});
		CHECK(array.empty());
	}

	TEST_CASE("Test1ElementArray")
	{
		Array<int> array { 1 };
		QuickSort(array.begin(), array.end(), less<int> {});
		CHECK(array[0] == 1);
	}

	TEST_CASE("Test2ElementArray")
	{
		Array<int> array { 2, 1 };
		QuickSort(array.begin(), array.end(), less<int> {});
		CHECK(array[0] == 1);
		CHECK(array[1] == 2);
	}
}
