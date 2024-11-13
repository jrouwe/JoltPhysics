// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"

#include <Jolt/Core/HashTable.h>

TEST_SUITE("HashTableTest")
{
	TEST_CASE("TestHashMap")
	{
		HashMap<int, int> map;
		map.reserve(10);
		CHECK(map.max_size() == 16);

		map.insert({ 1, 2 });
		map.insert({ 3, 4 });
		CHECK(map.size() == 2);
		CHECK(map.find(1)->second == 2);
		CHECK(map.find(3)->second == 4);
		CHECK(map.find(5) == map.cend());

		map[5] = 6;
		CHECK(map.size() == 3);
		CHECK(map.find(5)->second == 6);
		map[5] = 7;
		CHECK(map.size() == 3);
		CHECK(map.find(5)->second == 7);

		int count = 0;
		for (HashMap<int, int>::const_iterator i = map.begin(); i != map.end(); ++i)
			++count;
		CHECK(count == 3);
		for (HashMap<int, int>::iterator i = map.begin(); i != map.end(); ++i)
			--count;
		CHECK(count == 0);
	}

	TEST_CASE("TestHashSet")
	{
		HashSet<int> set;
		set.reserve(10);
		CHECK(set.max_size() == 16);

		set.insert(1);
		set.insert(3);
		CHECK(set.size() == 2);
		CHECK(*set.find(1) == 1);
		CHECK(*set.find(3) == 3);
		CHECK(set.find(5) == set.cend());

		int count = 0;
		for (HashSet<int>::const_iterator i = set.begin(); i != set.end(); ++i)
			++count;
		CHECK(count == 2);
		for (HashSet<int>::iterator i = set.begin(); i != set.end(); ++i)
			--count;
		CHECK(count == 0);
	}
}
