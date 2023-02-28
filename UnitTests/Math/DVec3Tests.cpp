// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include <Jolt/Math/DVec3.h>

TEST_SUITE("DVec3Tests")
{
	TEST_CASE("TestDVec3Zero")
	{
		DVec3 v = DVec3::sZero();

		CHECK(v.GetX() == 0);
		CHECK(v.GetY() == 0);
		CHECK(v.GetZ() == 0);
	}

	TEST_CASE("TestVec3NaN")
	{
		DVec3 v = DVec3::sNaN();

		CHECK(isnan(v.GetX()));
		CHECK(isnan(v.GetY()));
		CHECK(isnan(v.GetZ()));
		CHECK(v.IsNaN());

		v.SetComponent(0, 0);
		CHECK(v.IsNaN());
		v.SetComponent(1, 0);
		CHECK(v.IsNaN());
		v.SetComponent(2, 0);
		CHECK(!v.IsNaN());
	}

	TEST_CASE("TestDVec3ConstructComponents")
	{
		DVec3 v(1, 2, 3);

		// Test component access
		CHECK(v.GetX() == 1);
		CHECK(v.GetY() == 2);
		CHECK(v.GetZ() == 3);

		// Test component access by [] operators
		CHECK(v[0] == 1);
		CHECK(v[1] == 2);
		CHECK(v[2] == 3);

		// Test == and != operators
		CHECK(v == DVec3(1, 2, 3));
		CHECK(v != DVec3(1, 2, 4));

		// Set the components
		v.SetComponent(0, 4);
		v.SetComponent(1, 5);
		v.SetComponent(2, 6);
		CHECK(v == DVec3(4, 5, 6));
	}

	TEST_CASE("TestVec4ToDVec3")
	{
		CHECK(DVec3(Vec4(1, 3, 5, 7)) == DVec3(1, 3, 5));
	}

	TEST_CASE("TestDVec3Replicate")
	{
		CHECK(DVec3::sReplicate(2) == DVec3(2, 2, 2));
	}

	TEST_CASE("TestDVec3ToVec3")
	{
		CHECK(Vec3(DVec3(1, 3, 5)) == Vec3(1, 3, 5));

		// Check rounding up and down
		CHECK(DVec3(2.0, 0x1.0000000000001p1, -0x1.0000000000001p1).ToVec3RoundUp() == Vec3(2.0, 0x1.000002p1f, -2.0));
		CHECK(DVec3(2.0, 0x1.0000000000001p1, -0x1.0000000000001p1).ToVec3RoundDown() == Vec3(2.0, 2.0, -0x1.000002p1f));
	}

	TEST_CASE("TestVec3MinMax")
	{
		DVec3 v1(1, 5, 3);
		DVec3 v2(4, 2, 6);

		CHECK(DVec3::sMin(v1, v2) == DVec3(1, 2, 3));
		CHECK(DVec3::sMax(v1, v2) == DVec3(4, 5, 6));
	}

	TEST_CASE("TestDVec3Clamp")
	{
		DVec3 v1(1, 2, 3);
		DVec3 v2(4, 5, 6);
		DVec3 v(-1, 3, 7);

		CHECK(DVec3::sClamp(v, v1, v2) == DVec3(1, 3, 6));
	}

	TEST_CASE("TestDVec3Trues")
	{
		CHECK(DVec3(DVec3::cFalse, DVec3::cFalse, DVec3::cFalse).GetTrues() == 0b0000);
		CHECK(DVec3(DVec3::cTrue, DVec3::cFalse, DVec3::cFalse).GetTrues() == 0b0001);
		CHECK(DVec3(DVec3::cFalse, DVec3::cTrue, DVec3::cFalse).GetTrues() == 0b0010);
		CHECK(DVec3(DVec3::cTrue, DVec3::cTrue, DVec3::cFalse).GetTrues() == 0b0011);
		CHECK(DVec3(DVec3::cFalse, DVec3::cFalse, DVec3::cTrue).GetTrues() == 0b0100);
		CHECK(DVec3(DVec3::cTrue, DVec3::cFalse, DVec3::cTrue).GetTrues() == 0b0101);
		CHECK(DVec3(DVec3::cFalse, DVec3::cTrue, DVec3::cTrue).GetTrues() == 0b0110);
		CHECK(DVec3(DVec3::cTrue, DVec3::cTrue, DVec3::cTrue).GetTrues() == 0b0111);

		CHECK(!DVec3(DVec3::cFalse, DVec3::cFalse, DVec3::cFalse).TestAnyTrue());
		CHECK(DVec3(DVec3::cTrue, DVec3::cFalse, DVec3::cFalse).TestAnyTrue());
		CHECK(DVec3(DVec3::cFalse, DVec3::cTrue, DVec3::cFalse).TestAnyTrue());
		CHECK(DVec3(DVec3::cTrue, DVec3::cTrue, DVec3::cFalse).TestAnyTrue());
		CHECK(DVec3(DVec3::cFalse, DVec3::cFalse, DVec3::cTrue).TestAnyTrue());
		CHECK(DVec3(DVec3::cTrue, DVec3::cFalse, DVec3::cTrue).TestAnyTrue());
		CHECK(DVec3(DVec3::cFalse, DVec3::cTrue, DVec3::cTrue).TestAnyTrue());
		CHECK(DVec3(DVec3::cTrue, DVec3::cTrue, DVec3::cTrue).TestAnyTrue());

		CHECK(!DVec3(DVec3::cFalse, DVec3::cFalse, DVec3::cFalse).TestAllTrue());
		CHECK(!DVec3(DVec3::cTrue, DVec3::cFalse, DVec3::cFalse).TestAllTrue());
		CHECK(!DVec3(DVec3::cFalse, DVec3::cTrue, DVec3::cFalse).TestAllTrue());
		CHECK(!DVec3(DVec3::cTrue, DVec3::cTrue, DVec3::cFalse).TestAllTrue());
		CHECK(!DVec3(DVec3::cFalse, DVec3::cFalse, DVec3::cTrue).TestAllTrue());
		CHECK(!DVec3(DVec3::cTrue, DVec3::cFalse, DVec3::cTrue).TestAllTrue());
		CHECK(!DVec3(DVec3::cFalse, DVec3::cTrue, DVec3::cTrue).TestAllTrue());
		CHECK(DVec3(DVec3::cTrue, DVec3::cTrue, DVec3::cTrue).TestAllTrue());
	}

	TEST_CASE("TestDVec3Comparisons")
	{
		CHECK(DVec3::sEquals(DVec3(1, 2, 3), DVec3(1, 4, 3)).GetTrues() == 0b101); // Can't directly check if equal to (true, false, true) because true = -NaN and -NaN != -NaN
		CHECK(DVec3::sLess(DVec3(1, 2, 4), DVec3(1, 4, 3)).GetTrues() == 0b010);
		CHECK(DVec3::sLessOrEqual(DVec3(1, 2, 4), DVec3(1, 4, 3)).GetTrues() == 0b011);
		CHECK(DVec3::sGreater(DVec3(1, 2, 4), DVec3(1, 4, 3)).GetTrues() == 0b100);
		CHECK(DVec3::sGreaterOrEqual(DVec3(1, 2, 4), DVec3(1, 4, 3)).GetTrues() == 0b101);
	}

	TEST_CASE("TestDVec3FMA")
	{
		CHECK(DVec3::sFusedMultiplyAdd(DVec3(1, 2, 3), DVec3(4, 5, 6), DVec3(7, 8, 9)) == DVec3(1 * 4 + 7, 2 * 5 + 8, 3 * 6 + 9));
	}

	TEST_CASE("TestDVec3Select")
	{
		CHECK(DVec3::sSelect(DVec3(1, 2, 3), DVec3(4, 5, 6), DVec3(DVec3::cTrue, DVec3::cFalse, DVec3::cTrue)) == DVec3(4, 2, 6));
		CHECK(DVec3::sSelect(DVec3(1, 2, 3), DVec3(4, 5, 6), DVec3(DVec3::cFalse, DVec3::cTrue, DVec3::cFalse)) == DVec3(1, 5, 3));
	}

	TEST_CASE("TestDVec3BitOps")
	{
		// Test all bit permutations
		DVec3 v1(BitCast<double, uint64>(0b0011), BitCast<double, uint64>(0b00110), BitCast<double, uint64>(0b001100));
		DVec3 v2(BitCast<double, uint64>(0b0101), BitCast<double, uint64>(0b01010), BitCast<double, uint64>(0b010100));

		CHECK(DVec3::sOr(v1, v2) == DVec3(BitCast<double, uint64>(0b0111), BitCast<double, uint64>(0b01110), BitCast<double, uint64>(0b011100)));
		CHECK(DVec3::sXor(v1, v2) == DVec3(BitCast<double, uint64>(0b0110), BitCast<double, uint64>(0b01100), BitCast<double, uint64>(0b011000)));
		CHECK(DVec3::sAnd(v1, v2) == DVec3(BitCast<double, uint64>(0b0001), BitCast<double, uint64>(0b00010), BitCast<double, uint64>(0b000100)));
	}

	TEST_CASE("TestDVec3Close")
	{
		CHECK(DVec3(1, 2, 3).IsClose(DVec3(1.001, 2.001, 3.001), 1.0e-4));
		CHECK(!DVec3(1, 2, 3).IsClose(DVec3(1.001, 2.001, 3.001), 1.0e-6));

		CHECK(DVec3(1.001, 0, 0).IsNormalized(1.0e-2));
		CHECK(!DVec3(0, 1.001, 0).IsNormalized(1.0e-4));

		CHECK(DVec3(-1.0e-7, 1.0e-7, 1.0e-8).IsNearZero(1.0e-12));
		CHECK(!DVec3(-1.0e-7, 1.0e-7, -1.0e-5).IsNearZero(1.0e-12));
	}

	TEST_CASE("TestDVec3Operators")
	{
		CHECK(-DVec3(1, 2, 3) == DVec3(-1, -2, -3));

		CHECK(DVec3(1, 2, 3) + Vec3(4, 5, 6) == DVec3(5, 7, 9));
		CHECK(DVec3(1, 2, 3) - Vec3(6, 5, 4) == DVec3(-5, -3, -1));

		CHECK(DVec3(1, 2, 3) + DVec3(4, 5, 6) == DVec3(5, 7, 9));
		CHECK(DVec3(1, 2, 3) - DVec3(6, 5, 4) == DVec3(-5, -3, -1));

		CHECK(DVec3(1, 2, 3) * DVec3(4, 5, 6) == DVec3(4, 10, 18));
		CHECK(DVec3(1, 2, 3) * 2 == DVec3(2, 4, 6));
		CHECK(4 * DVec3(1, 2, 3) == DVec3(4, 8, 12));

		CHECK(DVec3(1, 2, 3) / 2 == DVec3(0.5, 1.0, 1.5));
		CHECK(DVec3(1, 2, 3) / DVec3(2, 8, 24) == DVec3(0.5, 0.25, 0.125));

		DVec3 v = DVec3(1, 2, 3);
		v *= DVec3(4, 5, 6);
		CHECK(v == DVec3(4, 10, 18));
		v *= 2;
		CHECK(v == DVec3(8, 20, 36));
		v /= 2;
		CHECK(v == DVec3(4, 10, 18));
		v += DVec3(1, 2, 3);
		CHECK(v == DVec3(5, 12, 21));
		v -= DVec3(1, 2, 3);
		CHECK(v == DVec3(4, 10, 18));
		v += Vec3(1, 2, 3);
		CHECK(v == DVec3(5, 12, 21));
		v -= Vec3(1, 2, 3);
		CHECK(v == DVec3(4, 10, 18));

		CHECK(DVec3(2, 4, 8).Reciprocal() == DVec3(0.5, 0.25, 0.125));
	}

	TEST_CASE("TestDVec3Abs")
	{
		CHECK(DVec3(1, -2, 3).Abs() == DVec3(1, 2, 3));
		CHECK(DVec3(-1, 2, -3).Abs() == DVec3(1, 2, 3));
	}

	TEST_CASE("TestDVec3Dot")
	{
		CHECK(DVec3(2, 3, 4).Dot(DVec3(5, 6, 7)) == double(2 * 5 + 3 * 6 + 4 * 7));
	}

	TEST_CASE("TestDVec3Length")
	{
		CHECK(DVec3(2, 3, 4).LengthSq() == double(4 + 9 + 16));
		CHECK(DVec3(2, 3, 4).Length() == sqrt(double(4 + 9 + 16)));
	}

	TEST_CASE("TestDVec3Sqrt")
	{
		CHECK(DVec3(13, 15, 17).Sqrt() == DVec3(sqrt(13.0), sqrt(15.0), sqrt(17.0)));
	}

	TEST_CASE("TestDVec3Equals")
	{
		CHECK_FALSE(DVec3(13, 15, 17) == DVec3(13, 15, 19));
		CHECK(DVec3(13, 15, 17) == DVec3(13, 15, 17));
		CHECK(DVec3(13, 15, 17) != DVec3(13, 15, 19));
	}

	TEST_CASE("TestDVec3LoadStoreDouble3Unsafe")
	{
		double d4[4] = { 1, 2, 3, 4 };
		Double3 &d3 = *(Double3 *)d4;
		DVec3 v = DVec3::sLoadDouble3Unsafe(d3);
		DVec3 v2(1, 2, 3);
		CHECK(v == v2);

		Double3 d3_out;
		DVec3(1, 2, 3).StoreDouble3(&d3_out);
		CHECK(d3 == d3_out);
	}

	TEST_CASE("TestDVec3Cross")
	{
		CHECK(DVec3(1, 0, 0).Cross(DVec3(0, 1, 0)) == DVec3(0, 0, 1));
		CHECK(DVec3(0, 1, 0).Cross(DVec3(1, 0, 0)) == DVec3(0, 0, -1));
		CHECK(DVec3(0, 1, 0).Cross(DVec3(0, 0, 1)) == DVec3(1, 0, 0));
		CHECK(DVec3(0, 0, 1).Cross(DVec3(0, 1, 0)) == DVec3(-1, 0, 0));
		CHECK(DVec3(0, 0, 1).Cross(DVec3(1, 0, 0)) == DVec3(0, 1, 0));
		CHECK(DVec3(1, 0, 0).Cross(DVec3(0, 0, 1)) == DVec3(0, -1, 0));
	}

	TEST_CASE("TestDVec3Normalize")
	{
		CHECK(DVec3(3, 2, 1).Normalized() == DVec3(3, 2, 1) / sqrt(9.0 + 4.0 + 1.0));
	}

	TEST_CASE("TestDVec3Sign")
	{
		CHECK(DVec3(1.2345, -6.7891, 0).GetSign() == DVec3(1, -1, 1));
		CHECK(DVec3(0, 2.3456, -7.8912).GetSign() == DVec3(1, 1, -1));
	}
}
