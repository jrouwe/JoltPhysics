// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include <Jolt/Physics/Collision/ShapeCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/TriangleShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/ShapeFilter.h>
#include <Jolt/Physics/Collision/CollisionDispatch.h>
#include "PhysicsTestContext.h"
#include "Layers.h"

TEST_SUITE("CastShapeTests")
{
	/// Helper function that tests a sphere against a triangle
	static void sTestCastSphereVertexOrEdge(const Shape *inSphere, Vec3Arg inPosition, Vec3Arg inDirection, const Shape *inTriangle)
	{
		ShapeCast shape_cast(inSphere, Vec3::sReplicate(1.0f), Mat44::sTranslation(inPosition - inDirection), inDirection);
		ShapeCastSettings cast_settings;
		cast_settings.mBackFaceModeTriangles = EBackFaceMode::CollideWithBackFaces;
		cast_settings.mBackFaceModeConvex = EBackFaceMode::CollideWithBackFaces;
		AllHitCollisionCollector<CastShapeCollector> collector;
		CollisionDispatch::sCastShapeVsShapeLocalSpace(shape_cast, cast_settings, inTriangle, Vec3::sReplicate(1.0f), ShapeFilter(), Mat44::sIdentity(), SubShapeIDCreator(), SubShapeIDCreator(), collector);
		CHECK(collector.mHits.size() == 1);
		const ShapeCastResult &result = collector.mHits.back();
		CHECK_APPROX_EQUAL(result.mFraction, 1.0f - 0.2f / inDirection.Length(), 1.0e-4f);
		CHECK_APPROX_EQUAL(result.mPenetrationAxis.Normalized(), inDirection.Normalized(), 1.0e-3f);
		CHECK_APPROX_EQUAL(result.mPenetrationDepth, 0.0f, 1.0e-3f);
		CHECK_APPROX_EQUAL(result.mContactPointOn1, inPosition, 1.0e-3f);
		CHECK_APPROX_EQUAL(result.mContactPointOn2, inPosition, 1.0e-3f);
	}

	/// Helper function that tests a shere against a triangle centered on the origin with normal Z
	static void sTestCastSphereTriangle(const Shape *inTriangle)
	{
		// Create sphere
		Ref<Shape> sphere = SphereShapeSettings(0.2f).Create().Get();

		{
			// Hit front face
			ShapeCast shape_cast(sphere, Vec3::sReplicate(1.0f), Mat44::sTranslation(Vec3(0, 0, 15)), Vec3(0, 0, -30));
			ShapeCastSettings cast_settings;
			cast_settings.mBackFaceModeTriangles = EBackFaceMode::IgnoreBackFaces;
			cast_settings.mBackFaceModeConvex = EBackFaceMode::IgnoreBackFaces;
			cast_settings.mReturnDeepestPoint = false;
			AllHitCollisionCollector<CastShapeCollector> collector;
			CollisionDispatch::sCastShapeVsShapeLocalSpace(shape_cast, cast_settings, inTriangle, Vec3::sReplicate(1.0f), ShapeFilter(), Mat44::sIdentity(), SubShapeIDCreator(), SubShapeIDCreator(), collector);
			CHECK(collector.mHits.size() == 1);
			const ShapeCastResult &result = collector.mHits.back();
			CHECK_APPROX_EQUAL(result.mFraction, (15.0f - 0.2f) / 30.0f, 1.0e-4f);
			CHECK_APPROX_EQUAL(result.mPenetrationAxis.Normalized(), Vec3(0, 0, -1), 1.0e-3f);
			CHECK(result.mPenetrationDepth == 0.0f);
			CHECK_APPROX_EQUAL(result.mContactPointOn1, Vec3::sZero(), 1.0e-3f);
			CHECK_APPROX_EQUAL(result.mContactPointOn2, Vec3::sZero(), 1.0e-3f);
			CHECK(!result.mIsBackFaceHit);
		}

		{
			// Hit back face -> ignored
			ShapeCast shape_cast(sphere, Vec3::sReplicate(1.0f), Mat44::sTranslation(Vec3(0, 0, -15)), Vec3(0, 0, 30));
			ShapeCastSettings cast_settings;
			cast_settings.mBackFaceModeTriangles = EBackFaceMode::IgnoreBackFaces;
			cast_settings.mBackFaceModeConvex = EBackFaceMode::IgnoreBackFaces;
			cast_settings.mReturnDeepestPoint = false;
			AllHitCollisionCollector<CastShapeCollector> collector;
			CollisionDispatch::sCastShapeVsShapeLocalSpace(shape_cast, cast_settings, inTriangle, Vec3::sReplicate(1.0f), ShapeFilter(), Mat44::sIdentity(), SubShapeIDCreator(), SubShapeIDCreator(), collector);
			CHECK(collector.mHits.empty());

			// Hit back face -> collision
			cast_settings.mBackFaceModeTriangles = EBackFaceMode::CollideWithBackFaces;
			cast_settings.mBackFaceModeConvex = EBackFaceMode::CollideWithBackFaces;
			CollisionDispatch::sCastShapeVsShapeLocalSpace(shape_cast, cast_settings, inTriangle, Vec3::sReplicate(1.0f), ShapeFilter(), Mat44::sIdentity(), SubShapeIDCreator(), SubShapeIDCreator(), collector);
			CHECK(collector.mHits.size() == 1);
			const ShapeCastResult &result = collector.mHits.back();
			CHECK_APPROX_EQUAL(result.mFraction, (15.0f - 0.2f) / 30.0f, 1.0e-4f);
			CHECK_APPROX_EQUAL(result.mPenetrationAxis.Normalized(), Vec3(0, 0, 1), 1.0e-3f);
			CHECK(result.mPenetrationDepth == 0.0f);
			CHECK_APPROX_EQUAL(result.mContactPointOn1, Vec3::sZero(), 1.0e-3f);
			CHECK_APPROX_EQUAL(result.mContactPointOn2, Vec3::sZero(), 1.0e-3f);
			CHECK(result.mIsBackFaceHit);
		}

		{
			// Hit back face while starting in collision -> ignored
			ShapeCast shape_cast(sphere, Vec3::sReplicate(1.0f), Mat44::sTranslation(Vec3(0, 0, -0.1f)), Vec3(0, 0, 15));
			ShapeCastSettings cast_settings;
			cast_settings.mBackFaceModeTriangles = EBackFaceMode::IgnoreBackFaces;
			cast_settings.mBackFaceModeConvex = EBackFaceMode::IgnoreBackFaces;
			cast_settings.mReturnDeepestPoint = true;
			AllHitCollisionCollector<CastShapeCollector> collector;
			CollisionDispatch::sCastShapeVsShapeLocalSpace(shape_cast, cast_settings, inTriangle, Vec3::sReplicate(1.0f), ShapeFilter(), Mat44::sIdentity(), SubShapeIDCreator(), SubShapeIDCreator(), collector);
			CHECK(collector.mHits.empty());

			// Hit back face while starting in collision -> collision
			cast_settings.mBackFaceModeTriangles = EBackFaceMode::CollideWithBackFaces;
			cast_settings.mBackFaceModeConvex = EBackFaceMode::CollideWithBackFaces;
			CollisionDispatch::sCastShapeVsShapeLocalSpace(shape_cast, cast_settings, inTriangle, Vec3::sReplicate(1.0f), ShapeFilter(), Mat44::sIdentity(), SubShapeIDCreator(), SubShapeIDCreator(), collector);
			CHECK(collector.mHits.size() == 1);
			const ShapeCastResult &result = collector.mHits.back();
			CHECK_APPROX_EQUAL(result.mFraction, 0.0f);
			CHECK_APPROX_EQUAL(result.mPenetrationAxis.Normalized(), Vec3(0, 0, 1), 1.0e-3f);
			CHECK_APPROX_EQUAL(result.mPenetrationDepth, 0.1f, 1.0e-3f);
			CHECK_APPROX_EQUAL(result.mContactPointOn1, Vec3(0, 0, 0.1f), 1.0e-3f);
			CHECK_APPROX_EQUAL(result.mContactPointOn2, Vec3::sZero(), 1.0e-3f);
			CHECK(result.mIsBackFaceHit);
		}

		// Hit vertex 1, 2 and 3
		sTestCastSphereVertexOrEdge(sphere, Vec3(50, 25, 0), Vec3(-10, -10, 0), inTriangle);
		sTestCastSphereVertexOrEdge(sphere, Vec3(-50, 25, 0), Vec3(10, -10, 0), inTriangle);
		sTestCastSphereVertexOrEdge(sphere, Vec3(0, -25, 0), Vec3(0, 10, 0), inTriangle);

		// Hit edge 1, 2 and 3
		sTestCastSphereVertexOrEdge(sphere, Vec3(0, 25, 0), Vec3(0, -10, 0), inTriangle); // Edge: Vec3(50, 25, 0), Vec3(-50, 25, 0)
		sTestCastSphereVertexOrEdge(sphere, Vec3(-25, 0, 0), Vec3(10, 10, 0), inTriangle); // Edge: Vec3(-50, 25, 0), Vec3(0,-25, 0)
		sTestCastSphereVertexOrEdge(sphere, Vec3(25, 0, 0), Vec3(-10, 10, 0), inTriangle); // Edge: Float3(0,-25, 0), Float3(50, 25, 0)
	}

	TEST_CASE("TestCastSphereTriangle")
	{
		// Create triangle
		Ref<Shape> triangle = TriangleShapeSettings(Vec3(50, 25, 0), Vec3(-50, 25, 0), Vec3(0,-25, 0)).Create().Get();
		sTestCastSphereTriangle(triangle);

		// Create a triangle mesh shape
		Ref<Shape> triangle_mesh = MeshShapeSettings({ Triangle(Float3(50, 25, 0), Float3(-50, 25, 0), Float3(0,-25, 0)) }).Create().Get();
		sTestCastSphereTriangle(triangle_mesh);
	}

	// Test CastShape for a (scaled) sphere vs box
	TEST_CASE("TestCastShapeSphereVsBox")
	{
		PhysicsTestContext c;

		// Create box to collide against (shape 2)
		// The box is scaled up by a factor 10 in the X axis and then rotated so that the X axis is up
		BoxShapeSettings box(Vec3::sReplicate(1.0f));
		box.SetEmbedded();
		ScaledShapeSettings scaled_box(&box, Vec3(10, 1, 1));
		scaled_box.SetEmbedded();
		Body &body2 = c.CreateBody(&scaled_box, Vec3(0, 1, 0), Quat::sRotation(Vec3::sAxisZ(), 0.5f * JPH_PI), EMotionType::Static, EMotionQuality::Discrete, Layers::NON_MOVING, EActivation::DontActivate);

		// Set settings
		ShapeCastSettings settings;
		settings.mReturnDeepestPoint = true;
		settings.mBackFaceModeTriangles = EBackFaceMode::CollideWithBackFaces;
		settings.mBackFaceModeConvex = EBackFaceMode::CollideWithBackFaces;

		{
			// Create shape cast
			Ref<Shape> normal_sphere = new SphereShape(1.0f);
			ShapeCast shape_cast { normal_sphere, Vec3::sReplicate(1.0f), Mat44::sTranslation(Vec3(0, 11, 0)), Vec3(0, 1, 0) };

			// Shape is intersecting at the start
			AllHitCollisionCollector<CastShapeCollector> collector;
			c.GetSystem()->GetNarrowPhaseQuery().CastShape(shape_cast, settings, collector);
			CHECK(collector.mHits.size() == 1);
			const ShapeCastResult &result = collector.mHits.front();
			CHECK(result.mBodyID2 == body2.GetID());
			CHECK_APPROX_EQUAL(result.mFraction, 0.0f);
			CHECK_APPROX_EQUAL(result.mPenetrationAxis.Normalized(), Vec3(0, -1, 0), 1.0e-3f);
			CHECK_APPROX_EQUAL(result.mPenetrationDepth, 1.0f, 1.0e-5f);
			CHECK_APPROX_EQUAL(result.mContactPointOn1, Vec3(0, 10, 0), 1.0e-3f);
			CHECK_APPROX_EQUAL(result.mContactPointOn2, Vec3(0, 11, 0), 1.0e-3f);
			CHECK(!result.mIsBackFaceHit);
		}

		{
			// This repeats the same test as above but uses scaling at all levels and validate that the penetration depth is still correct
			Ref<Shape> scaled_sphere = new ScaledShape(new SphereShape(0.1f), Vec3::sReplicate(5.0f));
			ShapeCast shape_cast { scaled_sphere, Vec3::sReplicate(2.0f), Mat44::sTranslation(Vec3(0, 11, 0)), Vec3(0, 1, 0) };

			// Shape is intersecting at the start
			AllHitCollisionCollector<CastShapeCollector> collector;
			c.GetSystem()->GetNarrowPhaseQuery().CastShape(shape_cast, settings, collector);
			CHECK(collector.mHits.size() == 1);
			const ShapeCastResult &result = collector.mHits.front();
			CHECK(result.mBodyID2 == body2.GetID());
			CHECK_APPROX_EQUAL(result.mFraction, 0.0f);
			CHECK_APPROX_EQUAL(result.mPenetrationAxis.Normalized(), Vec3(0, -1, 0), 1.0e-3f);
			CHECK_APPROX_EQUAL(result.mPenetrationDepth, 1.0f, 1.0e-5f);
			CHECK_APPROX_EQUAL(result.mContactPointOn1, Vec3(0, 10, 0), 1.0e-3f);
			CHECK_APPROX_EQUAL(result.mContactPointOn2, Vec3(0, 11, 0), 1.0e-3f);
			CHECK(!result.mIsBackFaceHit);
		}
	}

	// Test CastShape ordering according to penetration depth
	TEST_CASE("TestCastShapePenetrationDepthOrdering")
	{
		PhysicsTestContext c;

		// Create box to collide against (shape 2)
		BoxShapeSettings box(Vec3(0.1f, 2.0f, 2.0f));
		box.SetEmbedded();

		// Create 10 boxes that are 0.2 thick in the X axis and 4 in Y and Z, put them all next to each other on the X axis starting from X = 0 going to X = 2
		vector<Body *> bodies;
		for (int i = 0; i < 10; ++i)
			bodies.push_back(&c.CreateBody(&box, Vec3(0.1f + 0.2f * i, 0, 0), Quat::sIdentity(), EMotionType::Static, EMotionQuality::Discrete, Layers::NON_MOVING, EActivation::DontActivate));

		// Set settings
		ShapeCastSettings settings;
		settings.mReturnDeepestPoint = true;
		settings.mBackFaceModeTriangles = EBackFaceMode::CollideWithBackFaces;
		settings.mBackFaceModeConvex = EBackFaceMode::CollideWithBackFaces;
		settings.mCollisionTolerance = 1.0e-5f; // Increased precision
		settings.mPenetrationTolerance = 1.0e-5f;

		{
			// Create shape cast in X from -5 to 5
			RefConst<Shape> sphere = new SphereShape(1.0f);
			ShapeCast shape_cast { sphere, Vec3::sReplicate(1.0f), Mat44::sTranslation(Vec3(-5, 0, 0)), Vec3(10, 0, 0) };

			// We should hit the first body
			ClosestHitCollisionCollector<CastShapeCollector> collector;
			c.GetSystem()->GetNarrowPhaseQuery().CastShape(shape_cast, settings, collector);
			CHECK(collector.HadHit());
			CHECK(collector.mHit.mBodyID2 == bodies.front()->GetID());
			CHECK_APPROX_EQUAL(collector.mHit.mFraction, 4.0f / 10.0f);
			CHECK_APPROX_EQUAL(collector.mHit.mPenetrationAxis.Normalized(), Vec3(1, 0, 0), 2.0e-3f);
			CHECK_APPROX_EQUAL(collector.mHit.mPenetrationDepth, 0.0f);
			CHECK_APPROX_EQUAL(collector.mHit.mContactPointOn1, Vec3(0, 0, 0), 1.0e-4f);
			CHECK_APPROX_EQUAL(collector.mHit.mContactPointOn2, Vec3(0, 0, 0), 1.0e-4f);
			CHECK(!collector.mHit.mIsBackFaceHit);
		}

		{
			// Create shape cast in X from 5 to -5
			RefConst<Shape> sphere = new SphereShape(1.0f);
			ShapeCast shape_cast { sphere, Vec3::sReplicate(1.0f), Mat44::sTranslation(Vec3(5, 0, 0)), Vec3(-10, 0, 0) };

			// We should hit the last body
			ClosestHitCollisionCollector<CastShapeCollector> collector;
			c.GetSystem()->GetNarrowPhaseQuery().CastShape(shape_cast, settings, collector);
			CHECK(collector.HadHit());
			CHECK(collector.mHit.mBodyID2 == bodies.back()->GetID());
			CHECK_APPROX_EQUAL(collector.mHit.mFraction, 2.0f / 10.0f, 1.0e-4f);
			CHECK_APPROX_EQUAL(collector.mHit.mPenetrationAxis.Normalized(), Vec3(-1, 0, 0), 2.0e-3f);
			CHECK_APPROX_EQUAL(collector.mHit.mPenetrationDepth, 0.0f);
			CHECK_APPROX_EQUAL(collector.mHit.mContactPointOn1, Vec3(2, 0, 0), 4.0e-4f);
			CHECK_APPROX_EQUAL(collector.mHit.mContactPointOn2, Vec3(2, 0, 0), 4.0e-4f);
			CHECK(!collector.mHit.mIsBackFaceHit);
		}

		{
			// Create shape cast in X from 1.05 to 11, this should intersect with all bodies and have deepest penetration in bodies[5]
			RefConst<Shape> sphere = new SphereShape(1.0f);
			ShapeCast shape_cast { sphere, Vec3::sReplicate(1.0f), Mat44::sTranslation(Vec3(1.05f, 0, 0)), Vec3(10, 0, 0) };

			// We should hit bodies[5]
			AllHitCollisionCollector<CastShapeCollector> collector;
			c.GetSystem()->GetNarrowPhaseQuery().CastShape(shape_cast, settings, collector);
			collector.Sort();
			CHECK(collector.mHits.size() == 10);
			const ShapeCastResult &result = collector.mHits.front();
			CHECK(result.mBodyID2 == bodies[5]->GetID());
			CHECK_APPROX_EQUAL(result.mFraction, 0.0f);
			CHECK_APPROX_EQUAL(result.mPenetrationAxis.Normalized(), Vec3(1, 0, 0), 1.0e-3f);
			CHECK_APPROX_EQUAL(result.mPenetrationDepth, 1.05f);
			CHECK_APPROX_EQUAL(result.mContactPointOn1, Vec3(2.05f, 0, 0), 1.0e-5f); // Box starts at 1.0, center of sphere adds 0.05, radius of sphere is 1
			CHECK_APPROX_EQUAL(result.mContactPointOn2, Vec3(1.0f, 0, 0), 1.0e-5f); // Box starts at 1.0
			CHECK(!result.mIsBackFaceHit);
		}
	}
}
