// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/TaperedCapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/CylinderShape.h>
#include <Jolt/Physics/Collision/Shape/TaperedCylinderShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>
#include <Jolt/Physics/Collision/Shape/MutableCompoundShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Layers.h>

TEST_SUITE("RayShapeTests")
{
	// Function that does the actual ray cast test, inExpectedFraction1/2 should be FLT_MAX if no hit expected
	using TestFunction = function<void(const RayCast &inRay, float inExpectedFraction1, float inExpectedFraction2)>;

	// Test ray against inShape with lines going through inHitA and inHitB (which should be surface positions of the shape)
	static void TestRayHelperInternal(Vec3Arg inHitA, Vec3Arg inHitB, TestFunction inTestFunction)
	{
		// Determine points before and after the surface on both sides
		Vec3 delta = inHitB - inHitA;
		Vec3 l1 = inHitA - 2.0f * delta;
		Vec3 l2 = inHitA - 0.1f * delta;
		Vec3 i1 = inHitA + 0.1f * delta;
		Vec3 i2 = inHitB - 0.1f * delta;
		Vec3 r1 = inHitB + 0.1f * delta;
		Vec3 r2 = inHitB + 2.0f * delta;

		// -O---->-|--------|--------
		inTestFunction(RayCast { l1, l2 - l1 }, FLT_MAX, FLT_MAX);

		// -----O>-|--------|--------
		inTestFunction(RayCast { l2, Vec3::sZero() }, FLT_MAX, FLT_MAX);

		// ------O-|->------|--------
		inTestFunction(RayCast { l2, i1 - l2 }, 0.5f, FLT_MAX);

		// ------O-|--------|->------
		inTestFunction(RayCast { l2, r1 - l2 }, 0.1f / 1.2f, 1.1f / 1.2f);

		// --------|-----O>-|--------
		inTestFunction(RayCast { i2, Vec3::sZero() }, 0.0f, FLT_MAX);

		// --------|------O-|->------
		inTestFunction(RayCast { i2, r1 - i2 }, 0.0f, 0.5f);

		// --------|--------|-O---->-
		inTestFunction(RayCast { r1, r2 - l1 }, FLT_MAX, FLT_MAX);
	}

	static void TestRayHelper(const Shape *inShape, Vec3Arg inHitA, Vec3Arg inHitB)
	{
		//////////////////////////////////////////////////////////////////////////////////////////////////
		// Test function that directly tests against a shape
		//////////////////////////////////////////////////////////////////////////////////////////////////
		TestFunction TestShapeRay = [inShape](const RayCast &inRay, float inExpectedFraction1, float inExpectedFraction2)
		{
			// CastRay works relative to center of mass, so transform the ray
			RayCast ray = inRay;
			ray.mOrigin -= inShape->GetCenterOfMass();

			RayCastResult hit;
			SubShapeIDCreator id_creator;
			if (inExpectedFraction1 != FLT_MAX)
			{
				CHECK(inShape->CastRay(ray, id_creator, hit));
				CHECK_APPROX_EQUAL(hit.mFraction, inExpectedFraction1, 1.0e-5f);
			}
			else
			{
				CHECK_FALSE(inShape->CastRay(ray, id_creator, hit));
			}
		};

		// Test normal ray
		TestRayHelperInternal(inHitA, inHitB, TestShapeRay);

		// Test inverse ray
		TestRayHelperInternal(inHitB, inHitA, TestShapeRay);


		//////////////////////////////////////////////////////////////////////////////////////////////////
		// Test function that directly tests against a shape allowing multiple hits but no back facing hits, treating convex objects as solids
		//////////////////////////////////////////////////////////////////////////////////////////////////
		TestFunction TestShapeRayMultiHitIgnoreBackFace = [inShape](const RayCast &inRay, float inExpectedFraction1, float inExpectedFraction2)
		{
			// CastRay works relative to center of mass, so transform the ray
			RayCast ray = inRay;
			ray.mOrigin -= inShape->GetCenterOfMass();

			// Ray cast settings
			RayCastSettings settings;
			settings.SetBackFaceMode(EBackFaceMode::IgnoreBackFaces);
			settings.mTreatConvexAsSolid = true;

			AllHitCollisionCollector<CastRayCollector> collector;
			SubShapeIDCreator id_creator;
			inShape->CastRay(ray, settings, id_creator, collector);

			if (inExpectedFraction1 != FLT_MAX)
			{
				CHECK(collector.mHits.size() == 1);
				CHECK_APPROX_EQUAL(collector.mHits[0].mFraction, inExpectedFraction1, 1.0e-5f);
			}
			else
			{
				CHECK(collector.mHits.empty());
			}
		};

		// Test normal ray
		TestRayHelperInternal(inHitA, inHitB, TestShapeRayMultiHitIgnoreBackFace);

		// Test inverse ray
		TestRayHelperInternal(inHitB, inHitA, TestShapeRayMultiHitIgnoreBackFace);


		//////////////////////////////////////////////////////////////////////////////////////////////////
		// Test function that directly tests against a shape allowing multiple hits and back facing hits, treating convex objects as solids
		//////////////////////////////////////////////////////////////////////////////////////////////////
		TestFunction TestShapeRayMultiHitWithBackFace = [inShape](const RayCast &inRay, float inExpectedFraction1, float inExpectedFraction2)
		{
			// CastRay works relative to center of mass, so transform the ray
			RayCast ray = inRay;
			ray.mOrigin -= inShape->GetCenterOfMass();

			// Ray cast settings
			RayCastSettings settings;
			settings.SetBackFaceMode(EBackFaceMode::CollideWithBackFaces);
			settings.mTreatConvexAsSolid = true;

			AllHitCollisionCollector<CastRayCollector> collector;
			SubShapeIDCreator id_creator;
			inShape->CastRay(ray, settings, id_creator, collector);

			if (inExpectedFraction1 != FLT_MAX)
			{
				CHECK(collector.mHits.size() >= 1);
				CHECK_APPROX_EQUAL(collector.mHits[0].mFraction, inExpectedFraction1, 1.0e-5f);
			}
			else
			{
				JPH_ASSERT(inExpectedFraction2 == FLT_MAX);
				CHECK(collector.mHits.empty());
			}

			if (inExpectedFraction2 != FLT_MAX)
			{
				CHECK(collector.mHits.size() >= 2);
				CHECK_APPROX_EQUAL(collector.mHits[1].mFraction, inExpectedFraction2, 1.0e-5f);
			}
			else
			{
				CHECK(collector.mHits.size() < 2);
			}
		};

		// Test normal ray
		TestRayHelperInternal(inHitA, inHitB, TestShapeRayMultiHitWithBackFace);

		// Test inverse ray
		TestRayHelperInternal(inHitB, inHitA, TestShapeRayMultiHitWithBackFace);


		//////////////////////////////////////////////////////////////////////////////////////////////////
		// Test function that directly tests against a shape allowing multiple hits but no back facing hits, treating convex object as non-solids
		//////////////////////////////////////////////////////////////////////////////////////////////////
		TestFunction TestShapeRayMultiHitIgnoreBackFaceNonSolid = [inShape](const RayCast &inRay, float inExpectedFraction1, float inExpectedFraction2)
		{
			// CastRay works relative to center of mass, so transform the ray
			RayCast ray = inRay;
			ray.mOrigin -= inShape->GetCenterOfMass();

			// Ray cast settings
			RayCastSettings settings;
			settings.SetBackFaceMode(EBackFaceMode::IgnoreBackFaces);
			settings.mTreatConvexAsSolid = false;

			AllHitCollisionCollector<CastRayCollector> collector;
			SubShapeIDCreator id_creator;
			inShape->CastRay(ray, settings, id_creator, collector);

			// A fraction of 0 means that the ray starts in solid, we treat this as a non-hit
			if (inExpectedFraction1 != 0.0f && inExpectedFraction1 != FLT_MAX)
			{
				CHECK(collector.mHits.size() == 1);
				CHECK_APPROX_EQUAL(collector.mHits[0].mFraction, inExpectedFraction1, 1.0e-5f);
			}
			else
			{
				CHECK(collector.mHits.empty());
			}
		};

		// Test normal ray
		TestRayHelperInternal(inHitA, inHitB, TestShapeRayMultiHitIgnoreBackFaceNonSolid);

		// Test inverse ray
		TestRayHelperInternal(inHitB, inHitA, TestShapeRayMultiHitIgnoreBackFaceNonSolid);


		//////////////////////////////////////////////////////////////////////////////////////////////////
		// Test function that directly tests against a shape allowing multiple hits and back facing hits, treating convex object as non-solids
		//////////////////////////////////////////////////////////////////////////////////////////////////
		TestFunction TestShapeRayMultiHitWithBackFaceNonSolid = [inShape](const RayCast &inRay, float inExpectedFraction1, float inExpectedFraction2)
		{
			// CastRay works relative to center of mass, so transform the ray
			RayCast ray = inRay;
			ray.mOrigin -= inShape->GetCenterOfMass();

			// Ray cast settings
			RayCastSettings settings;
			settings.SetBackFaceMode(EBackFaceMode::CollideWithBackFaces);
			settings.mTreatConvexAsSolid = false;

			AllHitCollisionCollector<CastRayCollector> collector;
			SubShapeIDCreator id_creator;
			inShape->CastRay(ray, settings, id_creator, collector);

			// A fraction of 0 means that the ray starts in solid, we treat this as a non-hit
			if (inExpectedFraction1 == 0.0f)
			{
				inExpectedFraction1 = inExpectedFraction2;
				inExpectedFraction2 = FLT_MAX;
			}

			if (inExpectedFraction1 != FLT_MAX)
			{
				CHECK(collector.mHits.size() >= 1);
				CHECK_APPROX_EQUAL(collector.mHits[0].mFraction, inExpectedFraction1, 1.0e-5f);
			}
			else
			{
				JPH_ASSERT(inExpectedFraction2 == FLT_MAX);
				CHECK(collector.mHits.empty());
			}

			if (inExpectedFraction2 != FLT_MAX)
			{
				CHECK(collector.mHits.size() >= 2);
				CHECK_APPROX_EQUAL(collector.mHits[1].mFraction, inExpectedFraction2, 1.0e-5f);
			}
			else
			{
				CHECK(collector.mHits.size() < 2);
			}
		};

		// Test normal ray
		TestRayHelperInternal(inHitA, inHitB, TestShapeRayMultiHitWithBackFaceNonSolid);

		// Test inverse ray
		TestRayHelperInternal(inHitB, inHitA, TestShapeRayMultiHitWithBackFaceNonSolid);


		//////////////////////////////////////////////////////////////////////////////////////////////////
		// Insert the shape into the world
		//////////////////////////////////////////////////////////////////////////////////////////////////

		// A non-zero test position for the shape
		const Vec3 cShapePosition(2, 3, 4);
		const Quat cShapeRotation = Quat::sRotation(Vec3::sAxisX(), 0.25f * JPH_PI);
		const Mat44 cShapeMatrix = Mat44::sRotationTranslation(cShapeRotation, cShapePosition);

		// Make the shape part of a body and insert it into the physics system
		BPLayerInterfaceImpl broad_phase_layer_interface;
		ObjectVsBroadPhaseLayerFilter object_vs_broadphase_layer_filter;
		ObjectLayerPairFilter object_vs_object_layer_filter;
		PhysicsSystem system;
		system.Init(1, 0, 4, 4, broad_phase_layer_interface, object_vs_broadphase_layer_filter, object_vs_object_layer_filter);
		system.GetBodyInterface().CreateAndAddBody(BodyCreationSettings(inShape, RVec3(cShapePosition), cShapeRotation, EMotionType::Static, 0), EActivation::DontActivate);


		//////////////////////////////////////////////////////////////////////////////////////////////////
		// Test a ray against a shape through a physics system
		//////////////////////////////////////////////////////////////////////////////////////////////////
		TestFunction TestSystemRay = [&system, cShapeMatrix](const RayCast &inRay, float inExpectedFraction1, float inExpectedFraction2)
		{
			// inRay is relative to shape, transform it into world space
			RayCast ray = inRay.Transformed(cShapeMatrix);

			RayCastResult hit;
			if (inExpectedFraction1 != FLT_MAX)
			{
				CHECK(system.GetNarrowPhaseQuery().CastRay(RRayCast(ray), hit));
				CHECK_APPROX_EQUAL(hit.mFraction, inExpectedFraction1, 2.5e-5f);
			}
			else
			{
				CHECK_FALSE(system.GetNarrowPhaseQuery().CastRay(RRayCast(ray), hit));
			}
		};

		// Test normal ray
		TestRayHelperInternal(inHitA, inHitB, TestSystemRay);

		// Test inverse ray
		TestRayHelperInternal(inHitB, inHitA, TestSystemRay);


		//////////////////////////////////////////////////////////////////////////////////////////////////
		// Test a ray against a shape through a physics system allowing multiple hits but no back facing hits
		//////////////////////////////////////////////////////////////////////////////////////////////////
		TestFunction TestSystemRayMultiHitIgnoreBackFace = [&system, cShapeMatrix](const RayCast &inRay, float inExpectedFraction1, float inExpectedFraction2)
		{
			// inRay is relative to shape, transform it into world space
			RayCast ray = inRay.Transformed(cShapeMatrix);

			// Ray cast settings
			RayCastSettings settings;
			settings.SetBackFaceMode(EBackFaceMode::IgnoreBackFaces);
			settings.mTreatConvexAsSolid = true;

			AllHitCollisionCollector<CastRayCollector> collector;
			system.GetNarrowPhaseQuery().CastRay(RRayCast(ray), settings, collector);

			if (inExpectedFraction1 != FLT_MAX)
			{
				CHECK(collector.mHits.size() == 1);
				CHECK_APPROX_EQUAL(collector.mHits[0].mFraction, inExpectedFraction1, 2.5e-5f);
			}
			else
			{
				CHECK(collector.mHits.empty());
			}
		};

		// Test normal ray
		TestRayHelperInternal(inHitA, inHitB, TestSystemRayMultiHitIgnoreBackFace);

		// Test inverse ray
		TestRayHelperInternal(inHitB, inHitA, TestSystemRayMultiHitIgnoreBackFace);


		//////////////////////////////////////////////////////////////////////////////////////////////////
		// Test a ray against a shape through a physics system allowing multiple hits and back facing hits
		//////////////////////////////////////////////////////////////////////////////////////////////////
		TestFunction TestSystemRayMultiHitWithBackFace = [&system, cShapeMatrix](const RayCast &inRay, float inExpectedFraction1, float inExpectedFraction2)
		{
			// inRay is relative to shape, transform it into world space
			RayCast ray = inRay.Transformed(cShapeMatrix);

			// Ray cast settings
			RayCastSettings settings;
			settings.SetBackFaceMode(EBackFaceMode::CollideWithBackFaces);
			settings.mTreatConvexAsSolid = true;

			AllHitCollisionCollector<CastRayCollector> collector;
			system.GetNarrowPhaseQuery().CastRay(RRayCast(ray), settings, collector);
			collector.Sort();

			if (inExpectedFraction1 != FLT_MAX)
			{
				CHECK(collector.mHits.size() >= 1);
				CHECK_APPROX_EQUAL(collector.mHits[0].mFraction, inExpectedFraction1, 2.5e-5f);
			}
			else
			{
				JPH_ASSERT(inExpectedFraction2 == FLT_MAX);
				CHECK(collector.mHits.empty());
			}

			if (inExpectedFraction2 != FLT_MAX)
			{
				CHECK(collector.mHits.size() >= 2);
				CHECK_APPROX_EQUAL(collector.mHits[1].mFraction, inExpectedFraction2, 2.5e-5f);
			}
			else
			{
				CHECK(collector.mHits.size() < 2);
			}
		};

		// Test normal ray
		TestRayHelperInternal(inHitA, inHitB, TestSystemRayMultiHitWithBackFace);

		// Test inverse ray
		TestRayHelperInternal(inHitB, inHitA, TestSystemRayMultiHitWithBackFace);
	}

	/// Helper function to check that a ray misses a shape
	static void TestRayMiss(const Shape *inShape, Vec3Arg inOrigin, Vec3Arg inDirection)
	{
		RayCastResult hit;
		CHECK(!inShape->CastRay({ inOrigin - inShape->GetCenterOfMass(), inDirection }, SubShapeIDCreator(), hit));
	}

	TEST_CASE("TestBoxShapeRay")
	{
		// Create box shape
		BoxShape box(Vec3(2, 3, 4)); // Allocate on the stack to test embedded refcounted structs
		box.SetEmbedded();
		Ref<Shape> shape = &box; // Add a reference to see if we don't hit free() of a stack allocated struct

		TestRayHelper(shape, Vec3(-2, 0, 0), Vec3(2, 0, 0));
		TestRayHelper(shape, Vec3(0, -3, 0), Vec3(0, 3, 0));
		TestRayHelper(shape, Vec3(0, 0, -4), Vec3(0, 0, 4));
	}

	TEST_CASE("TestSphereShapeRay")
	{
		// Create sphere shape
		Ref<Shape> shape = new SphereShape(2);

		TestRayHelper(shape, Vec3(-2, 0, 0), Vec3(2, 0, 0));
		TestRayHelper(shape, Vec3(0, -2, 0), Vec3(0, 2, 0));
		TestRayHelper(shape, Vec3(0, 0, -2), Vec3(0, 0, 2));
	}

	TEST_CASE("TestConvexHullShapeRay")
	{
		// Create convex hull shape of a box (off center so the center of mass is not zero)
		Array<Vec3> box;
		box.push_back(Vec3(-2, -4, -6));
		box.push_back(Vec3(-2, -4, 7));
		box.push_back(Vec3(-2, 5, -6));
		box.push_back(Vec3(-2, 5, 7));
		box.push_back(Vec3(3, -4, -6));
		box.push_back(Vec3(3, -4, 7));
		box.push_back(Vec3(3, 5, -6));
		box.push_back(Vec3(3, 5, 7));
		RefConst<Shape> shape = ConvexHullShapeSettings(box).Create().Get();

		TestRayHelper(shape, Vec3(-2, 0, 0), Vec3(3, 0, 0));
		TestRayHelper(shape, Vec3(0, -4, 0), Vec3(0, 5, 0));
		TestRayHelper(shape, Vec3(0, 0, -6), Vec3(0, 0, 7));

		TestRayMiss(shape, Vec3(-3, -5, 0), Vec3(0, 1, 0));
		TestRayMiss(shape, Vec3(-3, 0, 0), Vec3(0, 1, 0));
		TestRayMiss(shape, Vec3(-3, 6, 0), Vec3(0, 1, 0));
	}

	TEST_CASE("TestCapsuleShapeRay")
	{
		// Create capsule shape
		Ref<Shape> shape = new CapsuleShape(4, 2);

		TestRayHelper(shape, Vec3(-2, 0, 0), Vec3(2, 0, 0));
		TestRayHelper(shape, Vec3(0, -6, 0), Vec3(0, 6, 0));
		TestRayHelper(shape, Vec3(0, 0, -2), Vec3(0, 0, 2));
	}

	TEST_CASE("TestTaperedCapsuleShapeRay")
	{
		// Create tapered capsule shape
		RefConst<Shape> shape = TaperedCapsuleShapeSettings(3, 4, 2).Create().Get();

		TestRayHelper(shape, Vec3(0, 7, 0), Vec3(0, -5, 0)); // Top to bottom
		TestRayHelper(shape, Vec3(-4, 3, 0), Vec3(4, 3, 0)); // Top sphere
		TestRayHelper(shape, Vec3(0, 3, -4), Vec3(0, 3, 4)); // Top sphere
	}

	TEST_CASE("TestCylinderShapeRay")
	{
		// Create cylinder shape
		Ref<Shape> shape = new CylinderShape(4, 2);

		TestRayHelper(shape, Vec3(-2, 0, 0), Vec3(2, 0, 0));
		TestRayHelper(shape, Vec3(0, -4, 0), Vec3(0, 4, 0));
		TestRayHelper(shape, Vec3(0, 0, -2), Vec3(0, 0, 2));
	}

	TEST_CASE("TestTaperedCylinderShapeRay")
	{
		// Create tapered cylinder shape
		Ref<Shape> shape = TaperedCylinderShapeSettings(4, 1, 3).Create().Get();

		// Ray through origin
		TestRayHelper(shape, Vec3(-2, 0, 0), Vec3(2, 0, 0));
		TestRayHelper(shape, Vec3(0, -4, 0), Vec3(0, 4, 0));
		TestRayHelper(shape, Vec3(0, 0, -2), Vec3(0, 0, 2));

		// Ray halfway to the top
		TestRayHelper(shape, Vec3(-1.5f, 2, 0), Vec3(1.5f, 2, 0));
		TestRayHelper(shape, Vec3(0, 2, -1.5f), Vec3(0, 2, 1.5f));

		// Ray halfway to the bottom
		TestRayHelper(shape, Vec3(-2.5f, -2, 0), Vec3(2.5f, -2, 0));
		TestRayHelper(shape, Vec3(0, -2, -2.5f), Vec3(0, -2, 2.5f));
	}

	TEST_CASE("TestScaledShapeRay")
	{
		// Create convex hull shape of a box (off center so the center of mass is not zero)
		Array<Vec3> box;
		box.push_back(Vec3(-2, -4, -6));
		box.push_back(Vec3(-2, -4, 7));
		box.push_back(Vec3(-2, 5, -6));
		box.push_back(Vec3(-2, 5, 7));
		box.push_back(Vec3(3, -4, -6));
		box.push_back(Vec3(3, -4, 7));
		box.push_back(Vec3(3, 5, -6));
		box.push_back(Vec3(3, 5, 7));
		RefConst<Shape> hull = ConvexHullShapeSettings(box).Create().Get();

		// Scale the hull
		Ref<Shape> shape1 = new ScaledShape(hull, Vec3(2, 3, 4));

		TestRayHelper(shape1, Vec3(-4, 0, 0), Vec3(6, 0, 0));
		TestRayHelper(shape1, Vec3(0, -12, 0), Vec3(0, 15, 0));
		TestRayHelper(shape1, Vec3(0, 0, -24), Vec3(0, 0, 28));

		// Scale the hull (and flip it inside out)
		Ref<Shape> shape2 = new ScaledShape(hull, Vec3(-2, 3, 4));

		TestRayHelper(shape2, Vec3(-6, 0, 0), Vec3(4, 0, 0));
		TestRayHelper(shape2, Vec3(0, -12, 0), Vec3(0, 15, 0));
		TestRayHelper(shape2, Vec3(0, 0, -24), Vec3(0, 0, 28));
	}

	TEST_CASE("TestStaticCompoundShapeRay")
	{
		// Create convex hull shape of a box (off center so the center of mass is not zero)
		Array<Vec3> box;
		box.push_back(Vec3(-2, -4, -6));
		box.push_back(Vec3(-2, -4, 7));
		box.push_back(Vec3(-2, 5, -6));
		box.push_back(Vec3(-2, 5, 7));
		box.push_back(Vec3(3, -4, -6));
		box.push_back(Vec3(3, -4, 7));
		box.push_back(Vec3(3, 5, -6));
		box.push_back(Vec3(3, 5, 7));
		RefConst<ShapeSettings> hull = new ConvexHullShapeSettings(box);

		// Translate/rotate the shape through a compound (off center to force center of mass not zero)
		const Vec3 cShape1Position(10, 20, 30);
		const Quat cShape1Rotation = Quat::sRotation(Vec3::sAxisX(), 0.1f * JPH_PI) * Quat::sRotation(Vec3::sAxisY(), 0.2f * JPH_PI);
		const Vec3 cShape2Position(40, 50, 60);
		const Quat cShape2Rotation = Quat::sRotation(Vec3::sAxisZ(), 0.3f * JPH_PI);

		StaticCompoundShapeSettings compound_settings;
		compound_settings.AddShape(cShape1Position, cShape1Rotation, hull); // Shape 1
		compound_settings.AddShape(cShape2Position, cShape2Rotation, hull); // Shape 2
		RefConst<Shape> compound = compound_settings.Create().Get();

		// Hitting shape 1
		TestRayHelper(compound, cShape1Position + cShape1Rotation * Vec3(-2, 0, 0), cShape1Position + cShape1Rotation * Vec3(3, 0, 0));
		TestRayHelper(compound, cShape1Position + cShape1Rotation * Vec3(0, -4, 0), cShape1Position + cShape1Rotation * Vec3(0, 5, 0));
		TestRayHelper(compound, cShape1Position + cShape1Rotation * Vec3(0, 0, -6), cShape1Position + cShape1Rotation * Vec3(0, 0, 7));

		// Hitting shape 2
		TestRayHelper(compound, cShape2Position + cShape2Rotation * Vec3(-2, 0, 0), cShape2Position + cShape2Rotation * Vec3(3, 0, 0));
		TestRayHelper(compound, cShape2Position + cShape2Rotation * Vec3(0, -4, 0), cShape2Position + cShape2Rotation * Vec3(0, 5, 0));
		TestRayHelper(compound, cShape2Position + cShape2Rotation * Vec3(0, 0, -6), cShape2Position + cShape2Rotation * Vec3(0, 0, 7));
	}

	TEST_CASE("TestMutableCompoundShapeRay")
	{
		// Create convex hull shape of a box (off center so the center of mass is not zero)
		Array<Vec3> box;
		box.push_back(Vec3(-2, -4, -6));
		box.push_back(Vec3(-2, -4, 7));
		box.push_back(Vec3(-2, 5, -6));
		box.push_back(Vec3(-2, 5, 7));
		box.push_back(Vec3(3, -4, -6));
		box.push_back(Vec3(3, -4, 7));
		box.push_back(Vec3(3, 5, -6));
		box.push_back(Vec3(3, 5, 7));
		RefConst<ShapeSettings> hull = new ConvexHullShapeSettings(box);

		// Translate/rotate the shape through a compound (off center to force center of mass not zero)
		const Vec3 cShape1Position(10, 20, 30);
		const Quat cShape1Rotation = Quat::sRotation(Vec3::sAxisX(), 0.1f * JPH_PI) * Quat::sRotation(Vec3::sAxisY(), 0.2f * JPH_PI);
		const Vec3 cShape2Position(40, 50, 60);
		const Quat cShape2Rotation = Quat::sRotation(Vec3::sAxisZ(), 0.3f * JPH_PI);

		MutableCompoundShapeSettings compound_settings;
		compound_settings.AddShape(cShape1Position, cShape1Rotation, hull); // Shape 1
		compound_settings.AddShape(cShape2Position, cShape2Rotation, hull); // Shape 2
		RefConst<Shape> compound = compound_settings.Create().Get();

		// Hitting shape 1
		TestRayHelper(compound, cShape1Position + cShape1Rotation * Vec3(-2, 0, 0), cShape1Position + cShape1Rotation * Vec3(3, 0, 0));
		TestRayHelper(compound, cShape1Position + cShape1Rotation * Vec3(0, -4, 0), cShape1Position + cShape1Rotation * Vec3(0, 5, 0));
		TestRayHelper(compound, cShape1Position + cShape1Rotation * Vec3(0, 0, -6), cShape1Position + cShape1Rotation * Vec3(0, 0, 7));

		// Hitting shape 2
		TestRayHelper(compound, cShape2Position + cShape2Rotation * Vec3(-2, 0, 0), cShape2Position + cShape2Rotation * Vec3(3, 0, 0));
		TestRayHelper(compound, cShape2Position + cShape2Rotation * Vec3(0, -4, 0), cShape2Position + cShape2Rotation * Vec3(0, 5, 0));
		TestRayHelper(compound, cShape2Position + cShape2Rotation * Vec3(0, 0, -6), cShape2Position + cShape2Rotation * Vec3(0, 0, 7));
	}
}
