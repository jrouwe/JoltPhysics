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
}
