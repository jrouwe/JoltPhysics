// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include <Jolt/Physics/Collision/ShapeCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/TriangleShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/ShapeFilter.h>
#include <Jolt/Physics/Collision/CollisionDispatch.h>
#include <Jolt/Physics/Collision/CastSphereVsTriangles.h>
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

	/// Helper function that tests a sphere against a triangle centered on the origin with normal Z
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
		Body &body2 = c.CreateBody(&scaled_box, RVec3(0, 1, 0), Quat::sRotation(Vec3::sAxisZ(), 0.5f * JPH_PI), EMotionType::Static, EMotionQuality::Discrete, Layers::NON_MOVING, EActivation::DontActivate);

		// Set settings
		ShapeCastSettings settings;
		settings.mReturnDeepestPoint = true;
		settings.mBackFaceModeTriangles = EBackFaceMode::CollideWithBackFaces;
		settings.mBackFaceModeConvex = EBackFaceMode::CollideWithBackFaces;

		{
			// Create shape cast
			Ref<Shape> normal_sphere = new SphereShape(1.0f);
			RShapeCast shape_cast { normal_sphere, Vec3::sReplicate(1.0f), RMat44::sTranslation(RVec3(0, 11, 0)), Vec3(0, 1, 0) };

			// Shape is intersecting at the start
			AllHitCollisionCollector<CastShapeCollector> collector;
			c.GetSystem()->GetNarrowPhaseQuery().CastShape(shape_cast, settings, RVec3::sZero(), collector);
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
			RShapeCast shape_cast { scaled_sphere, Vec3::sReplicate(2.0f), RMat44::sTranslation(RVec3(0, 11, 0)), Vec3(0, 1, 0) };

			// Shape is intersecting at the start
			AllHitCollisionCollector<CastShapeCollector> collector;
			c.GetSystem()->GetNarrowPhaseQuery().CastShape(shape_cast, settings, RVec3::sZero(), collector);
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
		Array<Body *> bodies;
		for (int i = 0; i < 10; ++i)
			bodies.push_back(&c.CreateBody(&box, RVec3(0.1f + 0.2f * i, 0, 0), Quat::sIdentity(), EMotionType::Static, EMotionQuality::Discrete, Layers::NON_MOVING, EActivation::DontActivate));

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
			RShapeCast shape_cast { sphere, Vec3::sReplicate(1.0f), RMat44::sTranslation(RVec3(-5, 0, 0)), Vec3(10, 0, 0) };

			// We should hit the first body
			ClosestHitCollisionCollector<CastShapeCollector> collector;
			c.GetSystem()->GetNarrowPhaseQuery().CastShape(shape_cast, settings, RVec3::sZero(), collector);
			CHECK(collector.HadHit());
			CHECK(collector.mHit.mBodyID2 == bodies.front()->GetID());
			CHECK_APPROX_EQUAL(collector.mHit.mFraction, 4.0f / 10.0f);
			CHECK(collector.mHit.mPenetrationAxis.Normalized().Dot(Vec3(1, 0, 0)) > Cos(DegreesToRadians(1.0f)));
			CHECK_APPROX_EQUAL(collector.mHit.mPenetrationDepth, 0.0f);
			CHECK_APPROX_EQUAL(collector.mHit.mContactPointOn1, Vec3(0, 0, 0), 2.0e-3f);
			CHECK_APPROX_EQUAL(collector.mHit.mContactPointOn2, Vec3(0, 0, 0), 2.0e-3f);
			CHECK(!collector.mHit.mIsBackFaceHit);
		}

		{
			// Create shape cast in X from 5 to -5
			RefConst<Shape> sphere = new SphereShape(1.0f);
			RShapeCast shape_cast { sphere, Vec3::sReplicate(1.0f), RMat44::sTranslation(RVec3(5, 0, 0)), Vec3(-10, 0, 0) };

			// We should hit the last body
			ClosestHitCollisionCollector<CastShapeCollector> collector;
			c.GetSystem()->GetNarrowPhaseQuery().CastShape(shape_cast, settings, RVec3::sZero(), collector);
			CHECK(collector.HadHit());
			CHECK(collector.mHit.mBodyID2 == bodies.back()->GetID());
			CHECK_APPROX_EQUAL(collector.mHit.mFraction, 2.0f / 10.0f, 1.0e-4f);
			CHECK(collector.mHit.mPenetrationAxis.Normalized().Dot(Vec3(-1, 0, 0)) > Cos(DegreesToRadians(1.0f)));
			CHECK_APPROX_EQUAL(collector.mHit.mPenetrationDepth, 0.0f);
			CHECK_APPROX_EQUAL(collector.mHit.mContactPointOn1, Vec3(2, 0, 0), 4.0e-4f);
			CHECK_APPROX_EQUAL(collector.mHit.mContactPointOn2, Vec3(2, 0, 0), 4.0e-4f);
			CHECK(!collector.mHit.mIsBackFaceHit);
		}

		{
			// Create shape cast in X from 1.05 to 11, this should intersect with all bodies and have deepest penetration in bodies[5]
			RefConst<Shape> sphere = new SphereShape(1.0f);
			RShapeCast shape_cast { sphere, Vec3::sReplicate(1.0f), RMat44::sTranslation(RVec3(1.05_r, 0, 0)), Vec3(10, 0, 0) };

			// We should hit bodies[5]
			AllHitCollisionCollector<CastShapeCollector> collector;
			c.GetSystem()->GetNarrowPhaseQuery().CastShape(shape_cast, settings, RVec3::sZero(), collector);
			collector.Sort();
			CHECK(collector.mHits.size() == 10);
			const ShapeCastResult &result = collector.mHits.front();
			CHECK(result.mBodyID2 == bodies[5]->GetID());
			CHECK_APPROX_EQUAL(result.mFraction, 0.0f);
			CHECK(result.mPenetrationAxis.Normalized().Dot(Vec3(1, 0, 0)) > Cos(DegreesToRadians(1.0f)));
			CHECK_APPROX_EQUAL(result.mPenetrationDepth, 1.05f);
			CHECK_APPROX_EQUAL(result.mContactPointOn1, Vec3(2.05f, 0, 0), 2.0e-5f); // Box starts at 1.0, center of sphere adds 0.05, radius of sphere is 1
			CHECK_APPROX_EQUAL(result.mContactPointOn2, Vec3(1.0f, 0, 0), 2.0e-5f); // Box starts at 1.0
			CHECK(!result.mIsBackFaceHit);
		}
	}

	// Test casting a capsule against a mesh that is intersecting at fraction 0 and test that it returns the deepest penetration
	TEST_CASE("TestDeepestPenetrationAtFraction0")
	{
		// Create an n x n grid of triangles
		const int n = 10;
		const float s = 0.1f;
		TriangleList triangles;
		for (int z = 0; z < n; ++z)
			for (int x = 0; x < n; ++x)
			{
				float fx = s * x - s * n / 2, fz = s * z - s * n / 2;
				triangles.push_back(Triangle(Vec3(fx, 0, fz), Vec3(fx, 0, fz + s), Vec3(fx + s, 0, fz + s)));
				triangles.push_back(Triangle(Vec3(fx, 0, fz), Vec3(fx + s, 0, fz + s), Vec3(fx + s, 0, fz)));
			}
		MeshShapeSettings mesh_settings(triangles);
		mesh_settings.SetEmbedded();

		// Create a compound shape with two copies of the mesh
		StaticCompoundShapeSettings compound_settings;
		compound_settings.AddShape(Vec3::sZero(), Quat::sIdentity(), &mesh_settings);
		compound_settings.AddShape(Vec3(0, -0.01f, 0), Quat::sIdentity(), &mesh_settings); // This will not result in the deepest penetration
		compound_settings.SetEmbedded();

		// Add it to the scene
		PhysicsTestContext c;
		c.CreateBody(&compound_settings, RVec3::sZero(), Quat::sIdentity(), EMotionType::Static, EMotionQuality::Discrete, Layers::NON_MOVING, EActivation::DontActivate);

		// Add the same compound a little bit lower (this will not result in the deepest penetration)
		c.CreateBody(&compound_settings, RVec3(0, -0.1_r, 0), Quat::sIdentity(), EMotionType::Static, EMotionQuality::Discrete, Layers::NON_MOVING, EActivation::DontActivate);

		// We want the deepest hit
		ShapeCastSettings cast_settings;
		cast_settings.mReturnDeepestPoint = true;

		// Create capsule to test
		const float capsule_half_height = 2.0f;
		const float capsule_radius = 1.0f;
		RefConst<Shape> cast_shape = new CapsuleShape(capsule_half_height, capsule_radius);

		// Cast the shape starting inside the mesh with a long distance so that internally in the mesh shape the RayAABox4 test will return a low negative fraction.
		// This used to be confused with the penetration depth and would cause an early out and return the wrong result.
		const float capsule_offset = 0.1f;
		RShapeCast shape_cast(cast_shape, Vec3::sReplicate(1.0f), RMat44::sTranslation(RVec3(0, capsule_half_height + capsule_offset, 0)), Vec3(0, -100, 0));

		// Cast first using the closest hit collector
		ClosestHitCollisionCollector<CastShapeCollector> collector;
		c.GetSystem()->GetNarrowPhaseQuery().CastShape(shape_cast, cast_settings, RVec3::sZero(), collector);

		// Check that it indeed found a hit at fraction 0 with the deepest penetration of all triangles
		CHECK(collector.HadHit());
		CHECK(collector.mHit.mFraction == 0.0f);
		CHECK_APPROX_EQUAL(collector.mHit.mPenetrationDepth, capsule_radius - capsule_offset, 1.0e-4f);
		CHECK_APPROX_EQUAL(collector.mHit.mPenetrationAxis.Normalized(), Vec3(0, -1, 0));
		CHECK_APPROX_EQUAL(collector.mHit.mContactPointOn2, Vec3::sZero());

		// Cast again while triggering a force early out after the first hit
		class MyCollector : public CastShapeCollector
		{
		public:
			virtual void	AddHit(const ShapeCastResult &inResult) override
			{
				++mNumHits;
				ForceEarlyOut();
			}

			int				mNumHits = 0;
		};
		MyCollector collector2;
		c.GetSystem()->GetNarrowPhaseQuery().CastShape(shape_cast, cast_settings, RVec3::sZero(), collector2);

		// Ensure that we indeed stopped after the first hit
		CHECK(collector2.mNumHits == 1);
	}

	// Test a problem case where a sphere cast would incorrectly hit a degenerate triangle (see: https://github.com/jrouwe/JoltPhysics/issues/886)
	TEST_CASE("TestCastSphereVsDegenerateTriangle")
	{
		AllHitCollisionCollector<CastShapeCollector> collector;
		SphereShape sphere(0.2f);
		sphere.SetEmbedded();
		ShapeCast cast(&sphere, Vec3::sReplicate(1.0f), Mat44::sTranslation(Vec3(14.8314590f, 8.19055080f, -4.30825043f)), Vec3(-0.0988006592f, 5.96046448e-08f, 0.000732421875f));
		ShapeCastSettings settings;
		CastSphereVsTriangles caster(cast, settings, Vec3::sReplicate(1.0f), Mat44::sIdentity(), { }, collector);
		caster.Cast(Vec3(14.5536213f, 10.5973721f, -0.00600051880f), Vec3(14.5536213f, 10.5969315f, -3.18638134f), Vec3(14.5536213f, 10.5969315f, -5.18637228f), 0b111, SubShapeID());
		CHECK(!collector.HadHit());
	}
}
