// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"

TEST_SUITE("UVec4Tests")
{
	TEST_CASE("TestUVec4Construct")
	{
		UVec4 v(1, 2, 3, 4);

		CHECK(v.GetX() == 1);
		CHECK(v.GetY() == 2);
		CHECK(v.GetZ() == 3);
		CHECK(v.GetW() == 4);

		CHECK(v[0] == 1);
		CHECK(v[1] == 2);
		CHECK(v[2] == 3);
		CHECK(v[3] == 4);

		// Test == and != operators
		CHECK(v == UVec4(1, 2, 3, 4));
		CHECK(v != UVec4(1, 2, 4, 3));
	}

	TEST_CASE("TestUVec4LoadStoreInt4")
	{
		alignas(16) uint32 i4[] = { 1, 2, 3, 4 };
		CHECK(UVec4::sLoadInt(i4) == UVec4(1, 0, 0, 0));
		CHECK(UVec4::sLoadInt4(i4) == UVec4(1, 2, 3, 4));
		CHECK(UVec4::sLoadInt4Aligned(i4) == UVec4(1, 2, 3, 4));

		uint32 i4_out1[4];
		UVec4(1, 2, 3, 4).StoreInt4(i4_out1);
		CHECK(i4_out1[0] == 1);
		CHECK(i4_out1[1] == 2);
		CHECK(i4_out1[2] == 3);
		CHECK(i4_out1[3] == 4);

		alignas(16) uint32 i4_out2[4];
		UVec4(1, 2, 3, 4).StoreInt4Aligned(i4_out2);
		CHECK(i4_out2[0] == 1);
		CHECK(i4_out2[1] == 2);
		CHECK(i4_out2[2] == 3);
		CHECK(i4_out2[3] == 4);

		uint32 si[] = { 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 4, 0 };
		CHECK(UVec4::sGatherInt4<2 * sizeof(uint32)>(si, UVec4(1, 3, 8, 9)) == UVec4(1, 2, 3, 4));
	}

	TEST_CASE("TestUVec4Zero")
	{
		UVec4 v = UVec4::sZero();

		CHECK(v.GetX() == 0);
		CHECK(v.GetY() == 0);
		CHECK(v.GetZ() == 0);
		CHECK(v.GetW() == 0);
	}

	TEST_CASE("TestUVec4Replicate")
	{
		CHECK(UVec4::sReplicate(2) == UVec4(2, 2, 2, 2));
	}

	TEST_CASE("TestUVec4MinMax")
	{
		UVec4 v1(1, 6, 3, 8);
		UVec4 v2(5, 2, 7, 4);

		CHECK(UVec4::sMin(v1, v2) == UVec4(1, 2, 3, 4));
		CHECK(UVec4::sMax(v1, v2) == UVec4(5, 6, 7, 8));
	}

	TEST_CASE("TestUVec4Comparisons")
	{
		CHECK(UVec4::sEquals(UVec4(1, 2, 3, 4), UVec4(2, 1, 3, 4)) == UVec4(0, 0, 0xffffffffU, 0xffffffffU));

		CHECK(UVec4(0x00000000U, 0x00000000U, 0x00000000U, 0x00000000U).GetTrues() == 0b0000);
		CHECK(UVec4(0xffffffffU, 0x00000000U, 0x00000000U, 0x00000000U).GetTrues() == 0b0001);
		CHECK(UVec4(0x00000000U, 0xffffffffU, 0x00000000U, 0x00000000U).GetTrues() == 0b0010);
		CHECK(UVec4(0xffffffffU, 0xffffffffU, 0x00000000U, 0x00000000U).GetTrues() == 0b0011);
		CHECK(UVec4(0x00000000U, 0x00000000U, 0xffffffffU, 0x00000000U).GetTrues() == 0b0100);
		CHECK(UVec4(0xffffffffU, 0x00000000U, 0xffffffffU, 0x00000000U).GetTrues() == 0b0101);
		CHECK(UVec4(0x00000000U, 0xffffffffU, 0xffffffffU, 0x00000000U).GetTrues() == 0b0110);
		CHECK(UVec4(0xffffffffU, 0xffffffffU, 0xffffffffU, 0x00000000U).GetTrues() == 0b0111);
		CHECK(UVec4(0x00000000U, 0x00000000U, 0x00000000U, 0xffffffffU).GetTrues() == 0b1000);
		CHECK(UVec4(0xffffffffU, 0x00000000U, 0x00000000U, 0xffffffffU).GetTrues() == 0b1001);
		CHECK(UVec4(0x00000000U, 0xffffffffU, 0x00000000U, 0xffffffffU).GetTrues() == 0b1010);
		CHECK(UVec4(0xffffffffU, 0xffffffffU, 0x00000000U, 0xffffffffU).GetTrues() == 0b1011);
		CHECK(UVec4(0x00000000U, 0x00000000U, 0xffffffffU, 0xffffffffU).GetTrues() == 0b1100);
		CHECK(UVec4(0xffffffffU, 0x00000000U, 0xffffffffU, 0xffffffffU).GetTrues() == 0b1101);
		CHECK(UVec4(0x00000000U, 0xffffffffU, 0xffffffffU, 0xffffffffU).GetTrues() == 0b1110);
		CHECK(UVec4(0xffffffffU, 0xffffffffU, 0xffffffffU, 0xffffffffU).GetTrues() == 0b1111);

		CHECK(UVec4(0x00000000U, 0x00000000U, 0x00000000U, 0x00000000U).CountTrues() == 0);
		CHECK(UVec4(0xffffffffU, 0x00000000U, 0x00000000U, 0x00000000U).CountTrues() == 1);
		CHECK(UVec4(0x00000000U, 0xffffffffU, 0x00000000U, 0x00000000U).CountTrues() == 1);
		CHECK(UVec4(0xffffffffU, 0xffffffffU, 0x00000000U, 0x00000000U).CountTrues() == 2);
		CHECK(UVec4(0x00000000U, 0x00000000U, 0xffffffffU, 0x00000000U).CountTrues() == 1);
		CHECK(UVec4(0xffffffffU, 0x00000000U, 0xffffffffU, 0x00000000U).CountTrues() == 2);
		CHECK(UVec4(0x00000000U, 0xffffffffU, 0xffffffffU, 0x00000000U).CountTrues() == 2);
		CHECK(UVec4(0xffffffffU, 0xffffffffU, 0xffffffffU, 0x00000000U).CountTrues() == 3);
		CHECK(UVec4(0x00000000U, 0x00000000U, 0x00000000U, 0xffffffffU).CountTrues() == 1);
		CHECK(UVec4(0xffffffffU, 0x00000000U, 0x00000000U, 0xffffffffU).CountTrues() == 2);
		CHECK(UVec4(0x00000000U, 0xffffffffU, 0x00000000U, 0xffffffffU).CountTrues() == 2);
		CHECK(UVec4(0xffffffffU, 0xffffffffU, 0x00000000U, 0xffffffffU).CountTrues() == 3);
		CHECK(UVec4(0x00000000U, 0x00000000U, 0xffffffffU, 0xffffffffU).CountTrues() == 2);
		CHECK(UVec4(0xffffffffU, 0x00000000U, 0xffffffffU, 0xffffffffU).CountTrues() == 3);
		CHECK(UVec4(0x00000000U, 0xffffffffU, 0xffffffffU, 0xffffffffU).CountTrues() == 3);
		CHECK(UVec4(0xffffffffU, 0xffffffffU, 0xffffffffU, 0xffffffffU).CountTrues() == 4);

		CHECK(!UVec4(0x00000000U, 0x00000000U, 0x00000000U, 0x00000000U).TestAllTrue());
		CHECK(!UVec4(0xffffffffU, 0x00000000U, 0x00000000U, 0x00000000U).TestAllTrue());
		CHECK(!UVec4(0x00000000U, 0xffffffffU, 0x00000000U, 0x00000000U).TestAllTrue());
		CHECK(!UVec4(0xffffffffU, 0xffffffffU, 0x00000000U, 0x00000000U).TestAllTrue());
		CHECK(!UVec4(0x00000000U, 0x00000000U, 0xffffffffU, 0x00000000U).TestAllTrue());
		CHECK(!UVec4(0xffffffffU, 0x00000000U, 0xffffffffU, 0x00000000U).TestAllTrue());
		CHECK(!UVec4(0x00000000U, 0xffffffffU, 0xffffffffU, 0x00000000U).TestAllTrue());
		CHECK(!UVec4(0xffffffffU, 0xffffffffU, 0xffffffffU, 0x00000000U).TestAllTrue());
		CHECK(!UVec4(0x00000000U, 0x00000000U, 0x00000000U, 0xffffffffU).TestAllTrue());
		CHECK(!UVec4(0xffffffffU, 0x00000000U, 0x00000000U, 0xffffffffU).TestAllTrue());
		CHECK(!UVec4(0x00000000U, 0xffffffffU, 0x00000000U, 0xffffffffU).TestAllTrue());
		CHECK(!UVec4(0xffffffffU, 0xffffffffU, 0x00000000U, 0xffffffffU).TestAllTrue());
		CHECK(!UVec4(0x00000000U, 0x00000000U, 0xffffffffU, 0xffffffffU).TestAllTrue());
		CHECK(!UVec4(0xffffffffU, 0x00000000U, 0xffffffffU, 0xffffffffU).TestAllTrue());
		CHECK(!UVec4(0x00000000U, 0xffffffffU, 0xffffffffU, 0xffffffffU).TestAllTrue());
		CHECK(UVec4(0xffffffffU, 0xffffffffU, 0xffffffffU, 0xffffffffU).TestAllTrue());

		CHECK(!UVec4(0x00000000U, 0x00000000U, 0x00000000U, 0x00000000U).TestAllXYZTrue());
		CHECK(!UVec4(0xffffffffU, 0x00000000U, 0x00000000U, 0x00000000U).TestAllXYZTrue());
		CHECK(!UVec4(0x00000000U, 0xffffffffU, 0x00000000U, 0x00000000U).TestAllXYZTrue());
		CHECK(!UVec4(0xffffffffU, 0xffffffffU, 0x00000000U, 0x00000000U).TestAllXYZTrue());
		CHECK(!UVec4(0x00000000U, 0x00000000U, 0xffffffffU, 0x00000000U).TestAllXYZTrue());
		CHECK(!UVec4(0xffffffffU, 0x00000000U, 0xffffffffU, 0x00000000U).TestAllXYZTrue());
		CHECK(!UVec4(0x00000000U, 0xffffffffU, 0xffffffffU, 0x00000000U).TestAllXYZTrue());
		CHECK(UVec4(0xffffffffU, 0xffffffffU, 0xffffffffU, 0x00000000U).TestAllXYZTrue());
		CHECK(!UVec4(0x00000000U, 0x00000000U, 0x00000000U, 0xffffffffU).TestAllXYZTrue());
		CHECK(!UVec4(0xffffffffU, 0x00000000U, 0x00000000U, 0xffffffffU).TestAllXYZTrue());
		CHECK(!UVec4(0x00000000U, 0xffffffffU, 0x00000000U, 0xffffffffU).TestAllXYZTrue());
		CHECK(!UVec4(0xffffffffU, 0xffffffffU, 0x00000000U, 0xffffffffU).TestAllXYZTrue());
		CHECK(!UVec4(0x00000000U, 0x00000000U, 0xffffffffU, 0xffffffffU).TestAllXYZTrue());
		CHECK(!UVec4(0xffffffffU, 0x00000000U, 0xffffffffU, 0xffffffffU).TestAllXYZTrue());
		CHECK(!UVec4(0x00000000U, 0xffffffffU, 0xffffffffU, 0xffffffffU).TestAllXYZTrue());
		CHECK(UVec4(0xffffffffU, 0xffffffffU, 0xffffffffU, 0xffffffffU).TestAllXYZTrue());

		CHECK(!UVec4(0x00000000U, 0x00000000U, 0x00000000U, 0x00000000U).TestAnyTrue());
		CHECK(UVec4(0xffffffffU, 0x00000000U, 0x00000000U, 0x00000000U).TestAnyTrue());
		CHECK(UVec4(0x00000000U, 0xffffffffU, 0x00000000U, 0x00000000U).TestAnyTrue());
		CHECK(UVec4(0xffffffffU, 0xffffffffU, 0x00000000U, 0x00000000U).TestAnyTrue());
		CHECK(UVec4(0x00000000U, 0x00000000U, 0xffffffffU, 0x00000000U).TestAnyTrue());
		CHECK(UVec4(0xffffffffU, 0x00000000U, 0xffffffffU, 0x00000000U).TestAnyTrue());
		CHECK(UVec4(0x00000000U, 0xffffffffU, 0xffffffffU, 0x00000000U).TestAnyTrue());
		CHECK(UVec4(0xffffffffU, 0xffffffffU, 0xffffffffU, 0x00000000U).TestAnyTrue());
		CHECK(UVec4(0x00000000U, 0x00000000U, 0x00000000U, 0xffffffffU).TestAnyTrue());
		CHECK(UVec4(0xffffffffU, 0x00000000U, 0x00000000U, 0xffffffffU).TestAnyTrue());
		CHECK(UVec4(0x00000000U, 0xffffffffU, 0x00000000U, 0xffffffffU).TestAnyTrue());
		CHECK(UVec4(0xffffffffU, 0xffffffffU, 0x00000000U, 0xffffffffU).TestAnyTrue());
		CHECK(UVec4(0x00000000U, 0x00000000U, 0xffffffffU, 0xffffffffU).TestAnyTrue());
		CHECK(UVec4(0xffffffffU, 0x00000000U, 0xffffffffU, 0xffffffffU).TestAnyTrue());
		CHECK(UVec4(0x00000000U, 0xffffffffU, 0xffffffffU, 0xffffffffU).TestAnyTrue());
		CHECK(UVec4(0xffffffffU, 0xffffffffU, 0xffffffffU, 0xffffffffU).TestAnyTrue());

		CHECK(!UVec4(0x00000000U, 0x00000000U, 0x00000000U, 0x00000000U).TestAnyXYZTrue());
		CHECK(UVec4(0xffffffffU, 0x00000000U, 0x00000000U, 0x00000000U).TestAnyXYZTrue());
		CHECK(UVec4(0x00000000U, 0xffffffffU, 0x00000000U, 0x00000000U).TestAnyXYZTrue());
		CHECK(UVec4(0xffffffffU, 0xffffffffU, 0x00000000U, 0x00000000U).TestAnyXYZTrue());
		CHECK(UVec4(0x00000000U, 0x00000000U, 0xffffffffU, 0x00000000U).TestAnyXYZTrue());
		CHECK(UVec4(0xffffffffU, 0x00000000U, 0xffffffffU, 0x00000000U).TestAnyXYZTrue());
		CHECK(UVec4(0x00000000U, 0xffffffffU, 0xffffffffU, 0x00000000U).TestAnyXYZTrue());
		CHECK(UVec4(0xffffffffU, 0xffffffffU, 0xffffffffU, 0x00000000U).TestAnyXYZTrue());
		CHECK(!UVec4(0x00000000U, 0x00000000U, 0x00000000U, 0xffffffffU).TestAnyXYZTrue());
		CHECK(UVec4(0xffffffffU, 0x00000000U, 0x00000000U, 0xffffffffU).TestAnyXYZTrue());
		CHECK(UVec4(0x00000000U, 0xffffffffU, 0x00000000U, 0xffffffffU).TestAnyXYZTrue());
		CHECK(UVec4(0xffffffffU, 0xffffffffU, 0x00000000U, 0xffffffffU).TestAnyXYZTrue());
		CHECK(UVec4(0x00000000U, 0x00000000U, 0xffffffffU, 0xffffffffU).TestAnyXYZTrue());
		CHECK(UVec4(0xffffffffU, 0x00000000U, 0xffffffffU, 0xffffffffU).TestAnyXYZTrue());
		CHECK(UVec4(0x00000000U, 0xffffffffU, 0xffffffffU, 0xffffffffU).TestAnyXYZTrue());
		CHECK(UVec4(0xffffffffU, 0xffffffffU, 0xffffffffU, 0xffffffffU).TestAnyXYZTrue());
	}

	TEST_CASE("TestUVec4Select")
	{
		CHECK(UVec4::sSelect(UVec4(1, 2, 3, 4), UVec4(5, 6, 7, 8), UVec4(0x80000000U, 0, 0x80000000U, 0)) == UVec4(5, 2, 7, 4));
		CHECK(UVec4::sSelect(UVec4(1, 2, 3, 4), UVec4(5, 6, 7, 8), UVec4(0, 0x80000000U, 0, 0x80000000U)) == UVec4(1, 6, 3, 8));
	}

	TEST_CASE("TestUVec4BitOps")
	{
		// Test all bit permutations
		UVec4 v1(0b0011, 0b00110, 0b001100, 0b0011000);
		UVec4 v2(0b0101, 0b01010, 0b010100, 0b0101000);

		CHECK(UVec4::sOr(v1, v2) == UVec4(0b0111, 0b01110, 0b011100, 0b0111000));
		CHECK(UVec4::sXor(v1, v2) == UVec4(0b0110, 0b01100, 0b011000, 0b0110000));
		CHECK(UVec4::sAnd(v1, v2) == UVec4(0b0001, 0b00010, 0b000100, 0b0001000));

		CHECK(UVec4::sNot(v1) == UVec4(0xfffffffcU, 0xfffffff9U, 0xfffffff3U, 0xffffffe7U));
		CHECK(UVec4::sNot(v2) == UVec4(0xfffffffaU, 0xfffffff5U, 0xffffffebU, 0xffffffd7U));

		CHECK(UVec4(0x80000000U, 0x40000000U, 0x20000000U, 0x10000000U).LogicalShiftRight<1>() == UVec4(0x40000000U, 0x20000000U, 0x10000000U, 0x08000000U));
		CHECK(UVec4(0x80000000U, 0x40000000U, 0x20000000U, 0x10000000U).ArithmeticShiftRight<1>() == UVec4(0xC0000000U, 0x20000000U, 0x10000000U, 0x08000000U));
		CHECK(UVec4(0x40000000U, 0x20000000U, 0x10000000U, 0x08000001U).LogicalShiftLeft<1>() == UVec4(0x80000000U, 0x40000000U, 0x20000000U, 0x10000002U));
	}

	TEST_CASE("TestUVec4Operators")
	{
		CHECK(UVec4(1, 2, 3, 4) + UVec4(5, 6, 7, 8) == UVec4(6, 8, 10, 12));

		CHECK(UVec4(1, 2, 3, 4) * UVec4(5, 6, 7, 8) == UVec4(1 * 5, 2 * 6, 3 * 7, 4 * 8));

		UVec4 v = UVec4(1, 2, 3, 4);
		v += UVec4(5, 6, 7, 8);
		CHECK(v == UVec4(6, 8, 10, 12));
	}

	TEST_CASE("TestUVec4Swizzle")
	{
		UVec4 v(1, 2, 3, 4);

		CHECK(v.SplatX() == UVec4::sReplicate(1));
		CHECK(v.SplatY() == UVec4::sReplicate(2));
		CHECK(v.SplatZ() == UVec4::sReplicate(3));
		CHECK(v.SplatW() == UVec4::sReplicate(4));

		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_X, SWIZZLE_X, SWIZZLE_X>() == UVec4(1, 1, 1, 1));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_X, SWIZZLE_X, SWIZZLE_Y>() == UVec4(1, 1, 1, 2));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_X, SWIZZLE_X, SWIZZLE_Z>() == UVec4(1, 1, 1, 3));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_X, SWIZZLE_X, SWIZZLE_W>() == UVec4(1, 1, 1, 4));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_X, SWIZZLE_Y, SWIZZLE_X>() == UVec4(1, 1, 2, 1));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_X, SWIZZLE_Y, SWIZZLE_Y>() == UVec4(1, 1, 2, 2));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_X, SWIZZLE_Y, SWIZZLE_Z>() == UVec4(1, 1, 2, 3));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_X, SWIZZLE_Y, SWIZZLE_W>() == UVec4(1, 1, 2, 4));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_X, SWIZZLE_Z, SWIZZLE_X>() == UVec4(1, 1, 3, 1));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_X, SWIZZLE_Z, SWIZZLE_Y>() == UVec4(1, 1, 3, 2));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_X, SWIZZLE_Z, SWIZZLE_Z>() == UVec4(1, 1, 3, 3));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_X, SWIZZLE_Z, SWIZZLE_W>() == UVec4(1, 1, 3, 4));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_X, SWIZZLE_W, SWIZZLE_X>() == UVec4(1, 1, 4, 1));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_X, SWIZZLE_W, SWIZZLE_Y>() == UVec4(1, 1, 4, 2));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_X, SWIZZLE_W, SWIZZLE_Z>() == UVec4(1, 1, 4, 3));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_X, SWIZZLE_W, SWIZZLE_W>() == UVec4(1, 1, 4, 4));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Y, SWIZZLE_X, SWIZZLE_X>() == UVec4(1, 2, 1, 1));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Y, SWIZZLE_X, SWIZZLE_Y>() == UVec4(1, 2, 1, 2));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Y, SWIZZLE_X, SWIZZLE_Z>() == UVec4(1, 2, 1, 3));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Y, SWIZZLE_X, SWIZZLE_W>() == UVec4(1, 2, 1, 4));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_X>() == UVec4(1, 2, 2, 1));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_Y>() == UVec4(1, 2, 2, 2));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_Z>() == UVec4(1, 2, 2, 3));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_W>() == UVec4(1, 2, 2, 4));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_X>() == UVec4(1, 2, 3, 1));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_Y>() == UVec4(1, 2, 3, 2));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_Z>() == UVec4(1, 2, 3, 3));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_W>() == UVec4(1, 2, 3, 4));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Y, SWIZZLE_W, SWIZZLE_X>() == UVec4(1, 2, 4, 1));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Y, SWIZZLE_W, SWIZZLE_Y>() == UVec4(1, 2, 4, 2));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Y, SWIZZLE_W, SWIZZLE_Z>() == UVec4(1, 2, 4, 3));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Y, SWIZZLE_W, SWIZZLE_W>() == UVec4(1, 2, 4, 4));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Z, SWIZZLE_X, SWIZZLE_X>() == UVec4(1, 3, 1, 1));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Z, SWIZZLE_X, SWIZZLE_Y>() == UVec4(1, 3, 1, 2));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Z, SWIZZLE_X, SWIZZLE_Z>() == UVec4(1, 3, 1, 3));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Z, SWIZZLE_X, SWIZZLE_W>() == UVec4(1, 3, 1, 4));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_X>() == UVec4(1, 3, 2, 1));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_Y>() == UVec4(1, 3, 2, 2));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_Z>() == UVec4(1, 3, 2, 3));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_W>() == UVec4(1, 3, 2, 4));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_X>() == UVec4(1, 3, 3, 1));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_Y>() == UVec4(1, 3, 3, 2));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_Z>() == UVec4(1, 3, 3, 3));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_W>() == UVec4(1, 3, 3, 4));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Z, SWIZZLE_W, SWIZZLE_X>() == UVec4(1, 3, 4, 1));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Z, SWIZZLE_W, SWIZZLE_Y>() == UVec4(1, 3, 4, 2));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Z, SWIZZLE_W, SWIZZLE_Z>() == UVec4(1, 3, 4, 3));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Z, SWIZZLE_W, SWIZZLE_W>() == UVec4(1, 3, 4, 4));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_W, SWIZZLE_X, SWIZZLE_X>() == UVec4(1, 4, 1, 1));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_W, SWIZZLE_X, SWIZZLE_Y>() == UVec4(1, 4, 1, 2));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_W, SWIZZLE_X, SWIZZLE_Z>() == UVec4(1, 4, 1, 3));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_W, SWIZZLE_X, SWIZZLE_W>() == UVec4(1, 4, 1, 4));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_W, SWIZZLE_Y, SWIZZLE_X>() == UVec4(1, 4, 2, 1));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_W, SWIZZLE_Y, SWIZZLE_Y>() == UVec4(1, 4, 2, 2));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_W, SWIZZLE_Y, SWIZZLE_Z>() == UVec4(1, 4, 2, 3));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_W, SWIZZLE_Y, SWIZZLE_W>() == UVec4(1, 4, 2, 4));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_W, SWIZZLE_Z, SWIZZLE_X>() == UVec4(1, 4, 3, 1));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_W, SWIZZLE_Z, SWIZZLE_Y>() == UVec4(1, 4, 3, 2));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_W, SWIZZLE_Z, SWIZZLE_Z>() == UVec4(1, 4, 3, 3));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_W, SWIZZLE_Z, SWIZZLE_W>() == UVec4(1, 4, 3, 4));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_W, SWIZZLE_W, SWIZZLE_X>() == UVec4(1, 4, 4, 1));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_W, SWIZZLE_W, SWIZZLE_Y>() == UVec4(1, 4, 4, 2));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_W, SWIZZLE_W, SWIZZLE_Z>() == UVec4(1, 4, 4, 3));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_W, SWIZZLE_W, SWIZZLE_W>() == UVec4(1, 4, 4, 4));

		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_X, SWIZZLE_X, SWIZZLE_X>() == UVec4(2, 1, 1, 1));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_X, SWIZZLE_X, SWIZZLE_Y>() == UVec4(2, 1, 1, 2));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_X, SWIZZLE_X, SWIZZLE_Z>() == UVec4(2, 1, 1, 3));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_X, SWIZZLE_X, SWIZZLE_W>() == UVec4(2, 1, 1, 4));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_X, SWIZZLE_Y, SWIZZLE_X>() == UVec4(2, 1, 2, 1));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_X, SWIZZLE_Y, SWIZZLE_Y>() == UVec4(2, 1, 2, 2));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_X, SWIZZLE_Y, SWIZZLE_Z>() == UVec4(2, 1, 2, 3));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_X, SWIZZLE_Y, SWIZZLE_W>() == UVec4(2, 1, 2, 4));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_X, SWIZZLE_Z, SWIZZLE_X>() == UVec4(2, 1, 3, 1));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_X, SWIZZLE_Z, SWIZZLE_Y>() == UVec4(2, 1, 3, 2));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_X, SWIZZLE_Z, SWIZZLE_Z>() == UVec4(2, 1, 3, 3));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_X, SWIZZLE_Z, SWIZZLE_W>() == UVec4(2, 1, 3, 4));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_X, SWIZZLE_W, SWIZZLE_X>() == UVec4(2, 1, 4, 1));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_X, SWIZZLE_W, SWIZZLE_Y>() == UVec4(2, 1, 4, 2));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_X, SWIZZLE_W, SWIZZLE_Z>() == UVec4(2, 1, 4, 3));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_X, SWIZZLE_W, SWIZZLE_W>() == UVec4(2, 1, 4, 4));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_X, SWIZZLE_X>() == UVec4(2, 2, 1, 1));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_X, SWIZZLE_Y>() == UVec4(2, 2, 1, 2));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_X, SWIZZLE_Z>() == UVec4(2, 2, 1, 3));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_X, SWIZZLE_W>() == UVec4(2, 2, 1, 4));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_X>() == UVec4(2, 2, 2, 1));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_Y>() == UVec4(2, 2, 2, 2));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_Z>() == UVec4(2, 2, 2, 3));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_W>() == UVec4(2, 2, 2, 4));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_X>() == UVec4(2, 2, 3, 1));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_Y>() == UVec4(2, 2, 3, 2));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_Z>() == UVec4(2, 2, 3, 3));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_W>() == UVec4(2, 2, 3, 4));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_W, SWIZZLE_X>() == UVec4(2, 2, 4, 1));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_W, SWIZZLE_Y>() == UVec4(2, 2, 4, 2));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_W, SWIZZLE_Z>() == UVec4(2, 2, 4, 3));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_W, SWIZZLE_W>() == UVec4(2, 2, 4, 4));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_X, SWIZZLE_X>() == UVec4(2, 3, 1, 1));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_X, SWIZZLE_Y>() == UVec4(2, 3, 1, 2));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_X, SWIZZLE_Z>() == UVec4(2, 3, 1, 3));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_X, SWIZZLE_W>() == UVec4(2, 3, 1, 4));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_X>() == UVec4(2, 3, 2, 1));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_Y>() == UVec4(2, 3, 2, 2));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_Z>() == UVec4(2, 3, 2, 3));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_W>() == UVec4(2, 3, 2, 4));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_X>() == UVec4(2, 3, 3, 1));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_Y>() == UVec4(2, 3, 3, 2));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_Z>() == UVec4(2, 3, 3, 3));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_W>() == UVec4(2, 3, 3, 4));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_W, SWIZZLE_X>() == UVec4(2, 3, 4, 1));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_W, SWIZZLE_Y>() == UVec4(2, 3, 4, 2));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_W, SWIZZLE_Z>() == UVec4(2, 3, 4, 3));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_W, SWIZZLE_W>() == UVec4(2, 3, 4, 4));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_W, SWIZZLE_X, SWIZZLE_X>() == UVec4(2, 4, 1, 1));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_W, SWIZZLE_X, SWIZZLE_Y>() == UVec4(2, 4, 1, 2));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_W, SWIZZLE_X, SWIZZLE_Z>() == UVec4(2, 4, 1, 3));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_W, SWIZZLE_X, SWIZZLE_W>() == UVec4(2, 4, 1, 4));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_W, SWIZZLE_Y, SWIZZLE_X>() == UVec4(2, 4, 2, 1));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_W, SWIZZLE_Y, SWIZZLE_Y>() == UVec4(2, 4, 2, 2));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_W, SWIZZLE_Y, SWIZZLE_Z>() == UVec4(2, 4, 2, 3));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_W, SWIZZLE_Y, SWIZZLE_W>() == UVec4(2, 4, 2, 4));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_W, SWIZZLE_Z, SWIZZLE_X>() == UVec4(2, 4, 3, 1));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_W, SWIZZLE_Z, SWIZZLE_Y>() == UVec4(2, 4, 3, 2));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_W, SWIZZLE_Z, SWIZZLE_Z>() == UVec4(2, 4, 3, 3));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_W, SWIZZLE_Z, SWIZZLE_W>() == UVec4(2, 4, 3, 4));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_W, SWIZZLE_W, SWIZZLE_X>() == UVec4(2, 4, 4, 1));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_W, SWIZZLE_W, SWIZZLE_Y>() == UVec4(2, 4, 4, 2));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_W, SWIZZLE_W, SWIZZLE_Z>() == UVec4(2, 4, 4, 3));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_W, SWIZZLE_W, SWIZZLE_W>() == UVec4(2, 4, 4, 4));

		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_X, SWIZZLE_X, SWIZZLE_X>() == UVec4(3, 1, 1, 1));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_X, SWIZZLE_X, SWIZZLE_Y>() == UVec4(3, 1, 1, 2));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_X, SWIZZLE_X, SWIZZLE_Z>() == UVec4(3, 1, 1, 3));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_X, SWIZZLE_X, SWIZZLE_W>() == UVec4(3, 1, 1, 4));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_X, SWIZZLE_Y, SWIZZLE_X>() == UVec4(3, 1, 2, 1));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_X, SWIZZLE_Y, SWIZZLE_Y>() == UVec4(3, 1, 2, 2));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_X, SWIZZLE_Y, SWIZZLE_Z>() == UVec4(3, 1, 2, 3));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_X, SWIZZLE_Y, SWIZZLE_W>() == UVec4(3, 1, 2, 4));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_X, SWIZZLE_Z, SWIZZLE_X>() == UVec4(3, 1, 3, 1));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_X, SWIZZLE_Z, SWIZZLE_Y>() == UVec4(3, 1, 3, 2));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_X, SWIZZLE_Z, SWIZZLE_Z>() == UVec4(3, 1, 3, 3));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_X, SWIZZLE_Z, SWIZZLE_W>() == UVec4(3, 1, 3, 4));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_X, SWIZZLE_W, SWIZZLE_X>() == UVec4(3, 1, 4, 1));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_X, SWIZZLE_W, SWIZZLE_Y>() == UVec4(3, 1, 4, 2));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_X, SWIZZLE_W, SWIZZLE_Z>() == UVec4(3, 1, 4, 3));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_X, SWIZZLE_W, SWIZZLE_W>() == UVec4(3, 1, 4, 4));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_X, SWIZZLE_X>() == UVec4(3, 2, 1, 1));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_X, SWIZZLE_Y>() == UVec4(3, 2, 1, 2));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_X, SWIZZLE_Z>() == UVec4(3, 2, 1, 3));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_X, SWIZZLE_W>() == UVec4(3, 2, 1, 4));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_X>() == UVec4(3, 2, 2, 1));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_Y>() == UVec4(3, 2, 2, 2));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_Z>() == UVec4(3, 2, 2, 3));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_W>() == UVec4(3, 2, 2, 4));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_X>() == UVec4(3, 2, 3, 1));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_Y>() == UVec4(3, 2, 3, 2));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_Z>() == UVec4(3, 2, 3, 3));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_W>() == UVec4(3, 2, 3, 4));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_W, SWIZZLE_X>() == UVec4(3, 2, 4, 1));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_W, SWIZZLE_Y>() == UVec4(3, 2, 4, 2));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_W, SWIZZLE_Z>() == UVec4(3, 2, 4, 3));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_W, SWIZZLE_W>() == UVec4(3, 2, 4, 4));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_X, SWIZZLE_X>() == UVec4(3, 3, 1, 1));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_X, SWIZZLE_Y>() == UVec4(3, 3, 1, 2));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_X, SWIZZLE_Z>() == UVec4(3, 3, 1, 3));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_X, SWIZZLE_W>() == UVec4(3, 3, 1, 4));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_X>() == UVec4(3, 3, 2, 1));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_Y>() == UVec4(3, 3, 2, 2));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_Z>() == UVec4(3, 3, 2, 3));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_W>() == UVec4(3, 3, 2, 4));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_X>() == UVec4(3, 3, 3, 1));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_Y>() == UVec4(3, 3, 3, 2));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_Z>() == UVec4(3, 3, 3, 3));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_W>() == UVec4(3, 3, 3, 4));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_W, SWIZZLE_X>() == UVec4(3, 3, 4, 1));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_W, SWIZZLE_Y>() == UVec4(3, 3, 4, 2));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_W, SWIZZLE_Z>() == UVec4(3, 3, 4, 3));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_W, SWIZZLE_W>() == UVec4(3, 3, 4, 4));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_W, SWIZZLE_X, SWIZZLE_X>() == UVec4(3, 4, 1, 1));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_W, SWIZZLE_X, SWIZZLE_Y>() == UVec4(3, 4, 1, 2));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_W, SWIZZLE_X, SWIZZLE_Z>() == UVec4(3, 4, 1, 3));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_W, SWIZZLE_X, SWIZZLE_W>() == UVec4(3, 4, 1, 4));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_W, SWIZZLE_Y, SWIZZLE_X>() == UVec4(3, 4, 2, 1));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_W, SWIZZLE_Y, SWIZZLE_Y>() == UVec4(3, 4, 2, 2));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_W, SWIZZLE_Y, SWIZZLE_Z>() == UVec4(3, 4, 2, 3));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_W, SWIZZLE_Y, SWIZZLE_W>() == UVec4(3, 4, 2, 4));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_W, SWIZZLE_Z, SWIZZLE_X>() == UVec4(3, 4, 3, 1));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_W, SWIZZLE_Z, SWIZZLE_Y>() == UVec4(3, 4, 3, 2));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_W, SWIZZLE_Z, SWIZZLE_Z>() == UVec4(3, 4, 3, 3));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_W, SWIZZLE_Z, SWIZZLE_W>() == UVec4(3, 4, 3, 4));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_W, SWIZZLE_W, SWIZZLE_X>() == UVec4(3, 4, 4, 1));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_W, SWIZZLE_W, SWIZZLE_Y>() == UVec4(3, 4, 4, 2));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_W, SWIZZLE_W, SWIZZLE_Z>() == UVec4(3, 4, 4, 3));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_W, SWIZZLE_W, SWIZZLE_W>() == UVec4(3, 4, 4, 4));

		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_X, SWIZZLE_X, SWIZZLE_X>() == UVec4(4, 1, 1, 1));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_X, SWIZZLE_X, SWIZZLE_Y>() == UVec4(4, 1, 1, 2));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_X, SWIZZLE_X, SWIZZLE_Z>() == UVec4(4, 1, 1, 3));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_X, SWIZZLE_X, SWIZZLE_W>() == UVec4(4, 1, 1, 4));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_X, SWIZZLE_Y, SWIZZLE_X>() == UVec4(4, 1, 2, 1));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_X, SWIZZLE_Y, SWIZZLE_Y>() == UVec4(4, 1, 2, 2));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_X, SWIZZLE_Y, SWIZZLE_Z>() == UVec4(4, 1, 2, 3));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_X, SWIZZLE_Y, SWIZZLE_W>() == UVec4(4, 1, 2, 4));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_X, SWIZZLE_Z, SWIZZLE_X>() == UVec4(4, 1, 3, 1));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_X, SWIZZLE_Z, SWIZZLE_Y>() == UVec4(4, 1, 3, 2));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_X, SWIZZLE_Z, SWIZZLE_Z>() == UVec4(4, 1, 3, 3));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_X, SWIZZLE_Z, SWIZZLE_W>() == UVec4(4, 1, 3, 4));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_X, SWIZZLE_W, SWIZZLE_X>() == UVec4(4, 1, 4, 1));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_X, SWIZZLE_W, SWIZZLE_Y>() == UVec4(4, 1, 4, 2));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_X, SWIZZLE_W, SWIZZLE_Z>() == UVec4(4, 1, 4, 3));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_X, SWIZZLE_W, SWIZZLE_W>() == UVec4(4, 1, 4, 4));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Y, SWIZZLE_X, SWIZZLE_X>() == UVec4(4, 2, 1, 1));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Y, SWIZZLE_X, SWIZZLE_Y>() == UVec4(4, 2, 1, 2));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Y, SWIZZLE_X, SWIZZLE_Z>() == UVec4(4, 2, 1, 3));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Y, SWIZZLE_X, SWIZZLE_W>() == UVec4(4, 2, 1, 4));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_X>() == UVec4(4, 2, 2, 1));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_Y>() == UVec4(4, 2, 2, 2));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_Z>() == UVec4(4, 2, 2, 3));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_W>() == UVec4(4, 2, 2, 4));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_X>() == UVec4(4, 2, 3, 1));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_Y>() == UVec4(4, 2, 3, 2));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_Z>() == UVec4(4, 2, 3, 3));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_W>() == UVec4(4, 2, 3, 4));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Y, SWIZZLE_W, SWIZZLE_X>() == UVec4(4, 2, 4, 1));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Y, SWIZZLE_W, SWIZZLE_Y>() == UVec4(4, 2, 4, 2));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Y, SWIZZLE_W, SWIZZLE_Z>() == UVec4(4, 2, 4, 3));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Y, SWIZZLE_W, SWIZZLE_W>() == UVec4(4, 2, 4, 4));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Z, SWIZZLE_X, SWIZZLE_X>() == UVec4(4, 3, 1, 1));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Z, SWIZZLE_X, SWIZZLE_Y>() == UVec4(4, 3, 1, 2));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Z, SWIZZLE_X, SWIZZLE_Z>() == UVec4(4, 3, 1, 3));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Z, SWIZZLE_X, SWIZZLE_W>() == UVec4(4, 3, 1, 4));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_X>() == UVec4(4, 3, 2, 1));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_Y>() == UVec4(4, 3, 2, 2));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_Z>() == UVec4(4, 3, 2, 3));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_W>() == UVec4(4, 3, 2, 4));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_X>() == UVec4(4, 3, 3, 1));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_Y>() == UVec4(4, 3, 3, 2));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_Z>() == UVec4(4, 3, 3, 3));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_W>() == UVec4(4, 3, 3, 4));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Z, SWIZZLE_W, SWIZZLE_X>() == UVec4(4, 3, 4, 1));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Z, SWIZZLE_W, SWIZZLE_Y>() == UVec4(4, 3, 4, 2));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Z, SWIZZLE_W, SWIZZLE_Z>() == UVec4(4, 3, 4, 3));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Z, SWIZZLE_W, SWIZZLE_W>() == UVec4(4, 3, 4, 4));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_W, SWIZZLE_X, SWIZZLE_X>() == UVec4(4, 4, 1, 1));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_W, SWIZZLE_X, SWIZZLE_Y>() == UVec4(4, 4, 1, 2));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_W, SWIZZLE_X, SWIZZLE_Z>() == UVec4(4, 4, 1, 3));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_W, SWIZZLE_X, SWIZZLE_W>() == UVec4(4, 4, 1, 4));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_W, SWIZZLE_Y, SWIZZLE_X>() == UVec4(4, 4, 2, 1));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_W, SWIZZLE_Y, SWIZZLE_Y>() == UVec4(4, 4, 2, 2));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_W, SWIZZLE_Y, SWIZZLE_Z>() == UVec4(4, 4, 2, 3));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_W, SWIZZLE_Y, SWIZZLE_W>() == UVec4(4, 4, 2, 4));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_W, SWIZZLE_Z, SWIZZLE_X>() == UVec4(4, 4, 3, 1));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_W, SWIZZLE_Z, SWIZZLE_Y>() == UVec4(4, 4, 3, 2));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_W, SWIZZLE_Z, SWIZZLE_Z>() == UVec4(4, 4, 3, 3));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_W, SWIZZLE_Z, SWIZZLE_W>() == UVec4(4, 4, 3, 4));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_W, SWIZZLE_W, SWIZZLE_X>() == UVec4(4, 4, 4, 1));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_W, SWIZZLE_W, SWIZZLE_Y>() == UVec4(4, 4, 4, 2));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_W, SWIZZLE_W, SWIZZLE_Z>() == UVec4(4, 4, 4, 3));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_W, SWIZZLE_W, SWIZZLE_W>() == UVec4(4, 4, 4, 4));
	}

	TEST_CASE("TestUVec4Cast")
	{
		CHECK(UVec4(1, 2, 3, 4).ToFloat() == Vec4(1, 2, 3, 4));
		CHECK(UVec4(0x3f800000U, 0x40000000U, 0x40400000U, 0x40800000U).ReinterpretAsFloat() == Vec4(1, 2, 3, 4));
	}

	TEST_CASE("TestUVec4ExtractUInt16")
	{
		uint16 ints[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
		UVec4 vector = UVec4::sLoadInt4((const uint32 *)ints);

		CHECK(vector.Expand4Uint16Lo() == UVec4(1, 2, 3, 4));
		CHECK(vector.Expand4Uint16Hi() == UVec4(5, 6, 7, 8));
	}

	TEST_CASE("TestUVec4ExtractBytes")
	{
		uint8 bytes[] = { 0x11, 0x12, 0x13, 0x14, 0x21, 0x22, 0x23, 0x24, 0x31, 0x32, 0x33, 0x34, 0x41, 0x42, 0x43, 0x44 };
		UVec4 vector = UVec4::sLoadInt4((const uint32 *)bytes);

		CHECK(vector.Expand4Byte0()  == UVec4(0x11, 0x12, 0x13, 0x14));
		CHECK(vector.Expand4Byte4()  == UVec4(0x21, 0x22, 0x23, 0x24));
		CHECK(vector.Expand4Byte8()  == UVec4(0x31, 0x32, 0x33, 0x34));
		CHECK(vector.Expand4Byte12() == UVec4(0x41, 0x42, 0x43, 0x44));
	}

	TEST_CASE("TestUVec4ShiftComponents")
	{
		UVec4 v(1, 2, 3, 4);

		CHECK(v.ShiftComponents4Minus(4) == UVec4(1, 2, 3, 4));
		CHECK(v.ShiftComponents4Minus(3) == UVec4(2, 3, 4, 0));
		CHECK(v.ShiftComponents4Minus(2) == UVec4(3, 4, 0, 0));
		CHECK(v.ShiftComponents4Minus(1) == UVec4(4, 0, 0, 0));
		CHECK(v.ShiftComponents4Minus(0) == UVec4(0, 0, 0, 0));
	}

	TEST_CASE("TestUVec4Sort4True")
	{
		CHECK(UVec4::sSort4True(UVec4(0x00000000U, 0x00000000U, 0x00000000U, 0x00000000U), UVec4(1, 2, 3, 4)) == UVec4(4, 4, 4, 4));
		CHECK(UVec4::sSort4True(UVec4(0xffffffffU, 0x00000000U, 0x00000000U, 0x00000000U), UVec4(1, 2, 3, 4)) == UVec4(1, 4, 4, 4));
		CHECK(UVec4::sSort4True(UVec4(0x00000000U, 0xffffffffU, 0x00000000U, 0x00000000U), UVec4(1, 2, 3, 4)) == UVec4(2, 4, 4, 4));
		CHECK(UVec4::sSort4True(UVec4(0xffffffffU, 0xffffffffU, 0x00000000U, 0x00000000U), UVec4(1, 2, 3, 4)) == UVec4(1, 2, 4, 4));
		CHECK(UVec4::sSort4True(UVec4(0x00000000U, 0x00000000U, 0xffffffffU, 0x00000000U), UVec4(1, 2, 3, 4)) == UVec4(3, 4, 4, 4));
		CHECK(UVec4::sSort4True(UVec4(0xffffffffU, 0x00000000U, 0xffffffffU, 0x00000000U), UVec4(1, 2, 3, 4)) == UVec4(1, 3, 4, 4));
		CHECK(UVec4::sSort4True(UVec4(0x00000000U, 0xffffffffU, 0xffffffffU, 0x00000000U), UVec4(1, 2, 3, 4)) == UVec4(2, 3, 4, 4));
		CHECK(UVec4::sSort4True(UVec4(0xffffffffU, 0xffffffffU, 0xffffffffU, 0x00000000U), UVec4(1, 2, 3, 4)) == UVec4(1, 2, 3, 4));
		CHECK(UVec4::sSort4True(UVec4(0x00000000U, 0x00000000U, 0x00000000U, 0xffffffffU), UVec4(1, 2, 3, 4)) == UVec4(4, 4, 4, 4));
		CHECK(UVec4::sSort4True(UVec4(0xffffffffU, 0x00000000U, 0x00000000U, 0xffffffffU), UVec4(1, 2, 3, 4)) == UVec4(1, 4, 4, 4));
		CHECK(UVec4::sSort4True(UVec4(0x00000000U, 0xffffffffU, 0x00000000U, 0xffffffffU), UVec4(1, 2, 3, 4)) == UVec4(2, 4, 4, 4));
		CHECK(UVec4::sSort4True(UVec4(0xffffffffU, 0xffffffffU, 0x00000000U, 0xffffffffU), UVec4(1, 2, 3, 4)) == UVec4(1, 2, 4, 4));
		CHECK(UVec4::sSort4True(UVec4(0x00000000U, 0x00000000U, 0xffffffffU, 0xffffffffU), UVec4(1, 2, 3, 4)) == UVec4(3, 4, 4, 4));
		CHECK(UVec4::sSort4True(UVec4(0xffffffffU, 0x00000000U, 0xffffffffU, 0xffffffffU), UVec4(1, 2, 3, 4)) == UVec4(1, 3, 4, 4));
		CHECK(UVec4::sSort4True(UVec4(0x00000000U, 0xffffffffU, 0xffffffffU, 0xffffffffU), UVec4(1, 2, 3, 4)) == UVec4(2, 3, 4, 4));
		CHECK(UVec4::sSort4True(UVec4(0xffffffffU, 0xffffffffU, 0xffffffffU, 0xffffffffU), UVec4(1, 2, 3, 4)) == UVec4(1, 2, 3, 4));
	}
}
