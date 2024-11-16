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

		// Insert some entries
		map.insert({ 1, 2 });
		map.insert({ 3, 4 });
		CHECK(map.size() == 2);
		CHECK(map.find(1)->second == 2);
		CHECK(map.find(3)->second == 4);
		CHECK(map.find(5) == map.cend());

		// Use operator []
		map[5] = 6;
		CHECK(map.size() == 3);
		CHECK(map.find(5)->second == 6);
		map[5] = 7;
		CHECK(map.size() == 3);
		CHECK(map.find(5)->second == 7);

		// Validate all elements are visited by a visitor
		int count = 0;
		bool visited[10] = { false };
		for (HashMap<int, int>::const_iterator i = map.begin(); i != map.end(); ++i)
		{
			visited[i->first] = true;
			++count;
		}
		CHECK(count == 3);
		CHECK(visited[1]);
		CHECK(visited[3]);
		CHECK(visited[5]);
		for (HashMap<int, int>::iterator i = map.begin(); i != map.end(); ++i)
		{
			visited[i->first] = false;
			--count;
		}
		CHECK(count == 0);
		CHECK(!visited[1]);
		CHECK(!visited[3]);
		CHECK(!visited[5]);

		// Copy the map
		HashMap<int, int> map2;
		map2 = map;
		CHECK(map2.find(1)->second == 2);
		CHECK(map2.find(3)->second == 4);
		CHECK(map2.find(5)->second == 7);
		CHECK(map2.find(7) == map2.cend());

		// Try emplace
		map.try_emplace(7, 8);
		CHECK(map.size() == 4);
		CHECK(map.find(7)->second == 8);
	}

	TEST_CASE("TestHashSet")
	{
		HashSet<int> set;
		set.reserve(10);
		CHECK(set.max_size() == 16);

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
		for (HashSet<int>::const_iterator i = set.begin(); i != set.end(); ++i)
		{
			visited[*i] = true;
			++count;
		}
		CHECK(count == 2);
		CHECK(visited[1]);
		CHECK(visited[3]);
		for (HashSet<int>::iterator i = set.begin(); i != set.end(); ++i)
		{
			visited[*i] = false;
			--count;
		}
		CHECK(count == 0);
		CHECK(!visited[1]);
		CHECK(!visited[3]);

		// Copy the set
		HashSet<int> set2;
		set2 = set;
		CHECK(*set2.find(1) == 1);
		CHECK(*set2.find(3) == 3);
		CHECK(set2.find(5) == set2.cend());
	}
}
