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
		set.insert(1);
		set.insert(3);
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
	}

	TEST_CASE("TestUnorderedSetGrow")
	{
		UnorderedSet<int> set;
		for (int i = 0; i < 10000; ++i)
			set.insert(i);

		CHECK(set.size() == 10000);

		for (int i = 0; i < 10000; ++i)
			CHECK(*set.find(i) == i);

		CHECK(set.find(10001) == set.cend());

		for (int i = 0; i < 5000; ++i)
			set.erase(i);

		for (int i = 0; i < 5000; ++i)
			CHECK(set.find(i) == set.end());

		for (int i = 5000; i < 10000; ++i)
			CHECK(*set.find(i) == i);

		CHECK(set.find(10001) == set.cend());

		for (int i = 0; i < 5000; ++i)
			set.insert(i);

		for (int i = 0; i < 10000; ++i)
			CHECK(*set.find(i) == i);

		CHECK(set.find(10001) == set.cend());
	}
}
