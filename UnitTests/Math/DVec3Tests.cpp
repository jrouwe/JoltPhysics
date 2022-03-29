// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include <Jolt/Math/DVec3.h>

#ifdef JPH_USE_AVX2

TEST_SUITE("DVec3Tests")
{
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

	TEST_CASE("TestDVec3LoadDouble3Unsafe")
	{
		double test[4] = { 1, 2, 3, 4 };
		DVec3 v = DVec3::sLoadDouble3Unsafe(test);
		DVec3 v2(1, 2, 3);
		CHECK(v == v2);
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
}

#endif // JPH_USE_AVX2
