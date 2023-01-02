// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include <Jolt/Geometry/ClosestPoint.h>

TEST_SUITE("ClosestPointTests")
{
	// Test closest point from inPoint to triangle (inA, inB, inC)
	inline static void TestClosestPointToTriangle(Vec3Arg inA, Vec3Arg inB, Vec3Arg inC, Vec3Arg inPoint, Vec3Arg inExpectedClosestPoint, uint32 inExpectedSet)
	{
		// Make triangle relative to inPoint so we can get the closest poin to the origin
		Vec3 a = inA - inPoint;
		Vec3 b = inB - inPoint;
		Vec3 c = inC - inPoint;

		// Extract bits for A, B and C
		uint32 expected_a = inExpectedSet & 1;
		uint32 expected_b = (inExpectedSet & 2) >> 1;
		uint32 expected_c = (inExpectedSet & 4) >> 2;

		// Test all permutations of ABC
		uint32 set = 0;
		Vec3 closest_point = inPoint + ClosestPoint::GetClosestPointOnTriangle(a, b, c, set);
		CHECK(set == inExpectedSet);
		CHECK_APPROX_EQUAL(closest_point, inExpectedClosestPoint, 2.0e-5f);

		closest_point = inPoint + ClosestPoint::GetClosestPointOnTriangle(a, c, b, set);
		CHECK(set == ((expected_b << 2) | (expected_c << 1) | expected_a));
		CHECK_APPROX_EQUAL(closest_point, inExpectedClosestPoint, 2.0e-5f);

		closest_point = inPoint + ClosestPoint::GetClosestPointOnTriangle(b, a, c, set);
		CHECK(set == ((expected_c << 2) | (expected_a << 1) | expected_b));
		CHECK_APPROX_EQUAL(closest_point, inExpectedClosestPoint, 2.0e-5f);

		closest_point = inPoint + ClosestPoint::GetClosestPointOnTriangle(b, c, a, set);
		CHECK(set == ((expected_a << 2) | (expected_c << 1) | expected_b));
		CHECK_APPROX_EQUAL(closest_point, inExpectedClosestPoint, 2.0e-5f);

		closest_point = inPoint + ClosestPoint::GetClosestPointOnTriangle(c, a, b, set);
		CHECK(set == ((expected_b << 2) | (expected_a << 1) | expected_c));
		CHECK_APPROX_EQUAL(closest_point, inExpectedClosestPoint, 2.0e-5f);

		closest_point = inPoint + ClosestPoint::GetClosestPointOnTriangle(c, b, a, set);
		CHECK(set == ((expected_a << 2) | (expected_b << 1) | expected_c));
		CHECK_APPROX_EQUAL(closest_point, inExpectedClosestPoint, 2.0e-5f);
	}

	TEST_CASE("TestLongTriangle")
	{
		Vec3 a(100, 1, 0);
		Vec3 b(100, 1, 1);
		Vec3 c(-100, 1, 0);

		// Test interior
		TestClosestPointToTriangle(a, b, c, Vec3(0, 0, 0.1f), Vec3(0, 1, 0.1f), 0b0111);

		// Edge AB
		TestClosestPointToTriangle(a, b, c, Vec3(101, 0, 0.5f), Vec3(100, 1, 0.5f), 0b0011);

		// Edge AC
		TestClosestPointToTriangle(a, b, c, Vec3(0, 0, -0.1f), Vec3(0, 1, 0), 0b0101);

		// Edge BC
		Vec3 point_bc(0, 0, 1);
		Vec3 bc = c - b;
		Vec3 closest_bc = b + ((point_bc - b).Dot(bc) / bc.LengthSq()) * bc;
		TestClosestPointToTriangle(a, b, c, point_bc, closest_bc, 0b0110);

		// Vertex A
		TestClosestPointToTriangle(a, b, c, Vec3(101, 0, -1), a, 0b0001);

		// Vertex B
		TestClosestPointToTriangle(a, b, c, Vec3(101, 0, 2), b, 0b0010);

		// Vertex C
		TestClosestPointToTriangle(a, b, c, Vec3(-101, 0, 0), c, 0b0100);
	}
	
	TEST_CASE("TestNearColinearTriangle")
	{
		// A very long triangle that is nearly colinear
		Vec3 a(99.9999847f, 0.946687222f, 99.9999847f);
		Vec3 b(-100.010002f, 0.977360725f, -100.010002f);
		Vec3 c(-100.000137f, 0.977310658f, -100.000137f);

		// Closest point is on edge AC
		Vec3 ac = c - a;
		Vec3 expected_closest = a + (-a.Dot(ac) / ac.LengthSq()) * ac;

		TestClosestPointToTriangle(a, b, c, Vec3::sZero(), expected_closest, 0b0101);
	}
}
