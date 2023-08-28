// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include "PhysicsTestContext.h"
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/TaperedCapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/CylinderShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>
#include <Jolt/Physics/Collision/Shape/MutableCompoundShape.h>
#include <Jolt/Physics/Collision/Shape/TriangleShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Jolt/Physics/Collision/Shape/HeightFieldShape.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Jolt/Physics/Collision/CollidePointResult.h>
#include <Jolt/Core/StreamWrapper.h>

TEST_SUITE("ShapeTests")
{
	// Test convex hull shape
	TEST_CASE("TestConvexHullShape")
	{
		const float cDensity = 1.5f;

		// Create convex hull shape of a box
		Array<Vec3> box;
		box.push_back(Vec3(5, 6, 7));
		box.push_back(Vec3(5, 6, 14));
		box.push_back(Vec3(5, 12, 7));
		box.push_back(Vec3(5, 12, 14));
		box.push_back(Vec3(10, 6, 7));
		box.push_back(Vec3(10, 6, 14));
		box.push_back(Vec3(10, 12, 7));
		box.push_back(Vec3(10, 12, 14));
		ConvexHullShapeSettings settings(box);
		settings.SetDensity(cDensity);
		RefConst<Shape> shape = settings.Create().Get();

		// Validate calculated center of mass
		Vec3 com = shape->GetCenterOfMass();
		CHECK_APPROX_EQUAL(Vec3(7.5f, 9.0f, 10.5f), com, 1.0e-5f);

		// Calculate reference value of mass and inertia of a box
		MassProperties reference;
		reference.SetMassAndInertiaOfSolidBox(Vec3(5, 6, 7), cDensity);

		// Mass is easy to calculate, double check if SetMassAndInertiaOfSolidBox calculated it correctly
		CHECK_APPROX_EQUAL(5.0f * 6.0f * 7.0f * cDensity, reference.mMass, 1.0e-6f);

		// Get calculated inertia tensor
		MassProperties m = shape->GetMassProperties();
		CHECK_APPROX_EQUAL(reference.mMass, m.mMass, 1.0e-6f);
		CHECK_APPROX_EQUAL(reference.mInertia, m.mInertia, 1.0e-4f);

		// Check inner radius
		CHECK_APPROX_EQUAL(shape->GetInnerRadius(), 2.5f);
	}

	// Test IsValidScale function
	TEST_CASE("TestIsValidScale")
	{
		// Test simple shapes
		Ref<Shape> sphere = new SphereShape(2.0f);
		CHECK(!sphere->IsValidScale(Vec3::sZero()));
		CHECK(sphere->IsValidScale(Vec3(2, 2, 2)));
		CHECK(sphere->IsValidScale(Vec3(-1, 1, -1)));
		CHECK(!sphere->IsValidScale(Vec3(2, 1, 1)));
		CHECK(!sphere->IsValidScale(Vec3(1, 2, 1)));
		CHECK(!sphere->IsValidScale(Vec3(1, 1, 2)));

		Ref<Shape> capsule = new CapsuleShape(2.0f, 0.5f);
		CHECK(!capsule->IsValidScale(Vec3::sZero()));
		CHECK(capsule->IsValidScale(Vec3(2, 2, 2)));
		CHECK(capsule->IsValidScale(Vec3(-1, 1, -1)));
		CHECK(!capsule->IsValidScale(Vec3(2, 1, 1)));
		CHECK(!capsule->IsValidScale(Vec3(1, 2, 1)));
		CHECK(!capsule->IsValidScale(Vec3(1, 1, 2)));

		Ref<Shape> tapered_capsule = TaperedCapsuleShapeSettings(2.0f, 0.5f, 0.7f).Create().Get();
		CHECK(!tapered_capsule->IsValidScale(Vec3::sZero()));
		CHECK(tapered_capsule->IsValidScale(Vec3(2, 2, 2)));
		CHECK(tapered_capsule->IsValidScale(Vec3(-1, 1, -1)));
		CHECK(!tapered_capsule->IsValidScale(Vec3(2, 1, 1)));
		CHECK(!tapered_capsule->IsValidScale(Vec3(1, 2, 1)));
		CHECK(!tapered_capsule->IsValidScale(Vec3(1, 1, 2)));

		Ref<Shape> cylinder = new CylinderShape(0.5f, 2.0f);
		CHECK(!cylinder->IsValidScale(Vec3::sZero()));
		CHECK(cylinder->IsValidScale(Vec3(2, 2, 2)));
		CHECK(cylinder->IsValidScale(Vec3(-1, 1, -1)));
		CHECK(!cylinder->IsValidScale(Vec3(2, 1, 1)));
		CHECK(cylinder->IsValidScale(Vec3(1, 2, 1)));
		CHECK(!cylinder->IsValidScale(Vec3(1, 1, 2)));

		Ref<Shape> triangle = new TriangleShape(Vec3(1, 2, 3), Vec3(4, 5, 6), Vec3(7, 8, 9));
		CHECK(!triangle->IsValidScale(Vec3::sZero()));
		CHECK(triangle->IsValidScale(Vec3(2, 2, 2)));
		CHECK(triangle->IsValidScale(Vec3(-1, 1, -1)));
		CHECK(triangle->IsValidScale(Vec3(2, 1, 1)));
		CHECK(triangle->IsValidScale(Vec3(1, 2, 1)));
		CHECK(triangle->IsValidScale(Vec3(1, 1, 2)));

		Ref<Shape> triangle2 = new TriangleShape(Vec3(1, 2, 3), Vec3(4, 5, 6), Vec3(7, 8, 9), 0.01f); // With convex radius
		CHECK(!triangle2->IsValidScale(Vec3::sZero()));
		CHECK(triangle2->IsValidScale(Vec3(2, 2, 2)));
		CHECK(triangle2->IsValidScale(Vec3(-1, 1, -1)));
		CHECK(!triangle2->IsValidScale(Vec3(2, 1, 1)));
		CHECK(!triangle2->IsValidScale(Vec3(1, 2, 1)));
		CHECK(!triangle2->IsValidScale(Vec3(1, 1, 2)));

		Ref<Shape> scaled = new ScaledShape(sphere, Vec3(1, 2, 1));
		CHECK(!scaled->IsValidScale(Vec3::sZero()));
		CHECK(!scaled->IsValidScale(Vec3(1, 1, 1)));
		CHECK(scaled->IsValidScale(Vec3(1, 0.5f, 1)));
		CHECK(scaled->IsValidScale(Vec3(-1, 0.5f, 1)));
		CHECK(!scaled->IsValidScale(Vec3(2, 1, 1)));
		CHECK(!scaled->IsValidScale(Vec3(1, 2, 1)));
		CHECK(!scaled->IsValidScale(Vec3(1, 1, 2)));

		Ref<Shape> scaled2 = new ScaledShape(scaled, Vec3(1, 0.5f, 1));
		CHECK(!scaled2->IsValidScale(Vec3::sZero()));
		CHECK(scaled2->IsValidScale(Vec3(2, 2, 2)));
		CHECK(scaled2->IsValidScale(Vec3(-1, 1, -1)));
		CHECK(!scaled2->IsValidScale(Vec3(2, 1, 1)));
		CHECK(!scaled2->IsValidScale(Vec3(1, 2, 1)));
		CHECK(!scaled2->IsValidScale(Vec3(1, 1, 2)));

		// Test a compound with shapes that can only be scaled uniformly
		StaticCompoundShapeSettings compound_settings;
		compound_settings.AddShape(Vec3(1, 2, 3), Quat::sRotation(Vec3::sAxisX(), 0.1f * JPH_PI), sphere);
		compound_settings.AddShape(Vec3(4, 5, 6), Quat::sRotation(Vec3::sAxisY(), 0.1f * JPH_PI), capsule);
		Ref<Shape> compound = compound_settings.Create().Get();
		CHECK(!compound->IsValidScale(Vec3::sZero()));
		CHECK(compound->IsValidScale(Vec3(1, 1, 1)));
		CHECK(compound->IsValidScale(Vec3(2, 2, 2)));
		CHECK(!compound->IsValidScale(Vec3(2, 1, 1)));
		CHECK(!compound->IsValidScale(Vec3(1, 2, 1)));
		CHECK(!compound->IsValidScale(Vec3(1, 1, 2)));

		// Test compound containing a triangle shape that can be scaled in any way
		StaticCompoundShapeSettings compound_settings2;
		compound_settings2.AddShape(Vec3(1, 2, 3), Quat::sIdentity(), triangle);
		compound_settings2.AddShape(Vec3(4, 5, 6), Quat::sIdentity(), new ScaledShape(triangle, Vec3(10, 11, 12)));
		Ref<Shape> compound2 = compound_settings2.Create().Get();
		CHECK(!compound2->IsValidScale(Vec3::sZero()));
		CHECK(compound2->IsValidScale(Vec3(1, 1, 1)));
		CHECK(compound2->IsValidScale(Vec3(2, 2, 2)));
		CHECK(compound2->IsValidScale(Vec3(2, 1, 1)));
		CHECK(compound2->IsValidScale(Vec3(1, 2, 1)));
		CHECK(compound2->IsValidScale(Vec3(1, 1, 2)));

		// Test rotations inside the compound of 90 degrees
		StaticCompoundShapeSettings compound_settings3;
		compound_settings3.AddShape(Vec3(1, 2, 3), Quat::sRotation(Vec3::sAxisZ(), -0.5f * JPH_PI), triangle);
		compound_settings3.AddShape(Vec3(4, 5, 6), Quat::sRotation(Vec3::sAxisZ(), 0.5f * JPH_PI), new ScaledShape(triangle, Vec3(10, 11, 12)));
		Ref<Shape> compound3 = compound_settings3.Create().Get();
		CHECK(!compound3->IsValidScale(Vec3::sZero()));
		CHECK(compound3->IsValidScale(Vec3(1, 1, 1)));
		CHECK(compound3->IsValidScale(Vec3(2, 2, 2)));
		CHECK(compound3->IsValidScale(Vec3(2, 1, 1)));
		CHECK(compound3->IsValidScale(Vec3(1, 2, 1)));
		CHECK(compound3->IsValidScale(Vec3(1, 1, 2)));

		// Test non-90 degree rotations, this would cause shearing so is not allowed (we can't express that by passing a diagonal scale vector)
		StaticCompoundShapeSettings compound_settings4;
		compound_settings4.AddShape(Vec3(1, 2, 3), Quat::sRotation(Vec3::sAxisZ(), 0.25f * JPH_PI), triangle);
		compound_settings4.AddShape(Vec3(1, 2, 3), Quat::sRotation(Vec3::sAxisZ(), -0.25f * JPH_PI), triangle);
		Ref<Shape> compound4 = compound_settings4.Create().Get();
		CHECK(!compound4->IsValidScale(Vec3::sZero()));
		CHECK(compound4->IsValidScale(Vec3(1, 1, 1)));
		CHECK(compound4->IsValidScale(Vec3(2, 2, 2)));
		CHECK(!compound4->IsValidScale(Vec3(2, 1, 1)));
		CHECK(!compound4->IsValidScale(Vec3(1, 2, 1)));
		CHECK(compound4->IsValidScale(Vec3(1, 1, 2))); // We're rotation around Z, so non-uniform in the Z direction is ok

		// Test a mutable compound with shapes that can only be scaled uniformly
		MutableCompoundShapeSettings mutable_compound_settings;
		mutable_compound_settings.AddShape(Vec3(1, 2, 3), Quat::sRotation(Vec3::sAxisX(), 0.1f * JPH_PI), sphere);
		mutable_compound_settings.AddShape(Vec3(4, 5, 6), Quat::sRotation(Vec3::sAxisY(), 0.1f * JPH_PI), capsule);
		Ref<Shape> mutable_compound = mutable_compound_settings.Create().Get();
		CHECK(!mutable_compound->IsValidScale(Vec3::sZero()));
		CHECK(mutable_compound->IsValidScale(Vec3(1, 1, 1)));
		CHECK(mutable_compound->IsValidScale(Vec3(2, 2, 2)));
		CHECK(!mutable_compound->IsValidScale(Vec3(2, 1, 1)));
		CHECK(!mutable_compound->IsValidScale(Vec3(1, 2, 1)));
		CHECK(!mutable_compound->IsValidScale(Vec3(1, 1, 2)));

		// Test mutable compound containing a triangle shape that can be scaled in any way
		MutableCompoundShapeSettings mutable_compound_settings2;
		mutable_compound_settings2.AddShape(Vec3(1, 2, 3), Quat::sIdentity(), triangle);
		mutable_compound_settings2.AddShape(Vec3(4, 5, 6), Quat::sIdentity(), new ScaledShape(triangle, Vec3(10, 11, 12)));
		Ref<Shape> mutable_compound2 = mutable_compound_settings2.Create().Get();
		CHECK(!mutable_compound2->IsValidScale(Vec3::sZero()));
		CHECK(mutable_compound2->IsValidScale(Vec3(1, 1, 1)));
		CHECK(mutable_compound2->IsValidScale(Vec3(2, 2, 2)));
		CHECK(mutable_compound2->IsValidScale(Vec3(2, 1, 1)));
		CHECK(mutable_compound2->IsValidScale(Vec3(1, 2, 1)));
		CHECK(mutable_compound2->IsValidScale(Vec3(1, 1, 2)));

		// Test rotations inside the mutable compound of 90 degrees
		MutableCompoundShapeSettings mutable_compound_settings3;
		mutable_compound_settings3.AddShape(Vec3(1, 2, 3), Quat::sRotation(Vec3::sAxisZ(), -0.5f * JPH_PI), triangle);
		mutable_compound_settings3.AddShape(Vec3(4, 5, 6), Quat::sRotation(Vec3::sAxisZ(), 0.5f * JPH_PI), new ScaledShape(triangle, Vec3(10, 11, 12)));
		Ref<Shape> mutable_compound3 = mutable_compound_settings3.Create().Get();
		CHECK(!mutable_compound3->IsValidScale(Vec3::sZero()));
		CHECK(mutable_compound3->IsValidScale(Vec3(1, 1, 1)));
		CHECK(mutable_compound3->IsValidScale(Vec3(2, 2, 2)));
		CHECK(mutable_compound3->IsValidScale(Vec3(2, 1, 1)));
		CHECK(mutable_compound3->IsValidScale(Vec3(1, 2, 1)));
		CHECK(mutable_compound3->IsValidScale(Vec3(1, 1, 2)));

		// Test non-90 degree rotations, this would cause shearing so is not allowed (we can't express that by passing a diagonal scale vector)
		MutableCompoundShapeSettings mutable_compound_settings4;
		mutable_compound_settings4.AddShape(Vec3(1, 2, 3), Quat::sRotation(Vec3::sAxisZ(), 0.25f * JPH_PI), triangle);
		mutable_compound_settings4.AddShape(Vec3(1, 2, 3), Quat::sRotation(Vec3::sAxisZ(), -0.25f * JPH_PI), triangle);
		Ref<Shape> mutable_compound4 = mutable_compound_settings4.Create().Get();
		CHECK(!mutable_compound4->IsValidScale(Vec3::sZero()));
		CHECK(mutable_compound4->IsValidScale(Vec3(1, 1, 1)));
		CHECK(mutable_compound4->IsValidScale(Vec3(2, 2, 2)));
		CHECK(!mutable_compound4->IsValidScale(Vec3(2, 1, 1)));
		CHECK(!mutable_compound4->IsValidScale(Vec3(1, 2, 1)));
		CHECK(mutable_compound4->IsValidScale(Vec3(1, 1, 2))); // We're rotation around Z, so non-uniform in the Z direction is ok

		// Test a rotated translated shape that can only be scaled uniformly
		RotatedTranslatedShapeSettings rt_settings(Vec3(1, 2, 3), Quat::sRotation(Vec3::sAxisX(), 0.1f * JPH_PI), sphere);
		Ref<Shape> rt_shape = rt_settings.Create().Get();
		CHECK(!rt_shape->IsValidScale(Vec3::sZero()));
		CHECK(rt_shape->IsValidScale(Vec3(1, 1, 1)));
		CHECK(rt_shape->IsValidScale(Vec3(2, 2, 2)));
		CHECK(!rt_shape->IsValidScale(Vec3(2, 1, 1)));
		CHECK(!rt_shape->IsValidScale(Vec3(1, 2, 1)));
		CHECK(!rt_shape->IsValidScale(Vec3(1, 1, 2)));

		// Test rotated translated shape containing a triangle shape that can be scaled in any way
		RotatedTranslatedShapeSettings rt_settings2(Vec3(4, 5, 6), Quat::sIdentity(), new ScaledShape(triangle, Vec3(10, 11, 12)));
		Ref<Shape> rt_shape2 = rt_settings2.Create().Get();
		CHECK(!rt_shape2->IsValidScale(Vec3::sZero()));
		CHECK(rt_shape2->IsValidScale(Vec3(1, 1, 1)));
		CHECK(rt_shape2->IsValidScale(Vec3(2, 2, 2)));
		CHECK(rt_shape2->IsValidScale(Vec3(2, 1, 1)));
		CHECK(rt_shape2->IsValidScale(Vec3(1, 2, 1)));
		CHECK(rt_shape2->IsValidScale(Vec3(1, 1, 2)));

		// Test rotations inside the rotated translated of 90 degrees
		RotatedTranslatedShapeSettings rt_settings3(Vec3(1, 2, 3), Quat::sRotation(Vec3::sAxisZ(), -0.5f * JPH_PI), triangle);
		Ref<Shape> rt_shape3 = rt_settings3.Create().Get();
		CHECK(!rt_shape3->IsValidScale(Vec3::sZero()));
		CHECK(rt_shape3->IsValidScale(Vec3(1, 1, 1)));
		CHECK(rt_shape3->IsValidScale(Vec3(2, 2, 2)));
		CHECK(rt_shape3->IsValidScale(Vec3(2, 1, 1)));
		CHECK(rt_shape3->IsValidScale(Vec3(1, 2, 1)));
		CHECK(rt_shape3->IsValidScale(Vec3(1, 1, 2)));

		// Test non-90 degree rotations, this would cause shearing so is not allowed (we can't express that by passing a diagonal scale vector)
		RotatedTranslatedShapeSettings rt_settings4(Vec3(1, 2, 3), Quat::sRotation(Vec3::sAxisZ(), 0.25f * JPH_PI), triangle);
		Ref<Shape> rt_shape4 = rt_settings4.Create().Get();
		CHECK(!rt_shape4->IsValidScale(Vec3::sZero()));
		CHECK(rt_shape4->IsValidScale(Vec3(1, 1, 1)));
		CHECK(rt_shape4->IsValidScale(Vec3(2, 2, 2)));
		CHECK(!rt_shape4->IsValidScale(Vec3(2, 1, 1)));
		CHECK(!rt_shape4->IsValidScale(Vec3(1, 2, 1)));
		CHECK(rt_shape4->IsValidScale(Vec3(1, 1, 2))); // We're rotation around Z, so non-uniform in the Z direction is ok
	}

	// Test embedded shape
	TEST_CASE("TestEmbeddedShape")
	{
		{
			// Test shape constructed on stack, where shape construction succeeds
			ConvexHullShapeSettings settings;
			settings.mPoints.push_back(Vec3(0, 0, 0));
			settings.mPoints.push_back(Vec3(1, 0, 0));
			settings.mPoints.push_back(Vec3(0, 1, 0));
			settings.mPoints.push_back(Vec3(0, 0, 1));
			Shape::ShapeResult result;
			ConvexHullShape shape(settings, result);
			shape.SetEmbedded();
			CHECK(result.IsValid());
			result.Clear(); // Release the reference from the result

			// Test CollidePoint for this shape
			AllHitCollisionCollector<CollidePointCollector> collector;
			shape.CollidePoint(Vec3::sReplicate(-0.1f) - shape.GetCenterOfMass(), SubShapeIDCreator(), collector);
			CHECK(collector.mHits.empty());
			shape.CollidePoint(Vec3::sReplicate(0.1f) - shape.GetCenterOfMass(), SubShapeIDCreator(), collector);
			CHECK(collector.mHits.size() == 1);
		}

		{
			// Test shape constructed on stack, where shape construction fails
			ConvexHullShapeSettings settings;
			Shape::ShapeResult result;
			ConvexHullShape shape(settings, result);
			shape.SetEmbedded();
			CHECK(!result.IsValid());
		}
	}

	// Test submerged volume calculation
	TEST_CASE("TestGetSubmergedVolume")
	{
		Ref<BoxShape> box = new BoxShape(Vec3(1, 2, 3));
		Vec3 scale(2, -3, 4);
		Mat44 translation = Mat44::sTranslation(Vec3(0, 6, 0)); // Translate so we're on the y = 0 plane

		// Plane pointing positive Y
		// Entirely above the plane
		{
			float total_volume, submerged_volume;
			Vec3 center_of_buoyancy;
			box->GetSubmergedVolume(translation, scale, Plane::sFromPointAndNormal(Vec3(0, -0.001f, 0), Vec3::sAxisY()), total_volume, submerged_volume, center_of_buoyancy JPH_IF_DEBUG_RENDERER(, RVec3::sZero()));
			CHECK_APPROX_EQUAL(total_volume, 4.0f * 12.0f * 24.0f);
			CHECK_APPROX_EQUAL(submerged_volume, 0.0f);
		}

		// Entirely below the plane
		{
			float total_volume, submerged_volume;
			Vec3 center_of_buoyancy;
			box->GetSubmergedVolume(translation, scale, Plane::sFromPointAndNormal(Vec3(0, 12.001f, 0), Vec3::sAxisY()), total_volume, submerged_volume, center_of_buoyancy JPH_IF_DEBUG_RENDERER(, RVec3::sZero()));
			CHECK_APPROX_EQUAL(total_volume, 4.0f * 12.0f * 24.0f);
			CHECK_APPROX_EQUAL(submerged_volume, 4.0f * 12.0f * 24.0f);
			CHECK_APPROX_EQUAL(center_of_buoyancy, Vec3(0, 6, 0));
		}

		// Halfway through
		{
			float total_volume, submerged_volume;
			Vec3 center_of_buoyancy;
			box->GetSubmergedVolume(translation, scale, Plane::sFromPointAndNormal(Vec3(0, 6.0f, 0), Vec3::sAxisY()), total_volume, submerged_volume, center_of_buoyancy JPH_IF_DEBUG_RENDERER(, RVec3::sZero()));
			CHECK_APPROX_EQUAL(total_volume, 4.0f * 12.0f * 24.0f);
			CHECK_APPROX_EQUAL(submerged_volume, 4.0f * 6.0f * 24.0f);
			CHECK_APPROX_EQUAL(center_of_buoyancy, Vec3(0, 3, 0));
		}

		// Plane pointing negative Y
		// Entirely above the plane
		{
			float total_volume, submerged_volume;
			Vec3 center_of_buoyancy;
			box->GetSubmergedVolume(translation, scale, Plane::sFromPointAndNormal(Vec3(-4, 12.001f, 0), -Vec3::sAxisY()), total_volume, submerged_volume, center_of_buoyancy JPH_IF_DEBUG_RENDERER(, RVec3::sZero()));
			CHECK_APPROX_EQUAL(total_volume, 4.0f * 12.0f * 24.0f);
			CHECK_APPROX_EQUAL(submerged_volume, 0.0f);
		}

		// Entirely below the plane
		{
			float total_volume, submerged_volume;
			Vec3 center_of_buoyancy;
			box->GetSubmergedVolume(translation, scale, Plane::sFromPointAndNormal(Vec3(0, -0.001f, 0), -Vec3::sAxisY()), total_volume, submerged_volume, center_of_buoyancy JPH_IF_DEBUG_RENDERER(, RVec3::sZero()));
			CHECK_APPROX_EQUAL(total_volume, 4.0f * 12.0f * 24.0f);
			CHECK_APPROX_EQUAL(submerged_volume, 4.0f * 12.0f * 24.0f);
			CHECK_APPROX_EQUAL(center_of_buoyancy, Vec3(0, 6, 0));
		}

		// Halfway through
		{
			float total_volume, submerged_volume;
			Vec3 center_of_buoyancy;
			box->GetSubmergedVolume(translation, scale, Plane::sFromPointAndNormal(Vec3(0, 6.0f, 0), -Vec3::sAxisY()), total_volume, submerged_volume, center_of_buoyancy JPH_IF_DEBUG_RENDERER(, RVec3::sZero()));
			CHECK_APPROX_EQUAL(total_volume, 4.0f * 12.0f * 24.0f);
			CHECK_APPROX_EQUAL(submerged_volume, 4.0f * 6.0f * 24.0f);
			CHECK_APPROX_EQUAL(center_of_buoyancy, Vec3(0, 9, 0));
		}

		// Plane pointing positive X
		// Entirely above the plane
		{
			float total_volume, submerged_volume;
			Vec3 center_of_buoyancy;
			box->GetSubmergedVolume(translation, scale, Plane::sFromPointAndNormal(Vec3(-2.001f, 0, 0), Vec3::sAxisX()), total_volume, submerged_volume, center_of_buoyancy JPH_IF_DEBUG_RENDERER(, RVec3::sZero()));
			CHECK_APPROX_EQUAL(total_volume, 4.0f * 12.0f * 24.0f);
			CHECK_APPROX_EQUAL(submerged_volume, 0.0f);
		}

		// Entirely below the plane
		{
			float total_volume, submerged_volume;
			Vec3 center_of_buoyancy;
			box->GetSubmergedVolume(translation, scale, Plane::sFromPointAndNormal(Vec3(2.001f, 0, 0), Vec3::sAxisX()), total_volume, submerged_volume, center_of_buoyancy JPH_IF_DEBUG_RENDERER(, RVec3::sZero()));
			CHECK_APPROX_EQUAL(total_volume, 4.0f * 12.0f * 24.0f);
			CHECK_APPROX_EQUAL(submerged_volume, 4.0f * 12.0f * 24.0f);
			CHECK_APPROX_EQUAL(center_of_buoyancy, Vec3(0, 6, 0));
		}

		// Halfway through
		{
			float total_volume, submerged_volume;
			Vec3 center_of_buoyancy;
			box->GetSubmergedVolume(translation, scale, Plane::sFromPointAndNormal(Vec3(0, 0, 0), Vec3::sAxisX()), total_volume, submerged_volume, center_of_buoyancy JPH_IF_DEBUG_RENDERER(, RVec3::sZero()));
			CHECK_APPROX_EQUAL(total_volume, 4.0f * 12.0f * 24.0f);
			CHECK_APPROX_EQUAL(submerged_volume, 2.0f * 12.0f * 24.0f);
			CHECK_APPROX_EQUAL(center_of_buoyancy, Vec3(-1, 6, 0));
		}

		// Plane pointing negative X
		// Entirely above the plane
		{
			float total_volume, submerged_volume;
			Vec3 center_of_buoyancy;
			box->GetSubmergedVolume(translation, scale, Plane::sFromPointAndNormal(Vec3(2.001f, 0, 0), -Vec3::sAxisX()), total_volume, submerged_volume, center_of_buoyancy JPH_IF_DEBUG_RENDERER(, RVec3::sZero()));
			CHECK_APPROX_EQUAL(total_volume, 4.0f * 12.0f * 24.0f);
			CHECK_APPROX_EQUAL(submerged_volume, 0.0f);
		}

		// Entirely below the plane
		{
			float total_volume, submerged_volume;
			Vec3 center_of_buoyancy;
			box->GetSubmergedVolume(translation, scale, Plane::sFromPointAndNormal(Vec3(-2.001f, 0, 0), -Vec3::sAxisX()), total_volume, submerged_volume, center_of_buoyancy JPH_IF_DEBUG_RENDERER(, RVec3::sZero()));
			CHECK_APPROX_EQUAL(total_volume, 4.0f * 12.0f * 24.0f);
			CHECK_APPROX_EQUAL(submerged_volume, 4.0f * 12.0f * 24.0f);
			CHECK_APPROX_EQUAL(center_of_buoyancy, Vec3(0, 6, 0));
		}

		// Halfway through
		{
			float total_volume, submerged_volume;
			Vec3 center_of_buoyancy;
			box->GetSubmergedVolume(translation, scale, Plane::sFromPointAndNormal(Vec3(0, 0, 0), -Vec3::sAxisX()), total_volume, submerged_volume, center_of_buoyancy JPH_IF_DEBUG_RENDERER(, RVec3::sZero()));
			CHECK_APPROX_EQUAL(total_volume, 4.0f * 12.0f * 24.0f);
			CHECK_APPROX_EQUAL(submerged_volume, 2.0f * 12.0f * 24.0f);
			CHECK_APPROX_EQUAL(center_of_buoyancy, Vec3(1, 6, 0));
		}

		// Plane pointing positive Z
		// Entirely above the plane
		{
			float total_volume, submerged_volume;
			Vec3 center_of_buoyancy;
			box->GetSubmergedVolume(translation, scale, Plane::sFromPointAndNormal(Vec3(0, 0, -12.001f), Vec3::sAxisZ()), total_volume, submerged_volume, center_of_buoyancy JPH_IF_DEBUG_RENDERER(, RVec3::sZero()));
			CHECK_APPROX_EQUAL(total_volume, 4.0f * 12.0f * 24.0f);
			CHECK_APPROX_EQUAL(submerged_volume, 0.0f);
		}

		// Entirely below the plane
		{
			float total_volume, submerged_volume;
			Vec3 center_of_buoyancy;
			box->GetSubmergedVolume(translation, scale, Plane::sFromPointAndNormal(Vec3(0, 0, 12.001f), Vec3::sAxisZ()), total_volume, submerged_volume, center_of_buoyancy JPH_IF_DEBUG_RENDERER(, RVec3::sZero()));
			CHECK_APPROX_EQUAL(total_volume, 4.0f * 12.0f * 24.0f);
			CHECK_APPROX_EQUAL(submerged_volume, 4.0f * 12.0f * 24.0f);
			CHECK_APPROX_EQUAL(center_of_buoyancy, Vec3(0, 6, 0));
		}

		// Halfway through
		{
			float total_volume, submerged_volume;
			Vec3 center_of_buoyancy;
			box->GetSubmergedVolume(translation, scale, Plane::sFromPointAndNormal(Vec3(0, 0, 0), Vec3::sAxisZ()), total_volume, submerged_volume, center_of_buoyancy JPH_IF_DEBUG_RENDERER(, RVec3::sZero()));
			CHECK_APPROX_EQUAL(total_volume, 4.0f * 12.0f * 24.0f);
			CHECK_APPROX_EQUAL(submerged_volume, 4.0f * 12.0f * 12.0f);
			CHECK_APPROX_EQUAL(center_of_buoyancy, Vec3(0, 6, -6));
		}

		// Plane pointing negative Z
		// Entirely above the plane
		{
			float total_volume, submerged_volume;
			Vec3 center_of_buoyancy;
			box->GetSubmergedVolume(translation, scale, Plane::sFromPointAndNormal(Vec3(0, 0, 12.001f), -Vec3::sAxisZ()), total_volume, submerged_volume, center_of_buoyancy JPH_IF_DEBUG_RENDERER(, RVec3::sZero()));
			CHECK_APPROX_EQUAL(total_volume, 4.0f * 12.0f * 24.0f);
			CHECK_APPROX_EQUAL(submerged_volume, 0.0f);
		}

		// Entirely below the plane
		{
			float total_volume, submerged_volume;
			Vec3 center_of_buoyancy;
			box->GetSubmergedVolume(translation, scale, Plane::sFromPointAndNormal(Vec3(0, 0, -12.001f), -Vec3::sAxisZ()), total_volume, submerged_volume, center_of_buoyancy JPH_IF_DEBUG_RENDERER(, RVec3::sZero()));
			CHECK_APPROX_EQUAL(total_volume, 4.0f * 12.0f * 24.0f);
			CHECK_APPROX_EQUAL(submerged_volume, 4.0f * 12.0f * 24.0f);
			CHECK_APPROX_EQUAL(center_of_buoyancy, Vec3(0, 6, 0));
		}

		// Halfway through
		{
			float total_volume, submerged_volume;
			Vec3 center_of_buoyancy;
			box->GetSubmergedVolume(translation, scale, Plane::sFromPointAndNormal(Vec3(0, 0, 0), -Vec3::sAxisZ()), total_volume, submerged_volume, center_of_buoyancy JPH_IF_DEBUG_RENDERER(, RVec3::sZero()));
			CHECK_APPROX_EQUAL(total_volume, 4.0f * 12.0f * 24.0f);
			CHECK_APPROX_EQUAL(submerged_volume, 4.0f * 12.0f * 12.0f);
			CHECK_APPROX_EQUAL(center_of_buoyancy, Vec3(0, 6, 6));
		}
	}

	// Test setting user data on shapes
	TEST_CASE("TestShapeUserData")
	{
		const float cRadius = 2.0f;

		// Create a sphere with user data
		SphereShapeSettings sphere_settings(cRadius);
		sphere_settings.mUserData = 0x1234567887654321;
		Ref<Shape> sphere = sphere_settings.Create().Get();
		CHECK(sphere->GetUserData() == 0x1234567887654321);

		// Change the user data
		sphere->SetUserData(0x5678123443218765);
		CHECK(sphere->GetUserData() == 0x5678123443218765);

		stringstream data;

		// Write sphere to a binary stream
		{
			StreamOutWrapper stream_out(data);
			sphere->SaveBinaryState(stream_out);
		}

		// Destroy the sphere
		sphere = nullptr;

		// Read sphere from binary stream
		{
			StreamInWrapper stream_in(data);
			sphere = Shape::sRestoreFromBinaryState(stream_in).Get();
		}

		// Check that the sphere and its user data was preserved
		CHECK(sphere->GetType() == EShapeType::Convex);
		CHECK(sphere->GetSubType() == EShapeSubType::Sphere);
		CHECK(sphere->GetUserData() == 0x5678123443218765);
		CHECK(static_cast<SphereShape *>(sphere.GetPtr())->GetRadius() == cRadius);
	}

	// Test setting user data on shapes
	TEST_CASE("TestIsValidSubShapeID")
	{
		MutableCompoundShapeSettings shape1_settings;
		RefConst<CompoundShape> shape1 = static_cast<const CompoundShape *>(shape1_settings.Create().Get().GetPtr());

		MutableCompoundShapeSettings shape2_settings;
		shape2_settings.AddShape(Vec3::sZero(), Quat::sIdentity(), new SphereShape(1.0f));
		shape2_settings.AddShape(Vec3::sZero(), Quat::sIdentity(), new SphereShape(1.0f));
		shape2_settings.AddShape(Vec3::sZero(), Quat::sIdentity(), new SphereShape(1.0f));
		RefConst<CompoundShape> shape2 = static_cast<const CompoundShape *>(shape2_settings.Create().Get().GetPtr());

		// Get sub shape IDs of shape 2 and test if they're valid
		SubShapeID sub_shape1 = shape2->GetSubShapeIDFromIndex(0, SubShapeIDCreator()).GetID();
		CHECK(shape2->IsSubShapeIDValid(sub_shape1));
		SubShapeID sub_shape2 = shape2->GetSubShapeIDFromIndex(1, SubShapeIDCreator()).GetID();
		CHECK(shape2->IsSubShapeIDValid(sub_shape2));
		SubShapeID sub_shape3 = shape2->GetSubShapeIDFromIndex(2, SubShapeIDCreator()).GetID();
		CHECK(shape2->IsSubShapeIDValid(sub_shape3));
		SubShapeID sub_shape4 = shape2->GetSubShapeIDFromIndex(3, SubShapeIDCreator()).GetID(); // This one doesn't exist
		CHECK(!shape2->IsSubShapeIDValid(sub_shape4));

		// Shape 1 has no parts so these sub shape ID's should not be valid
		CHECK(!shape1->IsSubShapeIDValid(sub_shape1));
		CHECK(!shape1->IsSubShapeIDValid(sub_shape2));
		CHECK(!shape1->IsSubShapeIDValid(sub_shape3));
		CHECK(!shape1->IsSubShapeIDValid(sub_shape4));
	}

	// Test that an error is reported when we run out of sub shape bits
	TEST_CASE("TestOutOfSubShapeIDBits")
	{
		static constexpr uint32 cHeightFieldSamples = 1024;
		static constexpr int cNumBitsPerCompound = 4;

		// Create a heightfield
		float *samples = new float [cHeightFieldSamples * cHeightFieldSamples];
		memset(samples, 0, cHeightFieldSamples * cHeightFieldSamples * sizeof(float));
		RefConst<Shape> previous_shape = HeightFieldShapeSettings(samples, Vec3::sZero(), Vec3::sReplicate(1.0f), cHeightFieldSamples).Create().Get();
		delete [] samples;

		// Calculate the amount of bits needed to address all triangles in the heightfield
		uint num_bits = 32 - CountLeadingZeros((cHeightFieldSamples - 1) * (cHeightFieldSamples - 1) * 2);

		for (;;)
		{
			// Check that the total sub shape ID bits up to this point is correct
			CHECK(previous_shape->GetSubShapeIDBitsRecursive() == num_bits);

			// Create a compound with a number of sub shapes
			StaticCompoundShapeSettings compound_settings;
			compound_settings.SetEmbedded();
			for (int i = 0; i < (1 << cNumBitsPerCompound) ; ++i)
				compound_settings.AddShape(Vec3((float)i, 0, 0), Quat::sIdentity(), previous_shape);
			Shape::ShapeResult result = compound_settings.Create();
			num_bits += cNumBitsPerCompound;

			if (num_bits < SubShapeID::MaxBits)
			{
				// Creation should have succeeded
				CHECK(result.IsValid());
				previous_shape = result.Get();
			}
			else
			{
				// Creation should have failed because we ran out of bits
				CHECK(!result.IsValid());
				break;
			}
		}
	}

	TEST_CASE("TestEmptyMutableCompound")
	{
		// Create empty shape
		RefConst<Shape> mutable_compound = new MutableCompoundShape();

		// A non-identity rotation
		Quat rotation = Quat::sRotation(Vec3::sReplicate(1.0f / sqrt(3.0f)), 0.1f * JPH_PI);

		// Check that local bounding box is invalid
		AABox bounds1 = mutable_compound->GetLocalBounds();
		CHECK(!bounds1.IsValid());

		// Check that get world space bounds returns an invalid bounding box
		AABox bounds2 = mutable_compound->GetWorldSpaceBounds(Mat44::sRotationTranslation(rotation, Vec3(100, 200, 300)), Vec3(1, 2, 3));
		CHECK(!bounds2.IsValid());

		// Check that get world space bounds returns an invalid bounding box for double precision parameters
		AABox bounds3 = mutable_compound->GetWorldSpaceBounds(DMat44::sRotationTranslation(rotation, DVec3(100, 200, 300)), Vec3(1, 2, 3));
		CHECK(!bounds3.IsValid());
	}
}
