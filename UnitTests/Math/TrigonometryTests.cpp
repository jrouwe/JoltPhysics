// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include <Jolt/Math/Trigonometry.h>

TEST_SUITE("TrigonometryTests")
{
	TEST_CASE("TestACosApproximate")
	{
		// Check error over entire range [-1, 1]
		for (int i = -1000; i <= 1000; i++)
		{
			float x = float(i) / 1000.0f;
			float acos1 = acos(x);
			float acos2 = ACosApproximate(x);
			CHECK_APPROX_EQUAL(acos1, acos2, 4.3e-3f);
		}

		// Check edge cases for exact matches
		CHECK(ACosApproximate(1.0f) == 0.0f);
		CHECK(ACosApproximate(1.0e-12f) == JPH_PI / 2);
		CHECK(ACosApproximate(-1.0e-12f) == JPH_PI / 2);
		CHECK(ACosApproximate(-1.0f) == JPH_PI);
	}
}
