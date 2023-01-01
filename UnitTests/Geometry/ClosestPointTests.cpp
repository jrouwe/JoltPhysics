// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include <Jolt/Geometry/ClosestPoint.h>

TEST_SUITE("ClosestPointTests")
{
	TEST_CASE("TestNearDegenerateTriangle")
	{
		// A very long triangle that is nearly colinear
		Vec3 a(99.9999847f, 0.946687222f, 99.9999847f);
		Vec3 b(-100.010002f, 0.977360725f, -100.010002f);
		Vec3 c(-100.000137f, 0.977310658f, -100.000137f);

		uint32 set;
		Vec3 p = ClosestPoint::GetClosestPointOnTriangle(a, b, c, set);

		CHECK(set == 0x3);
		CHECK_APPROX_EQUAL(p, Vec3(7.62939453e-05f, 0.962023199f, 7.62939453e-05f));
	}
}
