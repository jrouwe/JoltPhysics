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
		set.reserve(10);

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
}
