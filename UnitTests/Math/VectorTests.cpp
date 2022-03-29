// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include <Jolt/Math/Vector.h>

TEST_SUITE("VectorTests")
{
	TEST_CASE("TestVectorMultiplyFloat")
	{
		Vector<5> v;
		v[0] = 1;
		v[1] = 2;
		v[2] = 3;
		v[3] = 4;
		v[4] = 5;
		Vector<5> v2 = v * 2;
		CHECK(v2[0] == 2.0f);
		CHECK(v2[1] == 4.0f);
		CHECK(v2[2] == 6.0f);
		CHECK(v2[3] == 8.0f);
		CHECK(v2[4] == 10.0f);
	}

	TEST_CASE("TestVectorAdd")
	{
		Vector<5> v1 = Vector<5>::sZero();
		Vector<5> v2 = Vector<5>::sZero();
		v1[0] = 1;
		v2[0] = 2;
		v1[4] = 5;
		Vector<5> v3 = v1 + v2;
		CHECK(v3[0] == 3.0f);
		CHECK(v3[1] == 0.0f);
		CHECK(v3[2] == 0.0f);
		CHECK(v3[3] == 0.0f);
		CHECK(v3[4] == 5.0f);
	}

	TEST_CASE("TestVectorNegate")
	{
		Vector<5> v;
		v[0] = 1;
		v[1] = 2;
		v[2] = 3;
		v[3] = 4;
		v[4] = 5;
		Vector<5> v2 = -v;
		CHECK(v2[0] == -1.0f);
		CHECK(v2[1] == -2.0f);
		CHECK(v2[2] == -3.0f);
		CHECK(v2[3] == -4.0f);
		CHECK(v2[4] == -5.0f);
	}

	TEST_CASE("TestVectorLength")
	{
		Vector<5> v;
		v[0] = 1;
		v[1] = 2;
		v[2] = 3;
		v[3] = 4;
		v[4] = 5;
		CHECK(v.LengthSq() == float(1 + 4 + 9 + 16 + 25));
	}
}
