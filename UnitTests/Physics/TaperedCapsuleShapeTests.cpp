// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include <Jolt/Physics/Collision/Shape/TaperedCapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>

TEST_SUITE("TaperedCapsuleShapeTests")
{
	TEST_CASE("TestConstruction")
	{
		{
			// Regular tapered capsule
			TaperedCapsuleShapeSettings tapered_settings(2.0f, 1.0f, 3.0f);
			ShapeSettings::ShapeResult result = tapered_settings.Create();
			CHECK(result.IsValid());
			RefConst<Shape> shape = result.Get();
			CHECK(shape->GetSubType() == EShapeSubType::TaperedCapsule);
			const TaperedCapsuleShape *tapered = StaticCast<TaperedCapsuleShape>(shape);
			CHECK(tapered->GetTopRadius() == 1.0f);
			CHECK(tapered->GetBottomRadius() == 3.0f);
			CHECK(tapered->GetHalfHeight() == 2.0f);
		}

		{
			// Equal radii turns into a capsule
			TaperedCapsuleShapeSettings capsule_settings(2.0f, 1.0f, 1.0f);
			ShapeSettings::ShapeResult result = capsule_settings.Create();
			CHECK(result.IsValid());
			CHECK(result.Get()->GetSubType() == EShapeSubType::Capsule);
			const CapsuleShape *capsule = StaticCast<CapsuleShape>(result.Get());
			CHECK(capsule->GetRadius() == 1.0f);
			CHECK(capsule->GetHalfHeightOfCylinder() == 2.0f);
		}

		{
			// One sphere containing the other turns into a sphere
			TaperedCapsuleShapeSettings sphere_settings(1.0f, 2.0f, 4.0f);
			ShapeSettings::ShapeResult result = sphere_settings.Create();
			CHECK(result.IsValid());
			CHECK(result.Get()->GetSubType() == EShapeSubType::RotatedTranslated);
			const RotatedTranslatedShape *rt_shape = StaticCast<RotatedTranslatedShape>(result.Get());
			CHECK(rt_shape->GetInnerShape()->GetSubType() == EShapeSubType::Sphere);
			CHECK(rt_shape->GetPosition() == Vec3(0, -1.0f, 0));
			const SphereShape *sphere = StaticCast<SphereShape>(rt_shape->GetInnerShape());
			CHECK(sphere->GetRadius() == 4.0f);
		}

		{
			// Sphere without offset
			TaperedCapsuleShapeSettings sphere_settings(0.0f, 4.0f, 2.0f);
			ShapeSettings::ShapeResult result = sphere_settings.Create();
			CHECK(result.IsValid());
			CHECK(result.Get()->GetSubType() == EShapeSubType::Sphere);
			const SphereShape *sphere = StaticCast<SphereShape>(result.Get());
			CHECK(sphere->GetRadius() == 4.0f);
		}
	}
}
