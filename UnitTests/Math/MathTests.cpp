// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include <Jolt/Math/Mat44.h>

TEST_SUITE("Mat44Tests")
{
	TEST_CASE("TestCountTrailingZeros")
	{
		CHECK(CountTrailingZeros(0) == 32);
		for (int i = 0; i < 32; ++i)
			CHECK(CountTrailingZeros(1U << i) == i);
	}

	TEST_CASE("TestCountLeadingZeros")
	{
		CHECK(CountLeadingZeros(0) == 32);
		for (int i = 0; i < 32; ++i)
			CHECK(CountLeadingZeros(1U << i) == 31 - i);
	}

	TEST_CASE("TestCountBits")
	{
		CHECK(CountBits(0) == 0);
		CHECK(CountBits(0b10000000000000000000000000000000) == 1);
		CHECK(CountBits(0b00000000000000000000000000000001) == 1);
		CHECK(CountBits(0b10000000000000001000000000000000) == 2);
		CHECK(CountBits(0b00000000000000010000000000000001) == 2);
		CHECK(CountBits(0b10000000100000001000000010000000) == 4);
		CHECK(CountBits(0b00000001000000010000000100000001) == 4);
		CHECK(CountBits(0b10001000100010001000100010001000) == 8);
		CHECK(CountBits(0b00010001000100010001000100010001) == 8);
		CHECK(CountBits(0b10101010101010101010101010101010) == 16);
		CHECK(CountBits(0b01010101010101010101010101010101) == 16);
		CHECK(CountBits(0b11111111111111111111111111111111) == 32);
	}

	TEST_CASE("TestNextPowerOf2")
	{
		CHECK(GetNextPowerOf2(0) == 1);

		for (int i = 0; i < 31; ++i)
		{
			uint32 pow = uint32(1) << i;
			if (pow > 2)
				CHECK(GetNextPowerOf2(pow - 1) == pow);
			CHECK(GetNextPowerOf2(pow) == pow);
			CHECK(GetNextPowerOf2(pow + 1) == pow << 1);
		}

		CHECK(GetNextPowerOf2(0x8000000U - 1) == 0x8000000U);
		CHECK(GetNextPowerOf2(0x8000000U) == 0x8000000U);
	}
}
