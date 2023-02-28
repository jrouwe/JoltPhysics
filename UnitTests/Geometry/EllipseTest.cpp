// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include <Jolt/Geometry/Ellipse.h>

TEST_SUITE("EllipseTests")
{
	TEST_CASE("TestEllipseIsInside")
	{
		Ellipse e(1.0f, 2.0f);

		CHECK(e.IsInside(Float2(0.1f, 0.1f)));

		CHECK_FALSE(e.IsInside(Float2(2.0f, 0.0f)));
	}

	TEST_CASE("TestEllipseClosestPoint")
	{
		Ellipse e(1.0f, 2.0f);

		Float2 c = e.GetClosestPoint(Float2(2.0f, 0.0f));
		CHECK(c == Float2(1.0f, 0.0f));

		c = e.GetClosestPoint(Float2(-2.0f, 0.0f));
		CHECK(c == Float2(-1.0f, 0.0f));

		c = e.GetClosestPoint(Float2(0.0f, 4.0f));
		CHECK(c == Float2(0.0f, 2.0f));

		c = e.GetClosestPoint(Float2(0.0f, -4.0f));
		CHECK(c == Float2(0.0f, -2.0f));

		Ellipse e2(2.0f, 2.0f);

		c = e2.GetClosestPoint(Float2(4.0f, 4.0f));
		CHECK_APPROX_EQUAL(c, Float2(sqrt(2.0f), sqrt(2.0f)));
	}
}
