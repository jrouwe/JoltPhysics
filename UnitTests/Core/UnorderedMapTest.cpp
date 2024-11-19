// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"

#include <Jolt/Core/UnorderedMap.h>

TEST_SUITE("UnorderedMapTest")
{
	TEST_CASE("TestUnorderedMap")
	{
		UnorderedMap<int, int> map;
		map.reserve(10);

		// Insert some entries
		CHECK(map.insert({ 1, 2 }).first->first == 1);
		CHECK(map.insert({ 3, 4 }).second);
		CHECK(!map.insert({ 3, 5 }).second);
		CHECK(map.size() == 2);
		CHECK(map.find(1)->second == 2);
		CHECK(map.find(3)->second == 4);
		CHECK(map.find(5) == map.end());

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
		for (UnorderedMap<int, int>::const_iterator i = map.begin(); i != map.end(); ++i)
		{
			visited[i->first] = true;
			++count;
		}
		CHECK(count == 3);
		CHECK(visited[1]);
		CHECK(visited[3]);
		CHECK(visited[5]);
		for (UnorderedMap<int, int>::iterator i = map.begin(); i != map.end(); ++i)
		{
			visited[i->first] = false;
			--count;
		}
		CHECK(count == 0);
		CHECK(!visited[1]);
		CHECK(!visited[3]);
		CHECK(!visited[5]);

		// Copy the map
		UnorderedMap<int, int> map2;
		map2 = map;
		CHECK(map2.find(1)->second == 2);
		CHECK(map2.find(3)->second == 4);
		CHECK(map2.find(5)->second == 7);
		CHECK(map2.find(7) == map2.end());

		// Try emplace
		map.try_emplace(7, 8);
		CHECK(map.size() == 4);
		CHECK(map.find(7)->second == 8);

		// Swap
		UnorderedMap<int, int> map3;
		map3.swap(map);
		CHECK(map3.find(1)->second == 2);
		CHECK(map3.find(3)->second == 4);
		CHECK(map3.find(5)->second == 7);
		CHECK(map3.find(7)->second == 8);
		CHECK(map3.find(9) == map3.end());
		CHECK(map.empty());

		// Move construct
		UnorderedMap<int, int> map4(std::move(map3));
		CHECK(map4.find(1)->second == 2);
		CHECK(map4.find(3)->second == 4);
		CHECK(map4.find(5)->second == 7);
		CHECK(map4.find(7)->second == 8);
		CHECK(map4.find(9) == map4.end());
		CHECK(map3.empty());
	}

	TEST_CASE("TestUnorderedMapGrow")
	{
		UnorderedMap<int, int> map;
		for (int i = 0; i < 10000; ++i)
			CHECK(map.try_emplace(i, ~i).second);

		CHECK(map.size() == 10000);

		for (int i = 0; i < 10000; ++i)
			CHECK(map.find(i)->second == ~i);

		CHECK(map.find(10001) == map.end());

		for (int i = 0; i < 5000; ++i)
			CHECK(map.erase(i) == 1);

		CHECK(map.size() == 5000);

		for (int i = 0; i < 5000; ++i)
			CHECK(map.find(i) == map.end());

		for (int i = 5000; i < 10000; ++i)
			CHECK(map.find(i)->second == ~i);

		CHECK(map.find(10001) == map.end());

		for (int i = 0; i < 5000; ++i)
			CHECK(map.try_emplace(i, i + 1).second);

		CHECK(!map.try_emplace(0, 0).second);

		CHECK(map.size() == 10000);

		for (int i = 0; i < 5000; ++i)
			CHECK(map.find(i)->second == i + 1);

		for (int i = 5000; i < 10000; ++i)
			CHECK(map.find(i)->second == ~i);

		CHECK(map.find(10001) == map.end());
	}
}
