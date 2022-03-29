// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include <Jolt/Geometry/AABox.h>
#include <Jolt/Geometry/RayAABox.h>

TEST_SUITE("RayAABoxTests")
{
	TEST_CASE("TestRayAABox")
	{
		AABox box(Vec3::sReplicate(-1.0f), Vec3::sReplicate(1.0f));
			
		for (int axis = 0; axis < 3; ++axis)
		{
			{
				// Ray starting in the center of the box, pointing high
				Vec3 origin = Vec3::sZero();
				Vec3 direction = Vec3::sZero();
				direction.SetComponent(axis, 1.0f);
				float fraction = RayAABox(origin, RayInvDirection(direction), box.mMin, box.mMax);
				CHECK_APPROX_EQUAL(-1.0f, fraction, 1.0e-6f);
			}

			{
				// Ray starting in the center of the box, pointing low
				Vec3 origin = Vec3::sZero();
				Vec3 direction = Vec3::sZero();
				direction.SetComponent(axis, -1.0f);
				float fraction = RayAABox(origin, RayInvDirection(direction), box.mMin, box.mMax);
				CHECK_APPROX_EQUAL(-1.0f, fraction, 1.0e-6f);
			}

			{
				// Ray starting high, pointing to low
				Vec3 origin = Vec3::sZero();
				origin.SetComponent(axis, 1.1f);
				Vec3 direction = Vec3::sZero();
				direction.SetComponent(axis, -1.0f);
				float fraction = RayAABox(origin, RayInvDirection(direction), box.mMin, box.mMax);
				CHECK_APPROX_EQUAL(0.1f, fraction, 1.0e-6f);
			}

			{
				// Ray starting high, pointing to high
				Vec3 origin = Vec3::sZero();
				origin.SetComponent(axis, 1.1f);
				Vec3 direction = Vec3::sZero();
				direction.SetComponent(axis, 1.0f);
				float fraction = RayAABox(origin, RayInvDirection(direction), box.mMin, box.mMax);
				CHECK(fraction == FLT_MAX);
			}

			{
				// Ray starting low, pointing to high
				Vec3 origin = Vec3::sZero();
				origin.SetComponent(axis, -1.1f);
				Vec3 direction = Vec3::sZero();
				direction.SetComponent(axis, 1.0f);
				float fraction = RayAABox(origin, RayInvDirection(direction), box.mMin, box.mMax);
				CHECK_APPROX_EQUAL(0.1f, fraction, 1.0e-6f);
			}

			{
				// Ray starting low, pointing to low
				Vec3 origin = Vec3::sZero();
				origin.SetComponent(axis, -1.1f);
				Vec3 direction = Vec3::sZero();
				direction.SetComponent(axis, -1.0f);
				float fraction = RayAABox(origin, RayInvDirection(direction), box.mMin, box.mMax);
				CHECK(fraction == FLT_MAX);
			}
		}

		{
			// Test ray that hits top plane under an angle
			Vec3 expected_hit = Vec3(0, 1, 0);
			float expected_fraction = 0.123f;
			Vec3 direction = Vec3(4, -4, 0);
			Vec3 origin = expected_hit - expected_fraction * direction;
			float fraction = RayAABox(origin, RayInvDirection(direction), box.mMin, box.mMax);
			CHECK_APPROX_EQUAL(expected_fraction, fraction, 1.0e-6f);
		}
	}
}
