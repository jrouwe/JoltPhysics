// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"

TEST_SUITE("ArrayTest")
{
	// A test class that is non-trivially copyable to test if the Array class correctly constructs/destructs/copies and moves elements.
	class NonTriv
	{
	public:
							NonTriv() : mValue(0)										{ ++sNumConstructors; }
		explicit			NonTriv(int inValue) : mValue(inValue)						{ ++sNumConstructors; }
							NonTriv(const NonTriv &inValue) : mValue(inValue.mValue)	{ ++sNumCopyConstructors; }
							NonTriv(NonTriv &&inValue) : mValue(inValue.mValue)			{ inValue.mValue = 0; ++sNumMoveConstructors; }
							~NonTriv()													{ ++sNumDestructors; }

		NonTriv &			operator = (const NonTriv &inRHS)							{ mValue = inRHS.mValue; return *this; }

		bool				operator == (const NonTriv &inRHS) const					{ return mValue == inRHS.mValue; }
		bool				operator != (const NonTriv &inRHS) const					{ return mValue != inRHS.mValue; }

		int					Value() const												{ return mValue; }

		static void			sReset()													{ sNumConstructors = 0; sNumCopyConstructors = 0; sNumMoveConstructors = 0; sNumDestructors = 0; }

		static inline int	sNumConstructors = 0;
		static inline int	sNumCopyConstructors = 0;
		static inline int	sNumMoveConstructors = 0;
		static inline int	sNumDestructors = 0;

		int					mValue;
	};

	TEST_CASE("TestConstructLength")
	{
		Array<int> arr(55);
		CHECK(arr.size() == 55);
	}

	TEST_CASE("TestConstructLengthNonTriv")
	{
		NonTriv::sReset();
		Array<NonTriv> arr(55);
		CHECK(arr.size() == 55);
		CHECK(NonTriv::sNumConstructors == 55);
		CHECK(NonTriv::sNumCopyConstructors == 0);
		CHECK(NonTriv::sNumMoveConstructors == 0);
		CHECK(NonTriv::sNumDestructors == 0);
	}

	TEST_CASE("TestConstructValue")
	{
		Array<int> arr(5, 55);
		CHECK(arr.size() == 5);
		for (int i = 0; i < 5; ++i)
			CHECK(arr[i] == 55);
	}

	TEST_CASE("TestConstructValueNonTriv")
	{
		NonTriv v(55);
		NonTriv::sReset();
		Array<NonTriv> arr(5, v);
		CHECK(arr.size() == 5);
		for (int i = 0; i < 5; ++i)
			CHECK(arr[i].Value() == 55);
		CHECK(NonTriv::sNumConstructors == 0);
		CHECK(NonTriv::sNumCopyConstructors == 5);
		CHECK(NonTriv::sNumMoveConstructors == 0);
		CHECK(NonTriv::sNumDestructors == 0);
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

	TEST_CASE("TestConstructIteratorNonTriv")
	{
		NonTriv values[] = { NonTriv(1), NonTriv(2), NonTriv(3) };

		NonTriv::sReset();
		Array<NonTriv> arr(values, values + 3);
		CHECK(arr.size() == 3);
		CHECK(arr[0].Value() == 1);
		CHECK(arr[1].Value() == 2);
		CHECK(arr[2].Value() == 3);
		CHECK(NonTriv::sNumConstructors == 0);
		CHECK(NonTriv::sNumCopyConstructors == 3);
		CHECK(NonTriv::sNumMoveConstructors == 0);
		CHECK(NonTriv::sNumDestructors == 0);
	}

	TEST_CASE("TestConstructInitializerList")
	{
		Array<int> arr({ 1, 2, 3 });
		CHECK(arr.size() == 3);
		CHECK(arr[0] == 1);
		CHECK(arr[1] == 2);
		CHECK(arr[2] == 3);
	}

	TEST_CASE("TestConstructInitializerListNonTriv")
	{
		NonTriv::sReset();
		Array<NonTriv> arr({ NonTriv(1), NonTriv(2), NonTriv(3) });
		CHECK(arr.size() == 3);
		CHECK(arr[0].Value() == 1);
		CHECK(arr[1].Value() == 2);
		CHECK(arr[2].Value() == 3);
		CHECK(NonTriv::sNumConstructors == 3); // For the initializer list
		CHECK(NonTriv::sNumCopyConstructors == 3); // Initializing the array
		CHECK(NonTriv::sNumMoveConstructors == 0);
		CHECK(NonTriv::sNumDestructors == 3); // For the initializer list
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

	TEST_CASE("TestConstructFromArrayNonTriv")
	{
		Array<NonTriv> arr = { NonTriv(1), NonTriv(2), NonTriv(3) };
		NonTriv::sReset();
		Array<NonTriv> arr2(arr);
		CHECK(arr2.size() == 3);
		CHECK(arr2[0].Value() == 1);
		CHECK(arr2[1].Value() == 2);
		CHECK(arr2[2].Value() == 3);
		CHECK(NonTriv::sNumConstructors == 0);
		CHECK(NonTriv::sNumCopyConstructors == 3);
		CHECK(NonTriv::sNumMoveConstructors == 0);
		CHECK(NonTriv::sNumDestructors == 0);
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

	TEST_CASE("TestMoveFromArrayNonTriv")
	{
		Array<NonTriv> arr = { NonTriv(1), NonTriv(2), NonTriv(3) };
		NonTriv::sReset();
		Array<NonTriv> arr2(std::move(arr)); // This just updates the mElements pointer so should not call any constructors/destructors etc.
		CHECK(arr2.size() == 3);
		CHECK(arr2[0].Value() == 1);
		CHECK(arr2[1].Value() == 2);
		CHECK(arr2[2].Value() == 3);
		CHECK(arr.size() == 0);
		CHECK(arr.capacity() == 0);
		CHECK(NonTriv::sNumConstructors == 0);
		CHECK(NonTriv::sNumCopyConstructors == 0);
		CHECK(NonTriv::sNumMoveConstructors == 0);
		CHECK(NonTriv::sNumDestructors == 0);
	}

	TEST_CASE("TestClear")
	{
		Array<int> arr({ 1, 2, 3 });
		CHECK(arr.size() == 3);
		arr.clear();
		CHECK(arr.size() == 0);
	}

	TEST_CASE("TestClearNonTriv")
	{
		NonTriv::sReset();
		Array<NonTriv> arr({ NonTriv(1), NonTriv(2), NonTriv(3) });
		CHECK(arr.size() == 3);
		CHECK(NonTriv::sNumConstructors == 3); // For initializer list
		CHECK(NonTriv::sNumCopyConstructors == 3); // To move into array
		CHECK(NonTriv::sNumMoveConstructors == 0);
		CHECK(NonTriv::sNumDestructors == 3); // For initializer list
		NonTriv::sReset();
		arr.clear();
		CHECK(arr.size() == 0);
		CHECK(NonTriv::sNumConstructors == 0);
		CHECK(NonTriv::sNumCopyConstructors == 0);
		CHECK(NonTriv::sNumMoveConstructors == 0);
		CHECK(NonTriv::sNumDestructors == 3);
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

	TEST_CASE("TestPushBackNonTriv")
	{
		NonTriv v1(1);
		NonTriv v2(2);

		NonTriv::sReset();
		Array<NonTriv> arr;
		CHECK(arr.size() == 0);
		CHECK(arr.capacity() == 0);
		CHECK(NonTriv::sNumConstructors == 0);
		CHECK(NonTriv::sNumCopyConstructors == 0);
		CHECK(NonTriv::sNumMoveConstructors == 0);
		CHECK(NonTriv::sNumDestructors == 0);

		NonTriv::sReset();
		arr.push_back(v1);
		CHECK(arr.size() == 1);
		CHECK(arr[0].Value() == 1);
		CHECK(NonTriv::sNumConstructors == 0);
		CHECK(NonTriv::sNumCopyConstructors == 1);
		CHECK(NonTriv::sNumMoveConstructors == 0);
		CHECK(NonTriv::sNumDestructors == 0);

		NonTriv::sReset();
		arr.push_back(v2);
		CHECK(arr.size() == 2);
		CHECK(arr[0].Value() == 1);
		CHECK(arr[1].Value() == 2);
#ifndef JPH_USE_STD_VECTOR
		CHECK(NonTriv::sNumConstructors == 0);
		CHECK(NonTriv::sNumCopyConstructors == 1);
		CHECK(NonTriv::sNumMoveConstructors == 1); // Array resizes from 1 to 2
		CHECK(NonTriv::sNumDestructors == 1); // Array resizes from 1 to 2
#endif // JPH_USE_STD_VECTOR

		NonTriv::sReset();
		arr.pop_back();
		CHECK(arr.size() == 1);
		CHECK(NonTriv::sNumConstructors == 0);
		CHECK(NonTriv::sNumCopyConstructors == 0);
		CHECK(NonTriv::sNumMoveConstructors == 0);
		CHECK(NonTriv::sNumDestructors == 1);

		NonTriv::sReset();
		arr.pop_back();
		CHECK(arr.size() == 0);
		CHECK(arr.empty());
		CHECK(NonTriv::sNumConstructors == 0);
		CHECK(NonTriv::sNumCopyConstructors == 0);
		CHECK(NonTriv::sNumMoveConstructors == 0);
		CHECK(NonTriv::sNumDestructors == 1);
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

	TEST_CASE("TestReserveNonTriv")
	{
		NonTriv::sReset();

		Array<NonTriv> arr;
		CHECK(arr.capacity() == 0);

		arr.reserve(123);
		CHECK(arr.size() == 0);
		CHECK(arr.capacity() == 123);

		arr.reserve(456);
		CHECK(arr.size() == 0);
		CHECK(arr.capacity() == 456);

		CHECK(NonTriv::sNumConstructors == 0);
		CHECK(NonTriv::sNumCopyConstructors == 0);
		CHECK(NonTriv::sNumMoveConstructors == 0);
		CHECK(NonTriv::sNumDestructors == 0);
	}

	TEST_CASE("TestResize")
	{
		Array<int> arr;
		CHECK(arr.size() == 0);
		CHECK(arr.capacity() == 0);
		CHECK(arr.data() == nullptr);

		arr.resize(0);
		CHECK(arr.size() == 0);
		CHECK(arr.capacity() == 0);
		CHECK(arr.data() == nullptr);

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

	TEST_CASE("TestResizeNonTriv")
	{
		NonTriv::sReset();
		Array<NonTriv> arr;
		CHECK(arr.size() == 0);
		CHECK(arr.capacity() == 0);
		CHECK(NonTriv::sNumConstructors == 0);
		CHECK(NonTriv::sNumCopyConstructors == 0);
		CHECK(NonTriv::sNumMoveConstructors == 0);
		CHECK(NonTriv::sNumDestructors == 0);

		NonTriv::sReset();
		arr.resize(123);
		CHECK(arr.size() == 123);
		CHECK(arr.capacity() == 123);
		CHECK(NonTriv::sNumConstructors == 123);
		CHECK(NonTriv::sNumCopyConstructors == 0);
		CHECK(NonTriv::sNumMoveConstructors == 0);
		CHECK(NonTriv::sNumDestructors == 0);
		for (int i = 0; i < 123; ++i)
			arr[i] = NonTriv(i);

		NonTriv::sReset();
		arr.resize(456);
		CHECK(arr.size() == 456);
		CHECK(arr.capacity() == 456);
		for (int i = 0; i < 123; ++i)
			CHECK(arr[i].Value() == i);
#ifndef JPH_USE_STD_VECTOR
		CHECK(NonTriv::sNumConstructors == 456 - 123);
		CHECK(NonTriv::sNumCopyConstructors == 0);
		CHECK(NonTriv::sNumMoveConstructors == 123);
		CHECK(NonTriv::sNumDestructors == 123); // Switched to a new block, all old elements are destroyed after being moved
#endif // JPH_USE_STD_VECTOR

		NonTriv::sReset();
		arr.resize(10);
		CHECK(arr.size() == 10);
		CHECK(arr.capacity() >= 10);
		CHECK(NonTriv::sNumConstructors == 0);
		CHECK(NonTriv::sNumCopyConstructors == 0);
		CHECK(NonTriv::sNumMoveConstructors == 0);
		CHECK(NonTriv::sNumDestructors == 456 - 10);
	}

	TEST_CASE("TestResizeWithValue")
	{
		Array<int> arr;
		arr.resize(10, 55);
		CHECK(arr.size() == 10);
		CHECK(arr.capacity() == 10);
		for (int i = 0; i < 10; ++i)
			CHECK(arr[i] == 55);
	}

	TEST_CASE("TestResizeWithValueNonTriv")
	{
		NonTriv v(55);
		Array<NonTriv> arr;
		NonTriv::sReset();
		arr.resize(10, v);
		CHECK(arr.size() == 10);
		CHECK(arr.capacity() == 10);
		for (int i = 0; i < 10; ++i)
			CHECK(arr[i].Value() == 55);
		CHECK(NonTriv::sNumConstructors == 0);
		CHECK(NonTriv::sNumCopyConstructors == 10);
		CHECK(NonTriv::sNumMoveConstructors == 0);
		CHECK(NonTriv::sNumDestructors == 0);
	}

#ifndef JPH_USE_STD_VECTOR // std::vector can choose to not shrink the array when calling shrink_to_fit so we can't test this
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

		arr.clear();
		CHECK(arr.capacity() == 5);
		CHECK(arr.size() == 0);

		arr.shrink_to_fit();
		CHECK(arr.capacity() == 0);
		CHECK(arr.data() == nullptr);
	}

	TEST_CASE("TestShrinkToFitNonTriv")
	{
		Array<NonTriv> arr;
		for (int i = 0; i < 5; ++i)
			arr.push_back(NonTriv(i));
		CHECK(arr.capacity() > 5);
		CHECK(arr.size() == 5);

		NonTriv::sReset();
		arr.shrink_to_fit();
		CHECK(arr.capacity() == 5);
		CHECK(arr.size() == 5);
		for (int i = 0; i < 5; ++i)
			CHECK(arr[i].Value() == i);
		CHECK(NonTriv::sNumConstructors == 0);
		CHECK(NonTriv::sNumCopyConstructors == 0);
		CHECK(NonTriv::sNumMoveConstructors == 5);
		CHECK(NonTriv::sNumDestructors == 5); // Switched to a new block, all old elements are destroyed after being moved
	}
#endif // JPH_USE_STD_VECTOR

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

	TEST_CASE("TestAssignMove")
	{
		Array<int> arr({ 1, 2, 3 });
		Array<int> arr2({ 4, 5, 6 });
		arr = std::move(arr2);
		CHECK(arr == Array<int>({ 4, 5, 6 }));
		CHECK(arr2.empty());
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
