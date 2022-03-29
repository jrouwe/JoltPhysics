// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include <Jolt/Geometry/Plane.h>

TEST_SUITE("PlaneTests")
{
	TEST_CASE("TestPlaneSignedDistance")
	{
		Plane p = Plane::sFromPointAndNormal(Vec3(0, 2, 0), Vec3(0, 1, 0));
		CHECK(p.SignedDistance(Vec3(5, 7, 0)) == 5.0f);
		CHECK(p.SignedDistance(Vec3(5, -3, 0)) == -5.0f);
	}

	TEST_CASE("TestPlaneIntersectPlanes")
	{
		Plane p1 = Plane::sFromPointAndNormal(Vec3(0, 2, 0), Vec3(0, 1, 0));
		Plane p2 = Plane::sFromPointAndNormal(Vec3(3, 0, 0), Vec3(1, 0, 0));
		Plane p3 = Plane::sFromPointAndNormal(Vec3(0, 0, 4), Vec3(0, 0, 1));

		{
			Vec3 point;
			bool found = Plane::sIntersectPlanes(p1, p2, p3, point);
			CHECK(found);
			CHECK(point == Vec3(3, 2, 4));
		}

		{
			Plane p4 = Plane::sFromPointAndNormal(Vec3(0, 3, 0), Vec3(0, 1, 0));
			Vec3 point;
			bool found = Plane::sIntersectPlanes(p1, p2, p4, point);
			CHECK_FALSE(found);
		}
	}
}
