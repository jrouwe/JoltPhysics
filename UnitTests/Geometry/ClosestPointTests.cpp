// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include <Jolt/Geometry/ClosestPoint.h>

TEST_SUITE("ClosestPointTests")
{
	TEST_CASE("TestNearColinearTriangle")
	{
		// A very long triangle that is nearly colinear
		Vec3 a(99.9999847f, 0.946687222f, 99.9999847f);
		Vec3 b(-100.010002f, 0.977360725f, -100.010002f);
		Vec3 c(-100.000137f, 0.977310658f, -100.000137f);

		Vec3 expected_closest(6.86645508e-5f, 0.961998940f, 6.86645508e-5f);

		uint32 set;
		Vec3 p = ClosestPoint::GetClosestPointOnTriangle(a, b, c, set);
		CHECK(set == 0b0101);
		CHECK_APPROX_EQUAL(p, expected_closest, 2.0e-5f);

		p = ClosestPoint::GetClosestPointOnTriangle(a, c, b, set);
		CHECK(set == 0b0011);
		CHECK_APPROX_EQUAL(p, expected_closest, 2.0e-5f);

		p = ClosestPoint::GetClosestPointOnTriangle(b, a, c, set);
		CHECK(set == 0b0110);
		CHECK_APPROX_EQUAL(p, expected_closest, 2.0e-5f);

		p = ClosestPoint::GetClosestPointOnTriangle(b, c, a, set);
		CHECK(set == 0b0110);
		CHECK_APPROX_EQUAL(p, expected_closest, 2.0e-5f);

		p = ClosestPoint::GetClosestPointOnTriangle(c, a, b, set);
		CHECK(set == 0b0011);
		CHECK_APPROX_EQUAL(p, expected_closest, 2.0e-5f);

		p = ClosestPoint::GetClosestPointOnTriangle(c, b, a, set);
		CHECK(set == 0b0101);
		CHECK_APPROX_EQUAL(p, expected_closest, 2.0e-5f);
	}
}
