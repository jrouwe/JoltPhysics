// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include "PhysicsTestContext.h"
#include <Jolt/Physics/Collision/Shape/TaperedCylinderShape.h>

TEST_SUITE("TaperedCylinderShapeTests")
{
	TEST_CASE("TestMassAndInertia")
	{
		const float cDensity = 3.0f;
		const float cRadius = 5.0f;
		const float cHeight = 7.0f;

		TaperedCylinderShapeSettings settings1(0.5f * cHeight, 0.0f, cRadius, 0.0f);
		settings1.SetDensity(cDensity);

		TaperedCylinderShapeSettings settings2(0.5f * cHeight, cRadius, 0.0f, 0.0f);
		settings2.SetDensity(cDensity);

		RefConst<Shape> cylinder1 = StaticCast<TaperedCylinderShape>(settings1.Create().Get());
		RefConst<Shape> cylinder2 = StaticCast<TaperedCylinderShape>(settings2.Create().Get());

		MassProperties m1 = cylinder1->GetMassProperties();
		MassProperties m2 = cylinder2->GetMassProperties();

		// Inertia is the same for both shapes because they are mirrored versions
		CHECK_APPROX_EQUAL(m1.mMass, m2.mMass);
		CHECK_APPROX_EQUAL(m1.mInertia, m2.mInertia);

		// Mass of cone
		float expected_mass = cDensity * JPH_PI * Square(cRadius) * cHeight / 3.0f;
		CHECK_APPROX_EQUAL(expected_mass, m1.mMass);

		// Inertia of cone (according to https://en.wikipedia.org/wiki/List_of_moments_of_inertia)
		float expected_inertia_xx = expected_mass * (3.0f / 20.0f * Square(cRadius) + 3.0f / 80.0f * Square(cHeight));
		float expected_inertia_yy = expected_mass * (3.0f / 10.0f * Square(cRadius));
		CHECK_APPROX_EQUAL(expected_inertia_xx, m1.mInertia(0, 0));
		CHECK_APPROX_EQUAL(expected_inertia_yy, m1.mInertia(1, 1));
		CHECK_APPROX_EQUAL(expected_inertia_xx, m1.mInertia(2, 2));
	}
}
