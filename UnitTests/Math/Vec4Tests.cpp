// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"

TEST_SUITE("Vec4Tests")
{
	TEST_CASE("TestVec4Construct")
	{
		Vec4 v(1, 2, 3, 4);

		// Test component access
		CHECK(v.GetX() == 1);
		CHECK(v.GetY() == 2);
		CHECK(v.GetZ() == 3);
		CHECK(v.GetW() == 4);

		// Test component access by [] operators
		CHECK(v[0] == 1);
		CHECK(v[1] == 2);
		CHECK(v[2] == 3);
		CHECK(v[3] == 4);

		// Test == and != operators
		CHECK(v == Vec4(1, 2, 3, 4));
		CHECK(v != Vec4(1, 2, 4, 3));
	}

	TEST_CASE("TestVec4LoadStoreFloat4")
	{
		alignas(16) Float4 f4 = { 1, 2, 3, 4 };
		CHECK(Vec4::sLoadFloat4(&f4) == Vec4(1, 2, 3, 4));
		CHECK(Vec4::sLoadFloat4Aligned(&f4) == Vec4(1, 2, 3, 4));

		Float4 f4_out;
		Vec4(1, 2, 3, 4).StoreFloat4(&f4_out);
		CHECK(f4_out[0] == 1);
		CHECK(f4_out[1] == 2);
		CHECK(f4_out[2] == 3);
		CHECK(f4_out[3] == 4);

		float sf[] = { 0, 0,  1, 0,  0, 0,  2, 0,  0, 0,  0, 0,  0, 0,  0, 0,  3, 0, 4, 0 };
		CHECK(Vec4::sGatherFloat4<2 * sizeof(float)>(sf, UVec4(1, 3, 8, 9)) == Vec4(1, 2, 3, 4));
	}

	TEST_CASE("TestVec4ConstructVec3")
	{
		Vec3 v3(1, 2, 3);
		CHECK(Vec4(v3, 4) == Vec4(1, 2, 3, 4));
	}

	TEST_CASE("TestVec4Zero")
	{
		Vec4 v = Vec4::sZero();

		CHECK(v.GetX() == 0);
		CHECK(v.GetY() == 0);
		CHECK(v.GetZ() == 0);
		CHECK(v.GetW() == 0);
	}

	TEST_CASE("TestVec4NaN")
	{
		Vec4 v = Vec4::sNaN();

		CHECK(isnan(v.GetX()));
		CHECK(isnan(v.GetY()));
		CHECK(isnan(v.GetZ()));
		CHECK(isnan(v.GetW()));
		CHECK(v.IsNaN());

		v.SetX(0);
		CHECK(v.IsNaN());
		v.SetY(0);
		CHECK(v.IsNaN());
		v.SetZ(0);
		CHECK(v.IsNaN());
		v.SetW(0);
		CHECK(!v.IsNaN());
	}

	TEST_CASE("TestVec4Replicate")
	{
		CHECK(Vec4::sReplicate(2) == Vec4(2, 2, 2, 2));
	}

	TEST_CASE("TestVec4MinMax")
	{
		Vec4 v1(1, 6, 3, 8);
		Vec4 v2(5, 2, 7, 4);

		CHECK(Vec4::sMin(v1, v2) == Vec4(1, 2, 3, 4));
		CHECK(Vec4::sMax(v1, v2) == Vec4(5, 6, 7, 8));

		CHECK(v1.ReduceMin() == 1);
		CHECK(v1.ReduceMax() == 8);
		CHECK(v2.ReduceMin() == 2);
		CHECK(v2.ReduceMax() == 7);
	}

	TEST_CASE("TestVec4Comparisons")
	{
		CHECK(Vec4::sEquals(Vec4(1, 2, 3, 4), Vec4(2, 1, 3, 4)) == UVec4(0, 0, 0xffffffffU, 0xffffffffU));
		CHECK(Vec4::sLess(Vec4(1, 2, 3, 4), Vec4(2, 1, 3, 4)) == UVec4(0xffffffffU, 0, 0, 0));
		CHECK(Vec4::sLessOrEqual(Vec4(1, 2, 3, 4), Vec4(2, 1, 3, 4)) == UVec4(0xffffffffU, 0, 0xffffffffU, 0xffffffffU));
		CHECK(Vec4::sGreater(Vec4(1, 2, 3, 4), Vec4(2, 1, 3, 4)) == UVec4(0, 0xffffffffU, 0, 0));
		CHECK(Vec4::sGreaterOrEqual(Vec4(1, 2, 3, 4), Vec4(2, 1, 3, 4)) == UVec4(0, 0xffffffffU, 0xffffffffU, 0xffffffffU));
	}

	TEST_CASE("TestVec4FMA")
	{
		CHECK(Vec4::sFusedMultiplyAdd(Vec4(1, 2, 3, 4), Vec4(5, 6, 7, 8), Vec4(9, 10, 11, 12)) == Vec4(1 * 5 + 9, 2 * 6 + 10, 3 * 7 + 11, 4 * 8 + 12));
	}

	TEST_CASE("TestVec4Select")
	{
		CHECK(Vec4::sSelect(Vec4(1, 2, 3, 4), Vec4(5, 6, 7, 8), UVec4(0x80000000U, 0, 0x80000000U, 0)) == Vec4(5, 2, 7, 4));
		CHECK(Vec4::sSelect(Vec4(1, 2, 3, 4), Vec4(5, 6, 7, 8), UVec4(0, 0x80000000U, 0, 0x80000000U)) == Vec4(1, 6, 3, 8));
	}

	TEST_CASE("TestVec4BitOps")
	{
		// Test all bit permutations
		Vec4 v1(UVec4(0b0011, 0b00110, 0b001100, 0b0011000).ReinterpretAsFloat());
		Vec4 v2(UVec4(0b0101, 0b01010, 0b010100, 0b0101000).ReinterpretAsFloat());

		CHECK(Vec4::sOr(v1, v2) == Vec4(UVec4(0b0111, 0b01110, 0b011100, 0b0111000).ReinterpretAsFloat()));
		CHECK(Vec4::sXor(v1, v2) == Vec4(UVec4(0b0110, 0b01100, 0b011000, 0b0110000).ReinterpretAsFloat()));
		CHECK(Vec4::sAnd(v1, v2) == Vec4(UVec4(0b0001, 0b00010, 0b000100, 0b0001000).ReinterpretAsFloat()));
	}

	TEST_CASE("TestVec4Close")
	{
		CHECK(Vec4(1, 2, 3, 4).IsClose(Vec4(1.001f, 2.001f, 3.001f, 4.001f), 1.0e-4f));
		CHECK(!Vec4(1, 2, 3, 4).IsClose(Vec4(1.001f, 2.001f, 3.001f, 4.001f), 1.0e-6f));

		CHECK(Vec4(1.001f, 0, 0, 0).IsNormalized(1.0e-2f));
		CHECK(!Vec4(0, 1.001f, 0, 0).IsNormalized(1.0e-4f));
	}

	TEST_CASE("TestVec4Operators")
	{
		CHECK(-Vec4(1, 2, 3, 4) == Vec4(-1, -2, -3, -4));

		CHECK(Vec4(1, 2, 3, 4) + Vec4(5, 6, 7, 8) == Vec4(6, 8, 10, 12));
		CHECK(Vec4(1, 2, 3, 4) - Vec4(8, 7, 6, 5) == Vec4(-7, -5, -3, -1));

		CHECK(Vec4(1, 2, 3, 4) * Vec4(5, 6, 7, 8) == Vec4(5, 12, 21, 32));
		CHECK(Vec4(1, 2, 3, 4) * 2 == Vec4(2, 4, 6, 8));
		CHECK(4 * Vec4(1, 2, 3, 4) == Vec4(4, 8, 12, 16));

		CHECK(Vec4(1, 2, 3, 4) / 2 == Vec4(0.5f, 1.0f, 1.5f, 2.0f));
		CHECK(Vec4(1, 2, 3, 4) / Vec4(2, 8, 24, 64) == Vec4(0.5f, 0.25f, 0.125f, 0.0625f));

		Vec4 v = Vec4(1, 2, 3, 4);
		v *= Vec4(5, 6, 7, 8);
		CHECK(v == Vec4(5, 12, 21, 32));
		v *= 2;
		CHECK(v == Vec4(10, 24, 42, 64));
		v /= 2;
		CHECK(v == Vec4(5, 12, 21, 32));
		v += Vec4(1, 2, 3, 4);
		CHECK(v == Vec4(6, 14, 24, 36));

		CHECK(Vec4(2, 4, 8, 16).Reciprocal() == Vec4(0.5f, 0.25f, 0.125f, 0.0625f));
	}

	TEST_CASE("TestVec4Swizzle")
	{
		Vec4 v(1, 2, 3, 4);

		CHECK(v.SplatX() == Vec4::sReplicate(1));
		CHECK(v.SplatY() == Vec4::sReplicate(2));
		CHECK(v.SplatZ() == Vec4::sReplicate(3));
		CHECK(v.SplatW() == Vec4::sReplicate(4));

		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_X, SWIZZLE_X, SWIZZLE_X>() == Vec4(1, 1, 1, 1));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_X, SWIZZLE_X, SWIZZLE_Y>() == Vec4(1, 1, 1, 2));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_X, SWIZZLE_X, SWIZZLE_Z>() == Vec4(1, 1, 1, 3));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_X, SWIZZLE_X, SWIZZLE_W>() == Vec4(1, 1, 1, 4));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_X, SWIZZLE_Y, SWIZZLE_X>() == Vec4(1, 1, 2, 1));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_X, SWIZZLE_Y, SWIZZLE_Y>() == Vec4(1, 1, 2, 2));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_X, SWIZZLE_Y, SWIZZLE_Z>() == Vec4(1, 1, 2, 3));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_X, SWIZZLE_Y, SWIZZLE_W>() == Vec4(1, 1, 2, 4));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_X, SWIZZLE_Z, SWIZZLE_X>() == Vec4(1, 1, 3, 1));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_X, SWIZZLE_Z, SWIZZLE_Y>() == Vec4(1, 1, 3, 2));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_X, SWIZZLE_Z, SWIZZLE_Z>() == Vec4(1, 1, 3, 3));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_X, SWIZZLE_Z, SWIZZLE_W>() == Vec4(1, 1, 3, 4));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_X, SWIZZLE_W, SWIZZLE_X>() == Vec4(1, 1, 4, 1));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_X, SWIZZLE_W, SWIZZLE_Y>() == Vec4(1, 1, 4, 2));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_X, SWIZZLE_W, SWIZZLE_Z>() == Vec4(1, 1, 4, 3));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_X, SWIZZLE_W, SWIZZLE_W>() == Vec4(1, 1, 4, 4));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Y, SWIZZLE_X, SWIZZLE_X>() == Vec4(1, 2, 1, 1));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Y, SWIZZLE_X, SWIZZLE_Y>() == Vec4(1, 2, 1, 2));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Y, SWIZZLE_X, SWIZZLE_Z>() == Vec4(1, 2, 1, 3));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Y, SWIZZLE_X, SWIZZLE_W>() == Vec4(1, 2, 1, 4));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_X>() == Vec4(1, 2, 2, 1));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_Y>() == Vec4(1, 2, 2, 2));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_Z>() == Vec4(1, 2, 2, 3));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_W>() == Vec4(1, 2, 2, 4));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_X>() == Vec4(1, 2, 3, 1));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_Y>() == Vec4(1, 2, 3, 2));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_Z>() == Vec4(1, 2, 3, 3));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_W>() == Vec4(1, 2, 3, 4));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Y, SWIZZLE_W, SWIZZLE_X>() == Vec4(1, 2, 4, 1));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Y, SWIZZLE_W, SWIZZLE_Y>() == Vec4(1, 2, 4, 2));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Y, SWIZZLE_W, SWIZZLE_Z>() == Vec4(1, 2, 4, 3));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Y, SWIZZLE_W, SWIZZLE_W>() == Vec4(1, 2, 4, 4));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Z, SWIZZLE_X, SWIZZLE_X>() == Vec4(1, 3, 1, 1));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Z, SWIZZLE_X, SWIZZLE_Y>() == Vec4(1, 3, 1, 2));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Z, SWIZZLE_X, SWIZZLE_Z>() == Vec4(1, 3, 1, 3));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Z, SWIZZLE_X, SWIZZLE_W>() == Vec4(1, 3, 1, 4));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_X>() == Vec4(1, 3, 2, 1));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_Y>() == Vec4(1, 3, 2, 2));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_Z>() == Vec4(1, 3, 2, 3));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_W>() == Vec4(1, 3, 2, 4));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_X>() == Vec4(1, 3, 3, 1));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_Y>() == Vec4(1, 3, 3, 2));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_Z>() == Vec4(1, 3, 3, 3));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_W>() == Vec4(1, 3, 3, 4));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Z, SWIZZLE_W, SWIZZLE_X>() == Vec4(1, 3, 4, 1));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Z, SWIZZLE_W, SWIZZLE_Y>() == Vec4(1, 3, 4, 2));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Z, SWIZZLE_W, SWIZZLE_Z>() == Vec4(1, 3, 4, 3));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Z, SWIZZLE_W, SWIZZLE_W>() == Vec4(1, 3, 4, 4));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_W, SWIZZLE_X, SWIZZLE_X>() == Vec4(1, 4, 1, 1));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_W, SWIZZLE_X, SWIZZLE_Y>() == Vec4(1, 4, 1, 2));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_W, SWIZZLE_X, SWIZZLE_Z>() == Vec4(1, 4, 1, 3));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_W, SWIZZLE_X, SWIZZLE_W>() == Vec4(1, 4, 1, 4));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_W, SWIZZLE_Y, SWIZZLE_X>() == Vec4(1, 4, 2, 1));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_W, SWIZZLE_Y, SWIZZLE_Y>() == Vec4(1, 4, 2, 2));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_W, SWIZZLE_Y, SWIZZLE_Z>() == Vec4(1, 4, 2, 3));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_W, SWIZZLE_Y, SWIZZLE_W>() == Vec4(1, 4, 2, 4));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_W, SWIZZLE_Z, SWIZZLE_X>() == Vec4(1, 4, 3, 1));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_W, SWIZZLE_Z, SWIZZLE_Y>() == Vec4(1, 4, 3, 2));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_W, SWIZZLE_Z, SWIZZLE_Z>() == Vec4(1, 4, 3, 3));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_W, SWIZZLE_Z, SWIZZLE_W>() == Vec4(1, 4, 3, 4));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_W, SWIZZLE_W, SWIZZLE_X>() == Vec4(1, 4, 4, 1));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_W, SWIZZLE_W, SWIZZLE_Y>() == Vec4(1, 4, 4, 2));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_W, SWIZZLE_W, SWIZZLE_Z>() == Vec4(1, 4, 4, 3));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_W, SWIZZLE_W, SWIZZLE_W>() == Vec4(1, 4, 4, 4));

		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_X, SWIZZLE_X, SWIZZLE_X>() == Vec4(2, 1, 1, 1));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_X, SWIZZLE_X, SWIZZLE_Y>() == Vec4(2, 1, 1, 2));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_X, SWIZZLE_X, SWIZZLE_Z>() == Vec4(2, 1, 1, 3));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_X, SWIZZLE_X, SWIZZLE_W>() == Vec4(2, 1, 1, 4));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_X, SWIZZLE_Y, SWIZZLE_X>() == Vec4(2, 1, 2, 1));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_X, SWIZZLE_Y, SWIZZLE_Y>() == Vec4(2, 1, 2, 2));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_X, SWIZZLE_Y, SWIZZLE_Z>() == Vec4(2, 1, 2, 3));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_X, SWIZZLE_Y, SWIZZLE_W>() == Vec4(2, 1, 2, 4));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_X, SWIZZLE_Z, SWIZZLE_X>() == Vec4(2, 1, 3, 1));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_X, SWIZZLE_Z, SWIZZLE_Y>() == Vec4(2, 1, 3, 2));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_X, SWIZZLE_Z, SWIZZLE_Z>() == Vec4(2, 1, 3, 3));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_X, SWIZZLE_Z, SWIZZLE_W>() == Vec4(2, 1, 3, 4));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_X, SWIZZLE_W, SWIZZLE_X>() == Vec4(2, 1, 4, 1));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_X, SWIZZLE_W, SWIZZLE_Y>() == Vec4(2, 1, 4, 2));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_X, SWIZZLE_W, SWIZZLE_Z>() == Vec4(2, 1, 4, 3));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_X, SWIZZLE_W, SWIZZLE_W>() == Vec4(2, 1, 4, 4));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_X, SWIZZLE_X>() == Vec4(2, 2, 1, 1));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_X, SWIZZLE_Y>() == Vec4(2, 2, 1, 2));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_X, SWIZZLE_Z>() == Vec4(2, 2, 1, 3));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_X, SWIZZLE_W>() == Vec4(2, 2, 1, 4));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_X>() == Vec4(2, 2, 2, 1));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_Y>() == Vec4(2, 2, 2, 2));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_Z>() == Vec4(2, 2, 2, 3));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_W>() == Vec4(2, 2, 2, 4));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_X>() == Vec4(2, 2, 3, 1));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_Y>() == Vec4(2, 2, 3, 2));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_Z>() == Vec4(2, 2, 3, 3));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_W>() == Vec4(2, 2, 3, 4));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_W, SWIZZLE_X>() == Vec4(2, 2, 4, 1));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_W, SWIZZLE_Y>() == Vec4(2, 2, 4, 2));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_W, SWIZZLE_Z>() == Vec4(2, 2, 4, 3));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_W, SWIZZLE_W>() == Vec4(2, 2, 4, 4));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_X, SWIZZLE_X>() == Vec4(2, 3, 1, 1));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_X, SWIZZLE_Y>() == Vec4(2, 3, 1, 2));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_X, SWIZZLE_Z>() == Vec4(2, 3, 1, 3));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_X, SWIZZLE_W>() == Vec4(2, 3, 1, 4));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_X>() == Vec4(2, 3, 2, 1));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_Y>() == Vec4(2, 3, 2, 2));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_Z>() == Vec4(2, 3, 2, 3));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_W>() == Vec4(2, 3, 2, 4));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_X>() == Vec4(2, 3, 3, 1));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_Y>() == Vec4(2, 3, 3, 2));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_Z>() == Vec4(2, 3, 3, 3));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_W>() == Vec4(2, 3, 3, 4));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_W, SWIZZLE_X>() == Vec4(2, 3, 4, 1));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_W, SWIZZLE_Y>() == Vec4(2, 3, 4, 2));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_W, SWIZZLE_Z>() == Vec4(2, 3, 4, 3));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_W, SWIZZLE_W>() == Vec4(2, 3, 4, 4));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_W, SWIZZLE_X, SWIZZLE_X>() == Vec4(2, 4, 1, 1));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_W, SWIZZLE_X, SWIZZLE_Y>() == Vec4(2, 4, 1, 2));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_W, SWIZZLE_X, SWIZZLE_Z>() == Vec4(2, 4, 1, 3));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_W, SWIZZLE_X, SWIZZLE_W>() == Vec4(2, 4, 1, 4));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_W, SWIZZLE_Y, SWIZZLE_X>() == Vec4(2, 4, 2, 1));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_W, SWIZZLE_Y, SWIZZLE_Y>() == Vec4(2, 4, 2, 2));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_W, SWIZZLE_Y, SWIZZLE_Z>() == Vec4(2, 4, 2, 3));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_W, SWIZZLE_Y, SWIZZLE_W>() == Vec4(2, 4, 2, 4));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_W, SWIZZLE_Z, SWIZZLE_X>() == Vec4(2, 4, 3, 1));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_W, SWIZZLE_Z, SWIZZLE_Y>() == Vec4(2, 4, 3, 2));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_W, SWIZZLE_Z, SWIZZLE_Z>() == Vec4(2, 4, 3, 3));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_W, SWIZZLE_Z, SWIZZLE_W>() == Vec4(2, 4, 3, 4));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_W, SWIZZLE_W, SWIZZLE_X>() == Vec4(2, 4, 4, 1));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_W, SWIZZLE_W, SWIZZLE_Y>() == Vec4(2, 4, 4, 2));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_W, SWIZZLE_W, SWIZZLE_Z>() == Vec4(2, 4, 4, 3));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_W, SWIZZLE_W, SWIZZLE_W>() == Vec4(2, 4, 4, 4));

		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_X, SWIZZLE_X, SWIZZLE_X>() == Vec4(3, 1, 1, 1));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_X, SWIZZLE_X, SWIZZLE_Y>() == Vec4(3, 1, 1, 2));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_X, SWIZZLE_X, SWIZZLE_Z>() == Vec4(3, 1, 1, 3));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_X, SWIZZLE_X, SWIZZLE_W>() == Vec4(3, 1, 1, 4));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_X, SWIZZLE_Y, SWIZZLE_X>() == Vec4(3, 1, 2, 1));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_X, SWIZZLE_Y, SWIZZLE_Y>() == Vec4(3, 1, 2, 2));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_X, SWIZZLE_Y, SWIZZLE_Z>() == Vec4(3, 1, 2, 3));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_X, SWIZZLE_Y, SWIZZLE_W>() == Vec4(3, 1, 2, 4));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_X, SWIZZLE_Z, SWIZZLE_X>() == Vec4(3, 1, 3, 1));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_X, SWIZZLE_Z, SWIZZLE_Y>() == Vec4(3, 1, 3, 2));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_X, SWIZZLE_Z, SWIZZLE_Z>() == Vec4(3, 1, 3, 3));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_X, SWIZZLE_Z, SWIZZLE_W>() == Vec4(3, 1, 3, 4));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_X, SWIZZLE_W, SWIZZLE_X>() == Vec4(3, 1, 4, 1));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_X, SWIZZLE_W, SWIZZLE_Y>() == Vec4(3, 1, 4, 2));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_X, SWIZZLE_W, SWIZZLE_Z>() == Vec4(3, 1, 4, 3));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_X, SWIZZLE_W, SWIZZLE_W>() == Vec4(3, 1, 4, 4));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_X, SWIZZLE_X>() == Vec4(3, 2, 1, 1));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_X, SWIZZLE_Y>() == Vec4(3, 2, 1, 2));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_X, SWIZZLE_Z>() == Vec4(3, 2, 1, 3));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_X, SWIZZLE_W>() == Vec4(3, 2, 1, 4));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_X>() == Vec4(3, 2, 2, 1));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_Y>() == Vec4(3, 2, 2, 2));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_Z>() == Vec4(3, 2, 2, 3));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_W>() == Vec4(3, 2, 2, 4));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_X>() == Vec4(3, 2, 3, 1));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_Y>() == Vec4(3, 2, 3, 2));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_Z>() == Vec4(3, 2, 3, 3));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_W>() == Vec4(3, 2, 3, 4));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_W, SWIZZLE_X>() == Vec4(3, 2, 4, 1));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_W, SWIZZLE_Y>() == Vec4(3, 2, 4, 2));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_W, SWIZZLE_Z>() == Vec4(3, 2, 4, 3));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_W, SWIZZLE_W>() == Vec4(3, 2, 4, 4));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_X, SWIZZLE_X>() == Vec4(3, 3, 1, 1));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_X, SWIZZLE_Y>() == Vec4(3, 3, 1, 2));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_X, SWIZZLE_Z>() == Vec4(3, 3, 1, 3));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_X, SWIZZLE_W>() == Vec4(3, 3, 1, 4));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_X>() == Vec4(3, 3, 2, 1));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_Y>() == Vec4(3, 3, 2, 2));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_Z>() == Vec4(3, 3, 2, 3));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_W>() == Vec4(3, 3, 2, 4));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_X>() == Vec4(3, 3, 3, 1));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_Y>() == Vec4(3, 3, 3, 2));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_Z>() == Vec4(3, 3, 3, 3));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_W>() == Vec4(3, 3, 3, 4));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_W, SWIZZLE_X>() == Vec4(3, 3, 4, 1));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_W, SWIZZLE_Y>() == Vec4(3, 3, 4, 2));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_W, SWIZZLE_Z>() == Vec4(3, 3, 4, 3));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_W, SWIZZLE_W>() == Vec4(3, 3, 4, 4));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_W, SWIZZLE_X, SWIZZLE_X>() == Vec4(3, 4, 1, 1));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_W, SWIZZLE_X, SWIZZLE_Y>() == Vec4(3, 4, 1, 2));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_W, SWIZZLE_X, SWIZZLE_Z>() == Vec4(3, 4, 1, 3));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_W, SWIZZLE_X, SWIZZLE_W>() == Vec4(3, 4, 1, 4));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_W, SWIZZLE_Y, SWIZZLE_X>() == Vec4(3, 4, 2, 1));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_W, SWIZZLE_Y, SWIZZLE_Y>() == Vec4(3, 4, 2, 2));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_W, SWIZZLE_Y, SWIZZLE_Z>() == Vec4(3, 4, 2, 3));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_W, SWIZZLE_Y, SWIZZLE_W>() == Vec4(3, 4, 2, 4));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_W, SWIZZLE_Z, SWIZZLE_X>() == Vec4(3, 4, 3, 1));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_W, SWIZZLE_Z, SWIZZLE_Y>() == Vec4(3, 4, 3, 2));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_W, SWIZZLE_Z, SWIZZLE_Z>() == Vec4(3, 4, 3, 3));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_W, SWIZZLE_Z, SWIZZLE_W>() == Vec4(3, 4, 3, 4));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_W, SWIZZLE_W, SWIZZLE_X>() == Vec4(3, 4, 4, 1));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_W, SWIZZLE_W, SWIZZLE_Y>() == Vec4(3, 4, 4, 2));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_W, SWIZZLE_W, SWIZZLE_Z>() == Vec4(3, 4, 4, 3));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_W, SWIZZLE_W, SWIZZLE_W>() == Vec4(3, 4, 4, 4));

		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_X, SWIZZLE_X, SWIZZLE_X>() == Vec4(4, 1, 1, 1));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_X, SWIZZLE_X, SWIZZLE_Y>() == Vec4(4, 1, 1, 2));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_X, SWIZZLE_X, SWIZZLE_Z>() == Vec4(4, 1, 1, 3));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_X, SWIZZLE_X, SWIZZLE_W>() == Vec4(4, 1, 1, 4));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_X, SWIZZLE_Y, SWIZZLE_X>() == Vec4(4, 1, 2, 1));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_X, SWIZZLE_Y, SWIZZLE_Y>() == Vec4(4, 1, 2, 2));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_X, SWIZZLE_Y, SWIZZLE_Z>() == Vec4(4, 1, 2, 3));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_X, SWIZZLE_Y, SWIZZLE_W>() == Vec4(4, 1, 2, 4));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_X, SWIZZLE_Z, SWIZZLE_X>() == Vec4(4, 1, 3, 1));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_X, SWIZZLE_Z, SWIZZLE_Y>() == Vec4(4, 1, 3, 2));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_X, SWIZZLE_Z, SWIZZLE_Z>() == Vec4(4, 1, 3, 3));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_X, SWIZZLE_Z, SWIZZLE_W>() == Vec4(4, 1, 3, 4));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_X, SWIZZLE_W, SWIZZLE_X>() == Vec4(4, 1, 4, 1));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_X, SWIZZLE_W, SWIZZLE_Y>() == Vec4(4, 1, 4, 2));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_X, SWIZZLE_W, SWIZZLE_Z>() == Vec4(4, 1, 4, 3));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_X, SWIZZLE_W, SWIZZLE_W>() == Vec4(4, 1, 4, 4));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Y, SWIZZLE_X, SWIZZLE_X>() == Vec4(4, 2, 1, 1));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Y, SWIZZLE_X, SWIZZLE_Y>() == Vec4(4, 2, 1, 2));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Y, SWIZZLE_X, SWIZZLE_Z>() == Vec4(4, 2, 1, 3));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Y, SWIZZLE_X, SWIZZLE_W>() == Vec4(4, 2, 1, 4));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_X>() == Vec4(4, 2, 2, 1));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_Y>() == Vec4(4, 2, 2, 2));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_Z>() == Vec4(4, 2, 2, 3));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_W>() == Vec4(4, 2, 2, 4));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_X>() == Vec4(4, 2, 3, 1));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_Y>() == Vec4(4, 2, 3, 2));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_Z>() == Vec4(4, 2, 3, 3));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_W>() == Vec4(4, 2, 3, 4));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Y, SWIZZLE_W, SWIZZLE_X>() == Vec4(4, 2, 4, 1));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Y, SWIZZLE_W, SWIZZLE_Y>() == Vec4(4, 2, 4, 2));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Y, SWIZZLE_W, SWIZZLE_Z>() == Vec4(4, 2, 4, 3));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Y, SWIZZLE_W, SWIZZLE_W>() == Vec4(4, 2, 4, 4));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Z, SWIZZLE_X, SWIZZLE_X>() == Vec4(4, 3, 1, 1));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Z, SWIZZLE_X, SWIZZLE_Y>() == Vec4(4, 3, 1, 2));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Z, SWIZZLE_X, SWIZZLE_Z>() == Vec4(4, 3, 1, 3));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Z, SWIZZLE_X, SWIZZLE_W>() == Vec4(4, 3, 1, 4));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_X>() == Vec4(4, 3, 2, 1));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_Y>() == Vec4(4, 3, 2, 2));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_Z>() == Vec4(4, 3, 2, 3));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_W>() == Vec4(4, 3, 2, 4));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_X>() == Vec4(4, 3, 3, 1));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_Y>() == Vec4(4, 3, 3, 2));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_Z>() == Vec4(4, 3, 3, 3));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_W>() == Vec4(4, 3, 3, 4));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Z, SWIZZLE_W, SWIZZLE_X>() == Vec4(4, 3, 4, 1));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Z, SWIZZLE_W, SWIZZLE_Y>() == Vec4(4, 3, 4, 2));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Z, SWIZZLE_W, SWIZZLE_Z>() == Vec4(4, 3, 4, 3));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_Z, SWIZZLE_W, SWIZZLE_W>() == Vec4(4, 3, 4, 4));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_W, SWIZZLE_X, SWIZZLE_X>() == Vec4(4, 4, 1, 1));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_W, SWIZZLE_X, SWIZZLE_Y>() == Vec4(4, 4, 1, 2));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_W, SWIZZLE_X, SWIZZLE_Z>() == Vec4(4, 4, 1, 3));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_W, SWIZZLE_X, SWIZZLE_W>() == Vec4(4, 4, 1, 4));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_W, SWIZZLE_Y, SWIZZLE_X>() == Vec4(4, 4, 2, 1));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_W, SWIZZLE_Y, SWIZZLE_Y>() == Vec4(4, 4, 2, 2));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_W, SWIZZLE_Y, SWIZZLE_Z>() == Vec4(4, 4, 2, 3));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_W, SWIZZLE_Y, SWIZZLE_W>() == Vec4(4, 4, 2, 4));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_W, SWIZZLE_Z, SWIZZLE_X>() == Vec4(4, 4, 3, 1));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_W, SWIZZLE_Z, SWIZZLE_Y>() == Vec4(4, 4, 3, 2));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_W, SWIZZLE_Z, SWIZZLE_Z>() == Vec4(4, 4, 3, 3));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_W, SWIZZLE_Z, SWIZZLE_W>() == Vec4(4, 4, 3, 4));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_W, SWIZZLE_W, SWIZZLE_X>() == Vec4(4, 4, 4, 1));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_W, SWIZZLE_W, SWIZZLE_Y>() == Vec4(4, 4, 4, 2));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_W, SWIZZLE_W, SWIZZLE_Z>() == Vec4(4, 4, 4, 3));
		CHECK(v.Swizzle<SWIZZLE_W, SWIZZLE_W, SWIZZLE_W, SWIZZLE_W>() == Vec4(4, 4, 4, 4));
	}

	TEST_CASE("TestVec4Abs")
	{
		CHECK(Vec4(1, -2, 3, -4).Abs() == Vec4(1, 2, 3, 4));
		CHECK(Vec4(-1, 2, -3, 4).Abs() == Vec4(1, 2, 3, 4));
	}


	TEST_CASE("TestVec4Dot")
	{
		CHECK(Vec4(1, 2, 3, 4).Dot(Vec4(5, 6, 7, 8)) == float(1 * 5 + 2 * 6 + 3 * 7 + 4 * 8));
		CHECK(Vec4(1, 2, 3, 4).DotV(Vec4(5, 6, 7, 8)) == Vec4::sReplicate(1 * 5 + 2 * 6 + 3 * 7 + 4 * 8));
	}

	TEST_CASE("TestVec4Length")
	{
		CHECK(Vec4(1, 2, 3, 4).LengthSq() == float(1 + 4 + 9 + 16));
		CHECK(Vec4(1, 2, 3, 4).Length() == sqrt(float(1 + 4 + 9 + 16)));
	}

	TEST_CASE("TestVec4Sqrt")
	{
		CHECK_APPROX_EQUAL(Vec4(13, 15, 17, 19).Sqrt(), Vec4(sqrt(13.0f), sqrt(15.0f), sqrt(17.0f), sqrt(19.0f)));
	}

	TEST_CASE("TestVec4Normalize")
	{
		CHECK(Vec4(1, 2, 3, 4).Normalized() == Vec4(1, 2, 3, 4) / sqrt(30.0f));
	}

	TEST_CASE("TestVec4Cast")
	{
		CHECK(Vec4(1, 2, 3, 4).ToInt() == UVec4(1, 2, 3, 4));
		CHECK(Vec4(1, 2, 3, 4).ReinterpretAsInt() == UVec4(0x3f800000U, 0x40000000U, 0x40400000U, 0x40800000U));
	}

	TEST_CASE("TestVec4Sign")
	{
		CHECK(Vec4(1.2345f, -6.7891f, 0, 1).GetSign() == Vec4(1, -1, 1, 1));
		CHECK(Vec4(0, 2.3456f, -7.8912f, -1).GetSign() == Vec4(1, 1, -1, -1));
	}

	TEST_CASE("TestVec4SignBit")
	{
		CHECK(Vec4(2, -3, 4, -5).GetSignBits() == 0b1010);
		CHECK(Vec4(-2, 3, -4, 5).GetSignBits() == 0b0101);
	}

	TEST_CASE("TestVec4Sort")
	{
		for (int i = 0; i < 4; ++i)
			for (int j = 0; j < 4; ++j)
				if (i != j)
					for (int k = 0; k < 4; ++k)
						if (i != k && j != k)
							for (int l = 0; l < 4; ++l)
								if (i != l && j != l && k != l)
								{
									Vec4 v1((float)i, (float)j, (float)k, (float)l);
									Vec4 v2 = v1;
									UVec4 idx1 = UVec4(i + 4, j + 4, k + 4, l + 4);
									UVec4 idx2 = idx1;
									Vec4::sSort4(v1, idx1);
									Vec4::sSort4Reverse(v2, idx2);
									for (int m = 0; m < 4; ++m)
									{
										CHECK(v1[m] == float(m));
										CHECK(v2[m] == float(3 - m));
										CHECK(idx1[m] == uint32(m + 4));
										CHECK(idx2[m] == uint32(3 - m + 4));
									}
								}
	}

	TEST_CASE("TestVec4SinCos")
	{
		// Check edge cases
		Vec4 vs, vc;
		Vec4(0, 0.5f * JPH_PI, JPH_PI, -0.5f * JPH_PI).SinCos(vs, vc);
		CHECK(vs.IsClose(Vec4(0, 1, 0, -1), 1.0e-7f));
		CHECK(vc.IsClose(Vec4(1, 0, -1, 0), 1.0e-7f));

		double ms = 0.0, mc = 0.0;

		for (float x = -100.0f * JPH_PI; x < 100.0f * JPH_PI; x += 1.0e-3f)
		{
			// Create a vector with intermediate values
			Vec4 xv = Vec4::sReplicate(x) + Vec4(0.0e-4f, 2.5e-4f, 5.0e-4f, 7.5e-4f);

			// Calculate sin and cos
			xv.SinCos(vs, vc);

			for (int i = 0; i < 4; ++i)
			{
				// Check accuracy of sin
				double s1 = sin((double)xv[i]), s2 = (double)vs[i];
				double ds = abs(s2 - s1);
				ms = max(ms, ds);

				// Check accuracy of cos
				double c1 = cos((double)xv[i]), c2 = (double)vc[i];
				double dc = abs(c2 - c1);
				mc = max(mc, dc);
			}
		}

		CHECK(ms < 1.0e-7);
		CHECK(mc < 1.0e-7);
	}

	TEST_CASE("TestVec4Tan")
	{
		// Check edge cases
		CHECK(Vec4::sReplicate(0.0f).Tan() == Vec4::sZero());
		CHECK(Vec4::sReplicate(0.5f * JPH_PI - 1.0e-6f).Tan().GetX() > 1.0e6f);
		CHECK(Vec4::sReplicate(0.5f * JPH_PI + 1.0e-6f).Tan().GetX() < -1.0e6f);

		double mt = 0.0;

		for (float x = -100.0f * JPH_PI; x < 100.0f * JPH_PI; x += 1.0e-3f)
		{
			// Create a vector with intermediate values
			Vec4 xv = Vec4::sReplicate(x) + Vec4(0.0e-4f, 2.5e-4f, 5.0e-4f, 7.5e-4f);

			// Calculate tan
			Vec4 vt = xv.Tan();

			for (int i = 0; i < 4; ++i)
			{
				// Check accuracy of tan
				double t1 = tan((double)xv[i]), t2 = (double)vt[i];
				double dt = abs(t2 - t1);
				mt = max(mt, dt) / max(1.0, abs(t1)); // Take relative error
			}
		}

		CHECK(mt < 1.5e-7);
	}

	TEST_CASE("TestVec4ASin")
	{
		// Check edge cases
		CHECK(Vec4::sReplicate(0.0f).ASin() == Vec4::sZero());
		CHECK(Vec4::sReplicate(1.0f).ASin() == Vec4::sReplicate(0.5f * JPH_PI));
		CHECK(Vec4::sReplicate(-1.0f).ASin() == Vec4::sReplicate(-0.5f * JPH_PI));

		double ma = 0.0;

		for (float x = -1.0f; x <= 1.0f; x += 1.0e-3f)
		{
			// Create a vector with intermediate values
			Vec4 xv = Vec4::sMin(Vec4::sReplicate(x) + Vec4(0.0e-4f, 2.5e-4f, 5.0e-4f, 7.5e-4f), Vec4::sReplicate(1.0f));

			// Calculate asin
			Vec4 va = xv.ASin();

			for (int i = 0; i < 4; ++i)
			{
				// Check accuracy of asin
				double a1 = asin((double)xv[i]), a2 = (double)va[i];
				double da = abs(a2 - a1);
				ma = max(ma, da);
			}
		}

		CHECK(ma < 2.0e-7);

		// Check that inputs are clamped as promised
		CHECK(Vec4::sReplicate(-1.1f).ASin() == Vec4::sReplicate(-0.5f * JPH_PI));
		CHECK(Vec4::sReplicate(1.1f).ASin() == Vec4::sReplicate(0.5f * JPH_PI));
	}

	TEST_CASE("TestVec4ACos")
	{
		// Check edge cases
		CHECK(Vec4::sReplicate(0.0f).ACos() == Vec4::sReplicate(0.5f * JPH_PI));
		CHECK(Vec4::sReplicate(1.0f).ACos() == Vec4::sZero());
		CHECK(Vec4::sReplicate(-1.0f).ACos() == Vec4::sReplicate(JPH_PI));

		double ma = 0.0;

		for (float x = -1.0f; x <= 1.0f; x += 1.0e-3f)
		{
			// Create a vector with intermediate values
			Vec4 xv = Vec4::sMin(Vec4::sReplicate(x) + Vec4(0.0e-4f, 2.5e-4f, 5.0e-4f, 7.5e-4f), Vec4::sReplicate(1.0f));

			// Calculate acos
			Vec4 va = xv.ACos();

			for (int i = 0; i < 4; ++i)
			{
				// Check accuracy of acos
				double a1 = acos((double)xv[i]), a2 = (double)va[i];
				double da = abs(a2 - a1);
				ma = max(ma, da);
			}
		}

		CHECK(ma < 3.5e-7);

		// Check that inputs are clamped as promised
		CHECK(Vec4::sReplicate(-1.1f).ACos() == Vec4::sReplicate(JPH_PI));
		CHECK(Vec4::sReplicate(1.1f).ACos() == Vec4::sZero());
	}

	TEST_CASE("TestVec4ATan")
	{
		// Check edge cases
		CHECK(Vec4::sReplicate(0.0f).ATan() == Vec4::sZero());
		CHECK(Vec4::sReplicate(FLT_MAX).ATan() == Vec4::sReplicate(0.5f * JPH_PI));
		CHECK(Vec4::sReplicate(-FLT_MAX).ATan() == Vec4::sReplicate(-0.5f * JPH_PI));

		double ma = 0.0;

		for (float x = -100.0f; x < 100.0f; x += 1.0e-3f)
		{
			// Create a vector with intermediate values
			Vec4 xv = Vec4::sReplicate(x) + Vec4(0.0e-4f, 2.5e-4f, 5.0e-4f, 7.5e-4f);

			// Calculate atan
			Vec4 va = xv.ATan();

			for (int i = 0; i < 4; ++i)
			{
				// Check accuracy of atan
				double a1 = atan((double)xv[i]), a2 = (double)va[i];
				double da = abs(a2 - a1);
				ma = max(ma, da);
			}
		}

		CHECK(ma < 1.5e-7);
	}

	TEST_CASE("TestVec4ATan2")
	{
		double ma = 0.0;

		// Test the axis
		CHECK(Vec4::sATan2(Vec4::sZero(), Vec4::sReplicate(10.0f)) == Vec4::sZero());
		CHECK(Vec4::sATan2(Vec4::sZero(), Vec4::sReplicate(-10.0f)) == Vec4::sReplicate(JPH_PI));
		CHECK(Vec4::sATan2(Vec4::sReplicate(10.0f), Vec4::sZero()) == Vec4::sReplicate(0.5f * JPH_PI));
		CHECK(Vec4::sATan2(Vec4::sReplicate(-10.0f), Vec4::sZero()) == Vec4::sReplicate(-0.5f * JPH_PI));

		// Test the 4 quadrants
		CHECK(Vec4::sATan2(Vec4::sReplicate(10.0f), Vec4::sReplicate(10.0f)) == Vec4::sReplicate(0.25f * JPH_PI));
		CHECK(Vec4::sATan2(Vec4::sReplicate(10.0f), Vec4::sReplicate(-10.0f)) == Vec4::sReplicate(0.75f * JPH_PI));
		CHECK(Vec4::sATan2(Vec4::sReplicate(-10.0f), Vec4::sReplicate(-10.0f)) == Vec4::sReplicate(-0.75f * JPH_PI));
		CHECK(Vec4::sATan2(Vec4::sReplicate(-10.0f), Vec4::sReplicate(10.0f)) == Vec4::sReplicate(-0.25f * JPH_PI));

		for (float y = -5.0f; y < 5.0f; y += 1.0e-2f)
		{
			// Create a vector with intermediate values
			Vec4 yv = Vec4::sReplicate(y) + Vec4(0.0e-3f, 2.5e-3f, 5.0e-3f, 7.5e-3f);

			for (float x = -5.0f; x < 5.0f; x += 1.0e-2f)
			{
				// Create a vector with intermediate values
				Vec4 xv = Vec4::sReplicate(x) + Vec4(0.0e-3f, 2.5e-3f, 5.0e-3f, 7.5e-3f);

				// Calculate atan
				Vec4 va = Vec4::sATan2(yv, xv);

				for (int i = 0; i < 4; ++i)
				{
					// Check accuracy of atan
					double a1 = atan2((double)yv[i], (double)xv[i]), a2 = (double)va[i];
					double da = abs(a2 - a1);
					ma = max(ma, da);
				}
			}
		}

		CHECK(ma < 3.0e-7);
	}
}
