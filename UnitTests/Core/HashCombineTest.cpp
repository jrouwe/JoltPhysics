// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include <Jolt/Core/HashCombine.h>

TEST_SUITE("HashCombineTest")
{
	TEST_CASE("TestHashBytes")
	{
		CHECK(HashBytes("This is a test", 14) == 2733878766136413408UL);
	}

	TEST_CASE("TestHashString")
	{
		CHECK(HashString("This is a test") == 2733878766136413408UL);
	}

	TEST_CASE("TestHashStruct")
	{
		const char *char_test = "This is a test";
		CHECK(Hash<const char *> { } (char_test) == 2733878766136413408UL);

		std::string_view str_view_test = "This is a test";
		CHECK(Hash<std::string_view> { } (str_view_test) == 2733878766136413408UL);

		String str_test = "This is a test";
		CHECK(Hash<String> { } (str_test) == 2733878766136413408UL);
	}

	TEST_CASE("TestHashCombine")
	{
		int val1 = 0;
		uint64 val1_hash = Hash<int> { } (val1);
		int val2 = 1;
		uint64 val2_hash = Hash<int> { } (val2);

		// Check non-commutative
		uint64 seed1 = val1_hash;
		HashCombine(seed1, val2);
		uint64 seed2 = val2_hash;
		HashCombine(seed2, val1);
		CHECK(seed1 != seed2);

		// Check that adding a 0 changes the hash
		uint64 seed3 = val1_hash;
		HashCombine(seed3, val1);
		CHECK(seed3 != val1_hash);
	}
}
