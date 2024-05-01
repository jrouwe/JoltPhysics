// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"

TEST_SUITE("ArrayTest")
{
	TEST_CASE("TestConstructLength")
	{
		Array<int> arr(55);
		CHECK(arr.size() == 55);
	}

	TEST_CASE("TestConstructValue")
	{
		Array<int> arr(5, 55);
		CHECK(arr.size() == 5);
		CHECK(arr[0] == 55);
		CHECK(arr[1] == 55);
		CHECK(arr[2] == 55);
		CHECK(arr[3] == 55);
		CHECK(arr[4] == 55);
	}

	TEST_CASE("TestConstructIterator")
	{
		int values[] = { 1, 2, 3 };

		Array<int> arr(values, values + 3);
		CHECK(arr.size() == 3);
		CHECK(arr[0] == 1);
		CHECK(arr[1] == 2);
		CHECK(arr[2] == 3);
	}

	TEST_CASE("TestConstructInitializerList")
	{
		Array<int> arr({ 1, 2, 3 });
		CHECK(arr.size() == 3);
		CHECK(arr[0] == 1);
		CHECK(arr[1] == 2);
		CHECK(arr[2] == 3);
	}

	TEST_CASE("TestConstructFromArray")
	{
		Array<int> arr = { 1, 2, 3 };
		Array<int> arr2(arr);
		CHECK(arr2.size() == 3);
		CHECK(arr2[0] == 1);
		CHECK(arr2[1] == 2);
		CHECK(arr2[2] == 3);
	}

	TEST_CASE("TestMoveFromArray")
	{
		Array<int> arr = { 1, 2, 3 };
		Array<int> arr2(std::move(arr));
		CHECK(arr2.size() == 3);
		CHECK(arr2[0] == 1);
		CHECK(arr2[1] == 2);
		CHECK(arr2[2] == 3);
		CHECK(arr.size() == 0);
		CHECK(arr.capacity() == 0);
	}

	TEST_CASE("TestClear")
	{
		Array<int> arr({ 1, 2, 3 });
		CHECK(arr.size() == 3);
		arr.clear();
		CHECK(arr.size() == 0);
	}

	TEST_CASE("TestPushBack")
	{
		Array<int> arr;
		CHECK(arr.size() == 0);
		CHECK(arr.capacity() == 0);

		arr.push_back(1);
		CHECK(arr.size() == 1);
		CHECK(arr[0] == 1);

		arr.push_back(2);
		CHECK(arr.size() == 2);
		CHECK(arr[0] == 1);
		CHECK(arr[1] == 2);

		arr.pop_back();
		CHECK(arr.size() == 1);

		arr.pop_back();
		CHECK(arr.size() == 0);
		CHECK(arr.empty());
	}

	TEST_CASE("TestPushBackMove")
	{
		Array<Array<int>> arr;
		Array<int> arr2 = { 1, 2, 3 };
		arr.push_back(std::move(arr2));
		CHECK(arr2.size() == 0);
		CHECK(arr[0] == Array<int>({ 1, 2, 3 }));
	}

	TEST_CASE("TestEmplaceBack")
	{
		struct Test
		{
			Test(int inA, int inB) : mA(inA), mB(inB) { }

			int mA;
			int mB;
		};

		Array<Test> arr;
		arr.emplace_back(1, 2);
		CHECK(arr.size() == 1);
		CHECK(arr[0].mA == 1);
		CHECK(arr[0].mB == 2);
	}

	TEST_CASE("TestReserve")
	{
		Array<int> arr;
		CHECK(arr.capacity() == 0);

		arr.reserve(123);
		CHECK(arr.size() == 0);
		CHECK(arr.capacity() == 123);

		arr.reserve(456);
		CHECK(arr.size() == 0);
		CHECK(arr.capacity() == 456);
	}

	TEST_CASE("TestResize")
	{
		Array<int> arr;
		CHECK(arr.capacity() == 0);

		arr.resize(123);
		CHECK(arr.size() == 123);
		CHECK(arr.capacity() == 123);
		for (int i = 0; i < 123; ++i)
			arr[i] = i;

		arr.resize(456);
		CHECK(arr.size() == 456);
		CHECK(arr.capacity() == 456);
		for (int i = 0; i < 123; ++i)
			CHECK(arr[i] == i);

		arr.resize(10);
		CHECK(arr.size() == 10);
		CHECK(arr.capacity() >= 10);
	}

	TEST_CASE("TestShrinkToFit")
	{
		Array<int> arr;
		for (int i = 0; i < 5; ++i)
			arr.push_back(i);
		CHECK(arr.capacity() > 5);
		CHECK(arr.size() == 5);

		arr.shrink_to_fit();
		CHECK(arr.capacity() == 5);
		CHECK(arr.size() == 5);
		for (int i = 0; i < 5; ++i)
			CHECK(arr[i] == i);
	}

	TEST_CASE("TestAssignIterator")
	{
		int values[] = { 1, 2, 3 };

		Array<int> arr({ 4, 5, 6 });
		arr.assign(values, values + 3);
		CHECK(arr.size() == 3);
		CHECK(arr[0] == 1);
		CHECK(arr[1] == 2);
		CHECK(arr[2] == 3);
	}

	TEST_CASE("TestAssignInitializerList")
	{
		Array<int> arr({ 4, 5, 6 });
		arr.assign({ 1, 2, 3 });
		CHECK(arr.size() == 3);
		CHECK(arr[0] == 1);
		CHECK(arr[1] == 2);
		CHECK(arr[2] == 3);
	}

	TEST_CASE("TestSwap")
	{
		Array<int> arr({ 1, 2, 3 });
		Array<int> arr2({ 4, 5, 6 });
		arr.swap(arr2);
		CHECK(arr == Array<int>({ 4, 5, 6 }));
		CHECK(arr2 == Array<int>({ 1, 2, 3 }));
	}

	TEST_CASE("TestInsertBegin")
	{
		Array<int> arr = { 1, 2, 3 };
		arr.insert(arr.begin(), 4);
		CHECK(arr == Array<int>({ 4, 1, 2, 3 }));
	}

	TEST_CASE("TestInsertMid")
	{
		Array<int> arr = { 1, 2, 3 };
		arr.insert(arr.begin() + 1, 4);
		CHECK(arr == Array<int>({ 1, 4, 2, 3 }));
	}

	TEST_CASE("TestInsertEnd")
	{
		Array<int> arr = { 1, 2, 3 };
		arr.insert(arr.begin() + 3, 4);
		CHECK(arr == Array<int>({ 1, 2, 3, 4 }));
	}

	TEST_CASE("TestInsertMultipleBegin")
	{
		Array<int> values_to_insert = { 4, 5, 6, 7 };
		Array<int> arr = { 1, 2, 3 };
		arr.insert(arr.begin(), values_to_insert.begin(), values_to_insert.end());
		CHECK(arr == Array<int>({ 4, 5, 6, 7, 1, 2, 3 }));
	}

	TEST_CASE("TestInsertMultipleMid")
	{
		Array<int> values_to_insert = { 4, 5, 6, 7 };
		Array<int> arr = { 1, 2, 3 };
		arr.insert(arr.begin() + 1, values_to_insert.begin(), values_to_insert.end());
		CHECK(arr == Array<int>({ 1, 4, 5, 6, 7, 2, 3 }));
	}

	TEST_CASE("TestInsertMultipleEnd")
	{
		Array<int> values_to_insert = { 4, 5, 6, 7 };
		Array<int> arr = { 1, 2, 3 };
		arr.insert(arr.begin() + 3, values_to_insert.begin(), values_to_insert.end());
		CHECK(arr == Array<int>({ 1, 2, 3, 4, 5, 6, 7 }));
	}

	TEST_CASE("TestFrontBack")
	{
		Array<int> arr({ 1, 2, 3 });
		CHECK(arr.front() == 1);
		CHECK(arr.back() == 3);
	}

	TEST_CASE("TestAssign")
	{
		Array<int> arr({ 1, 2, 3 });
		Array<int> arr2({ 4, 5, 6 });
		arr = arr2;
		CHECK(arr == Array<int>({ 4, 5, 6 }));
		Array<int> &arr3 = arr; // Avoid compiler warning
		arr = arr3;
		CHECK(arr == Array<int>({ 4, 5, 6 }));
		arr = { 7, 8, 9 };
		CHECK(arr == Array<int>({ 7, 8, 9 }));
	}

	TEST_CASE("TestEraseBegin")
	{
		Array<int> arr({ 1, 2, 3 });
		arr.erase(arr.begin());
		CHECK(arr == Array<int>({ 2, 3 }));
	}

	TEST_CASE("TestEraseMid")
	{
		Array<int> arr({ 1, 2, 3 });
		arr.erase(arr.begin() + 1);
		CHECK(arr == Array<int>({ 1, 3 }));
	}

	TEST_CASE("TestEraseEnd")
	{
		Array<int> arr({ 1, 2, 3 });
		arr.erase(arr.begin() + 2);
		CHECK(arr == Array<int>({ 1, 2 }));
	}

	TEST_CASE("TestEraseMultipleBegin")
	{
		Array<int> arr({ 1, 2, 3, 4, 5 });
		arr.erase(arr.begin(), arr.begin() + 2);
		CHECK(arr == Array<int>({ 3, 4, 5 }));
	}

	TEST_CASE("TestEraseMultipleMid")
	{
		Array<int> arr({ 1, 2, 3, 4, 5 });
		arr.erase(arr.begin() + 2, arr.begin() + 4);
		CHECK(arr == Array<int>({ 1, 2, 5 }));
	}

	TEST_CASE("TestEraseMultipleEnd")
	{
		Array<int> arr({ 1, 2, 3, 4, 5 });
		arr.erase(arr.begin() + 3, arr.begin() + 5);
		CHECK(arr == Array<int>({ 1, 2, 3 }));
	}

	TEST_CASE("TestEquals")
	{
		Array<int> arr({ 1, 2, 3 });
		Array<int> arr2({ 4, 5, 6 });
		CHECK(arr == arr);
		CHECK(!(arr == arr2));
		CHECK(!(arr != arr));
		CHECK(arr != arr2);
	}
}
