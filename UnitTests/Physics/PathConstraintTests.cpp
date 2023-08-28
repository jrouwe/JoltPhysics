// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include "PhysicsTestContext.h"
#include <Jolt/Physics/Constraints/PathConstraintPathHermite.h>
#include <Jolt/Physics/Constraints/PathConstraintPath.h>
#include "Layers.h"

TEST_SUITE("PathConstraintTests")
{
	// Test a straight line using a hermite spline.
	TEST_CASE("TestPathConstraintPathHermite")
	{
		// A straight spline
		// This has e.g. for t = 0.1 a local minimum at 0.7 which breaks the Newton Raphson root finding if not doing the bisection algorithm first.
		Vec3 p1 = Vec3(1424.96313f, 468.565399f, 483.655975f);
		Vec3 t1 = Vec3(61.4222832f, 42.8926392f, -1.70530257e-13f);
		Vec3 n1 = Vec3(0, 0, 1);
		Vec3 p2 = Vec3(1445.20105f, 482.364319f, 483.655975f);
		Vec3 t2 = Vec3(20.2380009f, 13.7989082f, -5.68434189e-14f);
		Vec3 n2 = Vec3(0, 0, 1);

		// Construct path
		Ref<PathConstraintPathHermite> path = new PathConstraintPathHermite;
		path->AddPoint(p1, t1, n1);
		path->AddPoint(p2, t2, n2);

		// Test that positions before and after the line return 0 and 1
		float before_start = path->GetClosestPoint(p1 - 0.01f * t1);
		CHECK(before_start == 0.0f);
		float after_end = path->GetClosestPoint(p2 + 0.01f * t2);
		CHECK(after_end == 1.0f);

		for (int i = 0; i <= 10; ++i)
		{
			// Get point on the curve
			float fraction = 0.1f * i;
			Vec3 pos, tgt, nrm, bin;
			path->GetPointOnPath(fraction, pos, tgt, nrm, bin);

			// Let the path determine the fraction of the closest point
			float closest_fraction = path->GetClosestPoint(pos);

			// Validate that it is equal to what we put in
			CHECK_APPROX_EQUAL(fraction, closest_fraction, 1.0e-4f);
		}
	}
}
