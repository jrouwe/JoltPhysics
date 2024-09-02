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

		TaperedCylinderShapeSettings settings1(0.5f * cHeight, cRadius, 0.0f, 0.0f);
		settings1.SetDensity(cDensity);

		TaperedCylinderShapeSettings settings2(0.5f * cHeight, 0.0f, cRadius, 0.0f);
		settings2.SetDensity(cDensity);

		RefConst<TaperedCylinderShape> cylinder1 = StaticCast<TaperedCylinderShape>(settings1.Create().Get());
		RefConst<TaperedCylinderShape> cylinder2 = StaticCast<TaperedCylinderShape>(settings2.Create().Get());

		MassProperties m1 = cylinder1->GetMassProperties();
		MassProperties m2 = cylinder2->GetMassProperties();

		// Mass/inertia is the same for both shapes because they are mirrored versions (inertia is calculated from COM)
		CHECK_APPROX_EQUAL(m1.mMass, m2.mMass);
		CHECK_APPROX_EQUAL(m1.mInertia, m2.mInertia);

		// Center of mass for a cone is at 1/4 h (if cone runs from -h/2 to h/2)
		// See: https://www.miniphysics.com/uy1-centre-of-mass-of-a-cone.html
		Vec3 expected_com1(0, cHeight / 4.0f, 0);
		Vec3 expected_com2 = -expected_com1;
		CHECK_APPROX_EQUAL(cylinder1->GetCenterOfMass(), expected_com1);
		CHECK_APPROX_EQUAL(cylinder2->GetCenterOfMass(), expected_com2);

		// Mass of cone
		float expected_mass = cDensity * JPH_PI * Square(cRadius) * cHeight / 3.0f;
		CHECK_APPROX_EQUAL(expected_mass, m1.mMass);

		// Inertia of cone (according to https://en.wikipedia.org/wiki/List_of_moments_of_inertia)
		float expected_inertia_xx = expected_mass * (3.0f / 20.0f * Square(cRadius) + 3.0f / 80.0f * Square(cHeight));
		float expected_inertia_yy = expected_mass * (3.0f / 10.0f * Square(cRadius));
		CHECK_APPROX_EQUAL(expected_inertia_xx, m1.mInertia(0, 0), 1.0e-3f);
		CHECK_APPROX_EQUAL(expected_inertia_yy, m1.mInertia(1, 1), 1.0e-3f);
		CHECK_APPROX_EQUAL(expected_inertia_xx, m1.mInertia(2, 2), 1.0e-3f);
	}
}
