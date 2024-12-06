// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"

#include <Jolt/Math/BVec16.h>
#include <Jolt/Core/StringTools.h>

TEST_SUITE("BVec16Tests")
{
	TEST_CASE("TestBVec16Construct")
	{
		BVec16 v(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);

		CHECK(v[0] == 1);
		CHECK(v[1] == 2);
		CHECK(v[2] == 3);
		CHECK(v[3] == 4);
		CHECK(v[4] == 5);
		CHECK(v[5] == 6);
		CHECK(v[6] == 7);
		CHECK(v[7] == 8);
		CHECK(v[8] == 9);
		CHECK(v[9] == 10);
		CHECK(v[10] == 11);
		CHECK(v[11] == 12);
		CHECK(v[12] == 13);
		CHECK(v[13] == 14);
		CHECK(v[14] == 15);
		CHECK(v[15] == 16);

		// Test == and != operators
		CHECK(v == BVec16(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16));
		CHECK(v != BVec16(1, 2, 3, 4, 5, 6, 7, 8, 10, 9, 11, 12, 13, 14, 15, 16));

		// Check element modification
		CHECK(const_cast<const BVec16 &>(v)[15] == 16); // Check const operator
		v[15] = 17;
		CHECK(v[15] == 17);
		CHECK(v == BVec16(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 17));
	}

	TEST_CASE("TestBVec16LoadByte16")
	{
		uint8 u16[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
		CHECK(BVec16::sLoadByte16(u16) == BVec16(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16));
	}

	TEST_CASE("TestBVec16Zero")
	{
		BVec16 v = BVec16::sZero();

		for (int i = 0; i < 16; ++i)
			CHECK(v[i] == 0);
	}

	TEST_CASE("TestBVec16Replicate")
	{
		CHECK(BVec16::sReplicate(2) == BVec16(2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2));
	}

	TEST_CASE("TestBVec16Comparisons")
	{
		BVec16 eq = BVec16::sEquals(BVec16(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16), BVec16(6, 7, 3, 4, 5, 6, 7, 5, 9, 10, 11, 12, 13, 14, 15, 13));
		CHECK(eq.GetTrues() == 0b0111111101111100);
		CHECK(eq.TestAnyTrue());
		CHECK(!eq.TestAllTrue());
	}

	TEST_CASE("TestBVec16BitOps")
	{
		// Test all bit permutations
		BVec16 v1(0b011, 0b0110, 0b01100, 0b011000, 0b0110000, 0b01100000, 0b011, 0b0110, 0b01100, 0b011000, 0b0110000, 0b01100000, 0b011, 0b0110, 0b01100, 0b011000);
		BVec16 v2(0b101, 0b1010, 0b10100, 0b101000, 0b1010000, 0b10100000, 0b101, 0b1010, 0b10100, 0b101000, 0b1010000, 0b10100000, 0b101, 0b1010, 0b10100, 0b101000);

		CHECK(BVec16::sOr(v1, v2) == BVec16(0b111, 0b1110, 0b11100, 0b111000, 0b1110000, 0b11100000, 0b111, 0b1110, 0b11100, 0b111000, 0b1110000, 0b11100000, 0b111, 0b1110, 0b11100, 0b111000));
		CHECK(BVec16::sXor(v1, v2) == BVec16(0b110, 0b1100, 0b11000, 0b110000, 0b1100000, 0b11000000, 0b110, 0b1100, 0b11000, 0b110000, 0b1100000, 0b11000000, 0b110, 0b1100, 0b11000, 0b110000));
		CHECK(BVec16::sAnd(v1, v2) == BVec16(0b001, 0b0010, 0b00100, 0b001000, 0b0010000, 0b00100000, 0b001, 0b0010, 0b00100, 0b001000, 0b0010000, 0b00100000, 0b001, 0b0010, 0b00100, 0b001000));

		CHECK(BVec16::sNot(v1) == BVec16(0b11111100, 0b11111001, 0b11110011, 0b11100111, 0b11001111, 0b10011111, 0b11111100, 0b11111001, 0b11110011, 0b11100111, 0b11001111, 0b10011111, 0b11111100, 0b11111001, 0b11110011, 0b11100111));
		CHECK(BVec16::sNot(v2) == BVec16(0b11111010, 0b11110101, 0b11101011, 0b11010111, 0b10101111, 0b01011111, 0b11111010, 0b11110101, 0b11101011, 0b11010111, 0b10101111, 0b01011111, 0b11111010, 0b11110101, 0b11101011, 0b11010111));
	}

	TEST_CASE("TestBVec16ToString")
	{
		BVec16 v(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);

		CHECK(ConvertToString(v) == "1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16");
	}
}
