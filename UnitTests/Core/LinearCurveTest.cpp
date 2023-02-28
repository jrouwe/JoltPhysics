// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include <Jolt/Core/LinearCurve.h>

TEST_SUITE("LinearCurveTest")
{
	TEST_CASE("Test0PointCurve")
	{
		LinearCurve curve;

		CHECK(curve.GetValue(1.0f) == 0.0f);
	}

	TEST_CASE("Test1PointCurve")
	{
		LinearCurve curve;
		curve.AddPoint(1.0f, 20.0f);

		CHECK(curve.GetValue(0.9f) == 20.0f);
		CHECK(curve.GetValue(1.0f) == 20.0f);
		CHECK(curve.GetValue(1.1f) == 20.0f);
	}

	TEST_CASE("Test2PointCurve")
	{
		LinearCurve curve;
		curve.AddPoint(-1.0f, 40.0f);
		curve.AddPoint(-3.0f, 20.0f);
		curve.Sort();

		CHECK_APPROX_EQUAL(curve.GetValue(-3.1f), 20.0f);
		CHECK_APPROX_EQUAL(curve.GetValue(-3.0f), 20.0f);
		CHECK_APPROX_EQUAL(curve.GetValue(-2.0f), 30.0f);
		CHECK_APPROX_EQUAL(curve.GetValue(-1.0f), 40.0f);
		CHECK_APPROX_EQUAL(curve.GetValue(-0.9f), 40.0f);
	}

	TEST_CASE("Test3PointCurve")
	{
		LinearCurve curve;
		curve.AddPoint(1.0f, 20.0f);
		curve.AddPoint(5.0f, 60.0f);
		curve.AddPoint(3.0f, 40.0f);
		curve.Sort();

		CHECK_APPROX_EQUAL(curve.GetValue(0.9f), 20.0f);
		CHECK_APPROX_EQUAL(curve.GetValue(1.0f), 20.0f);
		CHECK_APPROX_EQUAL(curve.GetValue(2.0f), 30.0f);
		CHECK_APPROX_EQUAL(curve.GetValue(3.0f), 40.0f);
		CHECK_APPROX_EQUAL(curve.GetValue(4.0f), 50.0f);
		CHECK_APPROX_EQUAL(curve.GetValue(5.0f), 60.0f);
		CHECK_APPROX_EQUAL(curve.GetValue(5.1f), 60.0f);
	}
}
