// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include <Jolt/Core/StringTools.h>

TEST_SUITE("StringToolsTest")
{
	TEST_CASE("TestStringFormat")
	{
		CHECK(StringFormat("Test: %d", 1234) == "Test: 1234");
	}

	TEST_CASE("TestConvertToString")
	{
		CHECK(ConvertToString(1234) == "1234");
		CHECK(ConvertToString(-1) == "-1");
		CHECK(ConvertToString(0x7fffffffffffffffUL) == "9223372036854775807");
	}

	TEST_CASE("TestStringHash")
	{
		CHECK(HashString("This is a test") == 2733878766136413408UL);
	}

	TEST_CASE("StringReplace")
	{
		string value = "Hello this si si a test";
		StringReplace(value, "si", "is");
		CHECK(value == "Hello this is is a test");
		StringReplace(value, "is is", "is");
		CHECK(value == "Hello this is a test");
		StringReplace(value, "Hello", "Bye");
		CHECK(value == "Bye this is a test");
		StringReplace(value, "a test", "complete");
		CHECK(value == "Bye this is complete");
	}

	TEST_CASE("StringToVector")
	{
		vector<string> value;
		StringToVector("", value);
		CHECK(value.empty());

		StringToVector("a,b,c", value);
		CHECK(value[0] == "a");
		CHECK(value[1] == "b");
		CHECK(value[2] == "c");

		StringToVector("a,.b,.c,", value, ".");
		CHECK(value[0] == "a,");
		CHECK(value[1] == "b,");
		CHECK(value[2] == "c,");
	}

	TEST_CASE("VectorToString")
	{
		vector<string> input;
		string value;
		VectorToString(input, value);
		CHECK(value.empty());

		input = { "a", "b", "c" };
		VectorToString(input, value);
		CHECK(value == "a,b,c");

		VectorToString(input, value, ", ");
		CHECK(value == "a, b, c");
	}

	TEST_CASE("ToLower")
	{
		CHECK(ToLower("123 HeLlO!") == "123 hello!");
	}

	TEST_CASE("NibbleToBinary")
	{
		CHECK(strcmp(NibbleToBinary(0b0000), "0000") == 0);
		CHECK(strcmp(NibbleToBinary(0b0001), "0001") == 0);
		CHECK(strcmp(NibbleToBinary(0b0010), "0010") == 0);
		CHECK(strcmp(NibbleToBinary(0b0011), "0011") == 0);
		CHECK(strcmp(NibbleToBinary(0b0100), "0100") == 0);
		CHECK(strcmp(NibbleToBinary(0b0101), "0101") == 0);
		CHECK(strcmp(NibbleToBinary(0b0110), "0110") == 0);
		CHECK(strcmp(NibbleToBinary(0b0111), "0111") == 0);
		CHECK(strcmp(NibbleToBinary(0b1000), "1000") == 0);
		CHECK(strcmp(NibbleToBinary(0b1001), "1001") == 0);
		CHECK(strcmp(NibbleToBinary(0b1010), "1010") == 0);
		CHECK(strcmp(NibbleToBinary(0b1011), "1011") == 0);
		CHECK(strcmp(NibbleToBinary(0b1100), "1100") == 0);
		CHECK(strcmp(NibbleToBinary(0b1101), "1101") == 0);
		CHECK(strcmp(NibbleToBinary(0b1110), "1110") == 0);
		CHECK(strcmp(NibbleToBinary(0b1111), "1111") == 0);

		CHECK(strcmp(NibbleToBinary(0xfffffff0), "0000") == 0);
	}
}
