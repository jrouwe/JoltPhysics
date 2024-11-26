// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"

#include <Jolt/Core/UnorderedSet.h>

TEST_SUITE("UnorderedSetTest")
{
	TEST_CASE("TestUnorderedSet")
	{
		UnorderedSet<int> set;
		CHECK(set.bucket_count() == 0);
		set.reserve(10);
		CHECK(set.bucket_count() == 16);

		// Check system limits
		CHECK(set.max_bucket_count() == 0x80000000);
		CHECK(set.max_size() == uint64(0x80000000) * 7 / 8);

		// Insert some entries
		CHECK(*set.insert(1).first == 1);
		CHECK(set.insert(3).second);
		CHECK(!set.insert(3).second);
		CHECK(set.size() == 2);
		CHECK(*set.find(1) == 1);
		CHECK(*set.find(3) == 3);
		CHECK(set.find(5) == set.cend());

		// Validate all elements are visited by a visitor
		int count = 0;
		bool visited[10] = { false };
		for (UnorderedSet<int>::const_iterator i = set.begin(); i != set.end(); ++i)
		{
			visited[*i] = true;
			++count;
		}
		CHECK(count == 2);
		CHECK(visited[1]);
		CHECK(visited[3]);
		for (UnorderedSet<int>::iterator i = set.begin(); i != set.end(); ++i)
		{
			visited[*i] = false;
			--count;
		}
		CHECK(count == 0);
		CHECK(!visited[1]);
		CHECK(!visited[3]);

		// Copy the set
		UnorderedSet<int> set2;
		set2 = set;
		CHECK(*set2.find(1) == 1);
		CHECK(*set2.find(3) == 3);
		CHECK(set2.find(5) == set2.cend());

		// Swap
		UnorderedSet<int> set3;
		set3.swap(set);
		CHECK(*set3.find(1) == 1);
		CHECK(*set3.find(3) == 3);
		CHECK(set3.find(5) == set3.end());
		CHECK(set.empty());

		// Move construct
		UnorderedSet<int> set4(std::move(set3));
		CHECK(*set4.find(1) == 1);
		CHECK(*set4.find(3) == 3);
		CHECK(set4.find(5) == set4.end());
		CHECK(set3.empty());
	}

	TEST_CASE("TestUnorderedSetGrow")
	{
		UnorderedSet<int> set;
		for (int i = 0; i < 10000; ++i)
			CHECK(set.insert(i).second);

		CHECK(set.size() == 10000);

		for (int i = 0; i < 10000; ++i)
			CHECK(*set.find(i) == i);

		CHECK(set.find(10001) == set.cend());

		for (int i = 0; i < 5000; ++i)
			CHECK(set.erase(i) == 1);

		CHECK(set.size() == 5000);

		for (int i = 0; i < 5000; ++i)
			CHECK(set.find(i) == set.end());

		for (int i = 5000; i < 10000; ++i)
			CHECK(*set.find(i) == i);

		CHECK(set.find(10001) == set.cend());

		for (int i = 0; i < 5000; ++i)
			CHECK(set.insert(i).second);

		CHECK(!set.insert(0).second);

		CHECK(set.size() == 10000);

		for (int i = 0; i < 10000; ++i)
			CHECK(*set.find(i) == i);

		CHECK(set.find(10001) == set.cend());
	}

	TEST_CASE("TestUnorderedSetHashCollision")
	{
		// A hash function that's guaranteed to collide
		class MyBadHash
		{
		public:
			size_t operator () (int inValue) const
			{
				return 0;
			}
		};

		UnorderedSet<int, MyBadHash> set;
		for (int i = 0; i < 10; ++i)
			CHECK(set.insert(i).second);

		CHECK(set.size() == 10);

		for (int i = 0; i < 10; ++i)
			CHECK(*set.find(i) == i);

		CHECK(set.find(11) == set.cend());

		for (int i = 0; i < 5; ++i)
			CHECK(set.erase(i) == 1);

		CHECK(set.size() == 5);

		for (int i = 0; i < 5; ++i)
			CHECK(set.find(i) == set.end());

		for (int i = 5; i < 10; ++i)
			CHECK(*set.find(i) == i);

		CHECK(set.find(11) == set.cend());

		for (int i = 0; i < 5; ++i)
			CHECK(set.insert(i).second);

		CHECK(!set.insert(0).second);

		CHECK(set.size() == 10);

		for (int i = 0; i < 10; ++i)
			CHECK(*set.find(i) == i);

		CHECK(set.find(11) == set.cend());
	}

	TEST_CASE("TestUnorderedSetAddRemoveCyles")
	{
		UnorderedSet<int> set;
		constexpr int cNumElements = 64 * 7 / 8; // We add the max amount of elements possible when the bucket count is 64
		set.reserve(cNumElements);
		CHECK(set.bucket_count() == 64);

		// Repeatedly add and remove elements to see if the set cleans up tombstones
		int add_counter = 0;
		int remove_counter = 0;
		for (int i = 0; i < 100; ++i)
		{
			for (int j = 0; j < cNumElements; ++j)
				CHECK(set.insert(add_counter++).second);

			for (int j = 0; j < cNumElements; ++j)
			{
				CHECK(set.erase(remove_counter) == 1);
				CHECK(set.erase(remove_counter++) == 0);
			}
		}

		// TODO: enable CHECK(set.bucket_count() == 64);
	}
}
