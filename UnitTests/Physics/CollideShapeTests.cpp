// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include "PhysicsTestContext.h"
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/Shape/TriangleShape.h>
#include <Jolt/Physics/Collision/Shape/CylinderShape.h>
#include <Jolt/Physics/Collision/CollideShape.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Jolt/Physics/Collision/CollisionDispatch.h>
#include <Jolt/Physics/Collision/CollideConvexVsTriangles.h>
#include <Jolt/Geometry/EPAPenetrationDepth.h>
#include "Layers.h"

TEST_SUITE("CollideShapeTests")
{
	// Compares CollideShapeResult for two spheres with given positions and radii
	static void sCompareCollideShapeResultSphere(Vec3Arg inPosition1, float inRadius1, Vec3Arg inPosition2, float inRadius2, const CollideShapeResult &inResult)
	{
		// Test if spheres overlap
		Vec3 delta = inPosition2 - inPosition1;
		float len = delta.Length();
		CHECK(len > 0.0f);
		CHECK(len <= inRadius1 + inRadius2);

		// Calculate points on surface + vector that will push 2 out of collision
		Vec3 expected_point1 = inPosition1 + delta * (inRadius1 / len);
		Vec3 expected_point2 = inPosition2 - delta * (inRadius2 / len);
		Vec3 expected_penetration_axis = delta / len;

		// Get actual results
		Vec3 penetration_axis = inResult.mPenetrationAxis.Normalized();

		// Compare
		CHECK_APPROX_EQUAL(expected_point1, inResult.mContactPointOn1);
		CHECK_APPROX_EQUAL(expected_point2, inResult.mContactPointOn2);
		CHECK_APPROX_EQUAL(expected_penetration_axis, penetration_axis);
	}

	// Test CollideShape function for spheres
	TEST_CASE("TestCollideShapeSphere")
	{
		// Locations of test sphere
		static const RVec3 cPosition1A(10.0f, 11.0f, 12.0f);
		static const RVec3 cPosition1B(10.0f, 21.0f, 12.0f);
		static const float cRadius1 = 2.0f;

		// Locations of sphere in the physics system
		static const RVec3 cPosition2A(13.0f, 11.0f, 12.0f);
		static const RVec3 cPosition2B(13.0f, 22.0f, 12.0f);
		static const float cRadius2 = 1.5f;

		// Create sphere to test with (shape 1)
		Ref<Shape> shape1 = new SphereShape(cRadius1);
		Mat44 shape1_com = Mat44::sTranslation(shape1->GetCenterOfMass());
		RMat44 shape1_transform = RMat44::sTranslation(cPosition1A) * Mat44::sRotationX(0.1f * JPH_PI) * shape1_com;

		// Create sphere to collide against (shape 2)
		PhysicsTestContext c;
		Body &body2 = c.CreateSphere(cPosition2A, cRadius2, EMotionType::Static, EMotionQuality::Discrete, Layers::NON_MOVING);

		// Filters
		SpecifiedBroadPhaseLayerFilter broadphase_moving_filter(BroadPhaseLayers::MOVING);
		SpecifiedBroadPhaseLayerFilter broadphase_non_moving_filter(BroadPhaseLayers::NON_MOVING);
		SpecifiedObjectLayerFilter object_moving_filter(Layers::MOVING);
		SpecifiedObjectLayerFilter object_non_moving_filter(Layers::NON_MOVING);

		// Collector that fails the test
		class FailCollideShapeCollector : public CollideShapeCollector
		{
		public:
			virtual void	AddHit(const CollideShapeResult &inResult) override
			{
				FAIL("Callback should not be called");
			}
		};
		FailCollideShapeCollector fail_collector;

		// Set settings
		CollideShapeSettings settings;
		settings.mActiveEdgeMode = EActiveEdgeMode::CollideWithAll;
		settings.mBackFaceMode = EBackFaceMode::CollideWithBackFaces;

		// Test against wrong layer
		c.GetSystem()->GetNarrowPhaseQuery().CollideShape(shape1, Vec3::sOne(), shape1_transform, settings, RVec3::sZero(), fail_collector, broadphase_moving_filter, object_moving_filter);

		// Collector that tests that collision happens at position A
		class PositionACollideShapeCollector : public CollideShapeCollector
		{
		public:
							PositionACollideShapeCollector(const Body &inBody2) :
				mBody2(inBody2)
			{
			}

			virtual void	AddHit(const CollideShapeResult &inResult) override
			{
				CHECK(mBody2.GetID() == GetContext()->mBodyID);
				sCompareCollideShapeResultSphere(Vec3(cPosition1A), cRadius1, Vec3(cPosition2A), cRadius2, inResult);
				mWasHit = true;
			}

			bool			mWasHit = false;

		private:
			const Body &	mBody2;
		};
		PositionACollideShapeCollector position_a_collector(body2);

		// Test collision against correct layer
		CHECK(!position_a_collector.mWasHit);
		c.GetSystem()->GetNarrowPhaseQuery().CollideShape(shape1, Vec3::sOne(), shape1_transform, settings, RVec3::sZero(), position_a_collector, broadphase_non_moving_filter, object_non_moving_filter);
		CHECK(position_a_collector.mWasHit);

		// Now move body to position B
		c.GetSystem()->GetBodyInterface().SetPositionAndRotation(body2.GetID(), cPosition2B, Quat::sRotation(Vec3::sAxisY(), 0.2f * JPH_PI), EActivation::DontActivate);

		// Test that original position doesn't collide anymore
		c.GetSystem()->GetNarrowPhaseQuery().CollideShape(shape1, Vec3::sOne(), shape1_transform, settings, RVec3::sZero(), fail_collector, broadphase_non_moving_filter, object_non_moving_filter);

		// Move test shape to position B
		shape1_transform = RMat44::sTranslation(cPosition1B) * Mat44::sRotationZ(0.3f * JPH_PI) * shape1_com;

		// Test against wrong layer
		c.GetSystem()->GetNarrowPhaseQuery().CollideShape(shape1, Vec3::sOne(), shape1_transform, settings, RVec3::sZero(), fail_collector, broadphase_moving_filter, object_moving_filter);

		// Callback that tests that collision happens at position B
		class PositionBCollideShapeCollector : public CollideShapeCollector
		{
		public:
							PositionBCollideShapeCollector(const Body &inBody2) :
				mBody2(inBody2)
			{
			}

			virtual void	Reset() override
			{
				CollideShapeCollector::Reset();

				mWasHit = false;
			}

			virtual void	AddHit(const CollideShapeResult &inResult) override
			{
				CHECK(mBody2.GetID() == GetContext()->mBodyID);
				sCompareCollideShapeResultSphere(Vec3(cPosition1B), cRadius1, Vec3(cPosition2B), cRadius2, inResult);
				mWasHit = true;
			}

			bool			mWasHit = false;

		private:
			const Body &	mBody2;
		};
		PositionBCollideShapeCollector position_b_collector(body2);

		// Test collision
		CHECK(!position_b_collector.mWasHit);
		c.GetSystem()->GetNarrowPhaseQuery().CollideShape(shape1, Vec3::sOne(), shape1_transform, settings, RVec3::sZero(), position_b_collector, broadphase_non_moving_filter, object_non_moving_filter);
		CHECK(position_b_collector.mWasHit);

		// Update the physics system (optimizes the broadphase)
		c.Simulate(c.GetDeltaTime());

		// Test against wrong layer
		c.GetSystem()->GetNarrowPhaseQuery().CollideShape(shape1, Vec3::sOne(), shape1_transform, settings, RVec3::sZero(), fail_collector, broadphase_moving_filter, object_moving_filter);

		// Test collision again
		position_b_collector.Reset();
		CHECK(!position_b_collector.mWasHit);
		c.GetSystem()->GetNarrowPhaseQuery().CollideShape(shape1, Vec3::sOne(), shape1_transform, settings, RVec3::sZero(), position_b_collector, broadphase_non_moving_filter, object_non_moving_filter);
		CHECK(position_b_collector.mWasHit);
	}

	// Test CollideShape function for a (scaled) sphere vs box
	TEST_CASE("TestCollideShapeSphereVsBox")
	{
		PhysicsTestContext c;

		// Create box to collide against (shape 2)
		// The box is scaled up by a factor 10 in the X axis and then rotated so that the X axis is up
		BoxShapeSettings box(Vec3::sOne());
		box.SetEmbedded();
		ScaledShapeSettings scaled_box(&box, Vec3(10, 1, 1));
		scaled_box.SetEmbedded();
		Body &body2 = c.CreateBody(&scaled_box, RVec3(0, 1, 0), Quat::sRotation(Vec3::sAxisZ(), 0.5f * JPH_PI), EMotionType::Static, EMotionQuality::Discrete, Layers::NON_MOVING, EActivation::DontActivate);

		// Set settings
		CollideShapeSettings settings;
		settings.mActiveEdgeMode = EActiveEdgeMode::CollideWithAll;
		settings.mBackFaceMode = EBackFaceMode::CollideWithBackFaces;

		{
			// Create sphere
			Ref<Shape> normal_sphere = new SphereShape(1.0f);

			// Collect hit with normal sphere
			AllHitCollisionCollector<CollideShapeCollector> collector;
			c.GetSystem()->GetNarrowPhaseQuery().CollideShape(normal_sphere, Vec3::sOne(), RMat44::sTranslation(RVec3(0, 11, 0)), settings, RVec3::sZero(), collector);
			CHECK(collector.mHits.size() == 1);
			const CollideShapeResult &result = collector.mHits.front();
			CHECK(result.mBodyID2 == body2.GetID());
			CHECK_APPROX_EQUAL(result.mContactPointOn1, Vec3(0, 10, 0), 1.0e-4f);
			CHECK_APPROX_EQUAL(result.mContactPointOn2, Vec3(0, 11, 0), 1.0e-4f);
			Vec3 pen_axis = result.mPenetrationAxis.Normalized();
			CHECK_APPROX_EQUAL(pen_axis, Vec3(0, -1, 0), 1.0e-4f);
			CHECK_APPROX_EQUAL(result.mPenetrationDepth, 1.0f, 1.0e-5f);
		}

		{
			// This repeats the same test as above but uses scaling at all levels
			Ref<Shape> scaled_sphere = new ScaledShape(new SphereShape(0.1f), Vec3::sReplicate(5.0f));

			// Collect hit with scaled sphere
			AllHitCollisionCollector<CollideShapeCollector> collector;
			c.GetSystem()->GetNarrowPhaseQuery().CollideShape(scaled_sphere, Vec3::sReplicate(2.0f), RMat44::sTranslation(RVec3(0, 11, 0)), settings, RVec3::sZero(), collector);
			CHECK(collector.mHits.size() == 1);
			const CollideShapeResult &result = collector.mHits.front();
			CHECK(result.mBodyID2 == body2.GetID());
			CHECK_APPROX_EQUAL(result.mContactPointOn1, Vec3(0, 10, 0), 1.0e-4f);
			CHECK_APPROX_EQUAL(result.mContactPointOn2, Vec3(0, 11, 0), 1.0e-4f);
			Vec3 pen_axis = result.mPenetrationAxis.Normalized();
			CHECK_APPROX_EQUAL(pen_axis, Vec3(0, -1, 0), 1.0e-4f);
			CHECK_APPROX_EQUAL(result.mPenetrationDepth, 1.0f, 1.0e-5f);
		}
	}

	// Test colliding a very long capsule vs a box that is intersecting with the line segment inside the capsule
	// This particular config reported the wrong penetration due to accuracy problems before
	TEST_CASE("TestCollideShapeLongCapsuleVsEmbeddedBox")
	{
		// Create box
		Vec3 box_min(-1.0f, -2.0f, 0.5f);
		Vec3 box_max(2.0f, -0.5f, 3.0f);
		Ref<RotatedTranslatedShapeSettings> box_settings = new RotatedTranslatedShapeSettings(0.5f * (box_min + box_max), Quat::sIdentity(), new BoxShapeSettings(0.5f * (box_max - box_min)));
		Ref<Shape> box_shape = box_settings->Create().Get();
		Mat44 box_transform(Vec4(0.516170502f, -0.803887904f, -0.295520246f, 0.0f), Vec4(0.815010250f, 0.354940295f, 0.458012700f, 0.0f), Vec4(-0.263298869f, -0.477264702f, 0.838386655f, 0.0f), Vec4(-10.2214508f, -18.6808319f, 40.7468987f, 1.0f));

		// Create capsule
		float capsule_half_height = 75.0f;
		float capsule_radius = 1.5f;
		Ref<RotatedTranslatedShapeSettings> capsule_settings = new RotatedTranslatedShapeSettings(Vec3(0, 0, 75), Quat(0.499999970f, -0.499999970f, -0.499999970f, 0.499999970f), new CapsuleShapeSettings(capsule_half_height, capsule_radius));
		Ref<Shape> capsule_shape = capsule_settings->Create().Get();
		Mat44 capsule_transform = Mat44::sTranslation(Vec3(-9.68538570f, -18.0328083f, 41.3212280f));

		// Collision settings
		CollideShapeSettings settings;
		settings.mActiveEdgeMode = EActiveEdgeMode::CollideWithAll;
		settings.mBackFaceMode = EBackFaceMode::CollideWithBackFaces;
		settings.mCollectFacesMode = ECollectFacesMode::NoFaces;

		// Collide the two shapes
		AllHitCollisionCollector<CollideShapeCollector> collector;
		CollisionDispatch::sCollideShapeVsShape(capsule_shape, box_shape, Vec3::sOne(), Vec3::sOne(), capsule_transform, box_transform, SubShapeIDCreator(), SubShapeIDCreator(), settings, collector);

		// Check that there was a hit
		CHECK(collector.mHits.size() == 1);
		const CollideShapeResult &result = collector.mHits.front();

		// Now move the box 1% further than the returned penetration depth and check that it is no longer in collision
		Vec3 distance_to_move_box = result.mPenetrationAxis.Normalized() * result.mPenetrationDepth;
		collector.Reset();
		CHECK(!collector.HadHit());
		CollisionDispatch::sCollideShapeVsShape(capsule_shape, box_shape, Vec3::sOne(), Vec3::sOne(), capsule_transform, Mat44::sTranslation(1.01f * distance_to_move_box) * box_transform, SubShapeIDCreator(), SubShapeIDCreator(), settings, collector);
		CHECK(!collector.HadHit());

		// Now check that moving 1% less than the penetration distance makes the shapes still overlap
		CollisionDispatch::sCollideShapeVsShape(capsule_shape, box_shape, Vec3::sOne(), Vec3::sOne(), capsule_transform, Mat44::sTranslation(0.99f * distance_to_move_box) * box_transform, SubShapeIDCreator(), SubShapeIDCreator(), settings, collector);
		CHECK(collector.mHits.size() == 1);
	}

	// Another test case found in practice of a very large oriented box (convex hull) vs a small triangle outside the hull. This should not report a collision
	TEST_CASE("TestCollideShapeSmallTriangleVsLargeBox")
	{
		// Triangle vertices
		Vec3 v0(-81.5637589f, -126.987244f, -146.771729f);
		Vec3 v1(-81.8749924f, -127.270691f, -146.544403f);
		Vec3 v2(-81.6972275f, -127.383545f, -146.773254f);

		// Oriented box vertices
		Array<Vec3> obox_points = {
			Vec3(125.932892f, -374.712250f, 364.192169f),
			Vec3(319.492218f, -73.2614441f, 475.009613f),
			Vec3(-122.277550f, -152.200287f, 192.441437f),
			Vec3(71.2817841f, 149.250519f, 303.258881f),
			Vec3(-77.8921967f, -359.410797f, 678.579712f),
			Vec3(115.667137f, -57.9600067f, 789.397095f),
			Vec3(-326.102631f, -136.898834f, 506.828949f),
			Vec3(-132.543304f, 164.551971f, 617.646362f)
		};
		ConvexHullShapeSettings hull_settings(obox_points, 0.0f);
		RefConst<ConvexShape> convex_hull = StaticCast<ConvexShape>(hull_settings.Create().Get());

		// Create triangle support function
		TriangleConvexSupport triangle(v0, v1, v2);

		// Create the convex hull support function
		ConvexShape::SupportBuffer buffer;
		const ConvexShape::Support *support = convex_hull->GetSupportFunction(ConvexShape::ESupportMode::IncludeConvexRadius, buffer, Vec3::sOne());

		// Triangle is close enough to make GJK report indeterminate
		Vec3 penetration_axis = Vec3::sAxisX(), point1, point2;
		EPAPenetrationDepth pen_depth;
		EPAPenetrationDepth::EStatus status = pen_depth.GetPenetrationDepthStepGJK(*support, support->GetConvexRadius(), triangle, 0.0f, cDefaultCollisionTolerance, penetration_axis, point1, point2);
		CHECK(status == EPAPenetrationDepth::EStatus::Indeterminate);

		// But there should not be an actual collision
		CHECK(!pen_depth.GetPenetrationDepthStepEPA(*support, triangle, cDefaultPenetrationTolerance, penetration_axis, point1, point2));
	}

	// A test case of a triangle that's nearly parallel to a capsule and penetrating it. This one was causing numerical issues.
	TEST_CASE("TestCollideParallelTriangleVsCapsule")
	{
		Vec3 v1(-0.479988575f, -1.36185002f, 0.269966960f);
		Vec3 v2(-0.104996204f, 0.388152480f, 0.269967079f);
		Vec3 v3(-0.104996204f, -1.36185002f, 0.269966960f);
		TriangleShape triangle(v1, v2, v3);
		triangle.SetEmbedded();

		float capsule_radius = 0.37f;
		float capsule_half_height = 0.5f;
		CapsuleShape capsule(capsule_half_height, capsule_radius);
		capsule.SetEmbedded();

		CollideShapeSettings settings;
		AllHitCollisionCollector<CollideShapeCollector> collector;
		CollisionDispatch::sCollideShapeVsShape(&triangle, &capsule, Vec3::sOne(), Vec3::sOne(), Mat44::sIdentity(), Mat44::sIdentity(), SubShapeIDCreator(), SubShapeIDCreator(), settings, collector);

		// The capsule's center is closest to the triangle's edge v2 v3
		Vec3 capsule_center_to_triangle_v2_v3 = v3;
		capsule_center_to_triangle_v2_v3.SetY(0); // The penetration axis will be in x, z only because the triangle is parallel to the capsule axis
		float capsule_center_to_triangle_v2_v3_len = capsule_center_to_triangle_v2_v3.Length();
		Vec3 expected_penetration_axis = -capsule_center_to_triangle_v2_v3 / capsule_center_to_triangle_v2_v3_len;
		float expected_penetration_depth = capsule_radius - capsule_center_to_triangle_v2_v3_len;

		CHECK(collector.mHits.size() == 1);
		const CollideShapeResult &hit = collector.mHits[0];
		Vec3 actual_penetration_axis = hit.mPenetrationAxis.Normalized();
		float actual_penetration_depth = hit.mPenetrationDepth;

		CHECK_APPROX_EQUAL(actual_penetration_axis, expected_penetration_axis);
		CHECK_APPROX_EQUAL(actual_penetration_depth, expected_penetration_depth);
	}

	// A test case of a triangle that's nearly parallel to a capsule and penetrating it. This one was causing numerical issues.
	TEST_CASE("TestCollideParallelTriangleVsCapsule2")
	{
		Vec3 v1(-0.0904417038f, -4.72410202f, 0.307858467f);
		Vec3 v2(-0.0904417038f, 5.27589798f, 0.307857513f);
		Vec3 v3(9.90955830f, 5.27589798f, 0.307864189f);
		TriangleShape triangle(v1, v2, v3);
		triangle.SetEmbedded();

		float capsule_radius = 0.42f;
		float capsule_half_height = 0.675f;
		CapsuleShape capsule(capsule_half_height, capsule_radius);
		capsule.SetEmbedded();

		CollideShapeSettings settings;
		AllHitCollisionCollector<CollideShapeCollector> collector;
		CollisionDispatch::sCollideShapeVsShape(&triangle, &capsule, Vec3::sOne(), Vec3::sOne(), Mat44::sIdentity(), Mat44::sIdentity(), SubShapeIDCreator(), SubShapeIDCreator(), settings, collector);

		// The capsule intersects with the triangle and the closest point is in the interior of the triangle
		Vec3 expected_penetration_axis = Vec3(0, 0, -1); // Triangle is in the XY plane so the normal is Z
		float expected_penetration_depth = capsule_radius - v1.GetZ();

		CHECK(collector.mHits.size() == 1);
		const CollideShapeResult &hit = collector.mHits[0];
		Vec3 actual_penetration_axis = hit.mPenetrationAxis.Normalized();
		float actual_penetration_depth = hit.mPenetrationDepth;

		CHECK_APPROX_EQUAL(actual_penetration_axis, expected_penetration_axis);
		CHECK_APPROX_EQUAL(actual_penetration_depth, expected_penetration_depth);
	}

	// A test case of a triangle that's nearly parallel to a capsule and almost penetrating it. This one was causing numerical issues.
	TEST_CASE("TestCollideParallelTriangleVsCapsule3")
	{
		Vec3 v1(-0.474807739f, 17.2921791f, 0.212532043f);
		Vec3 v2(-0.474807739f, -2.70782185f, 0.212535858f);
		Vec3 v3(-0.857490540f, -2.70782185f, -0.711341858f);
		TriangleShape triangle(v1, v2, v3);
		triangle.SetEmbedded();

		float capsule_radius = 0.5f;
		float capsule_half_height = 0.649999976f;
		CapsuleShape capsule(capsule_half_height, capsule_radius);
		capsule.SetEmbedded();

		CollideShapeSettings settings;
		settings.mMaxSeparationDistance = 0.120000005f;
		ClosestHitCollisionCollector<CollideShapeCollector> collector;
		CollisionDispatch::sCollideShapeVsShape(&capsule, &triangle, Vec3::sOne(), Vec3::sOne(), Mat44::sIdentity(), Mat44::sIdentity(), SubShapeIDCreator(), SubShapeIDCreator(), settings, collector);

		CHECK(collector.HadHit());
		Vec3 expected_normal = (v2 - v1).Cross(v3 - v1).Normalized();
		Vec3 actual_normal = -collector.mHit.mPenetrationAxis.Normalized();
		CHECK_APPROX_EQUAL(actual_normal, expected_normal, 1.0e-6f);
		float expected_penetration_depth = capsule.GetRadius() + v1.Dot(expected_normal);
		CHECK_APPROX_EQUAL(collector.mHit.mPenetrationDepth, expected_penetration_depth, 1.0e-6f);
	}

	// A test case of a triangle that's nearly parallel to a cylinder and is just penetrating it. This one was causing numerical issues. See issue #1008.
	TEST_CASE("TestCollideParallelTriangleVsCylinder")
	{
		CylinderShape cylinder(0.85f, 0.25f, 0.02f);
		cylinder.SetEmbedded();

		Mat44 cylinder_transform = Mat44::sTranslation(Vec3(-42.8155518f, -4.32299995f, 12.1734285f));

		CollideShapeSettings settings;
		settings.mMaxSeparationDistance = 0.001f;
		ClosestHitCollisionCollector<CollideShapeCollector> collector;
		CollideConvexVsTriangles c(&cylinder, Vec3::sOne(), Vec3::sOne(), cylinder_transform, Mat44::sIdentity(), SubShapeID(), settings, collector);

		Vec3 v0(-42.7954292f, -0.647318780f, 12.4227943f);
		Vec3 v1(-29.9111290f, -0.647318780f, 12.4227943f);
		Vec3 v2(-42.7954292f, -4.86970234f, 12.4227943f);
		c.Collide(v0, v1, v2, 0, SubShapeID());

		// Check there was a hit
		CHECK(collector.HadHit());
		CHECK(collector.mHit.mPenetrationDepth < 1.0e-4f);
		CHECK(collector.mHit.mPenetrationAxis.Normalized().IsClose(Vec3::sAxisZ()));
	}

	// A test case of a box and a convex hull that are nearly touching and that should return a contact with correct normal because the collision settings specify a max separation distance. This was producing the wrong normal.
	TEST_CASE("BoxVsConvexHullNoConvexRadius")
	{
		const float separation_distance = 0.001f;
		const float box_separation_from_hull = 0.5f * separation_distance;
		const float hull_height = 0.25f;

		// Box with no convex radius
		Ref<BoxShapeSettings> box_settings = new BoxShapeSettings(Vec3(0.25f, 0.75f, 0.375f), 0.0f);
		Ref<Shape> box_shape = box_settings->Create().Get();

		// Convex hull (also a box) with no convex radius
		Vec3 hull_points[] =
		{
			Vec3(-2.5f, -hull_height, -1.5f),
			Vec3(-2.5f, hull_height, -1.5f),
			Vec3(2.5f, -hull_height, -1.5f),
			Vec3(-2.5f, -hull_height, 1.5f),
			Vec3(-2.5f, hull_height, 1.5f),
			Vec3(2.5f, hull_height, -1.5f),
			Vec3(2.5f, -hull_height, 1.5f),
			Vec3(2.5f, hull_height, 1.5f)
		};
		Ref<ConvexHullShapeSettings> hull_settings = new ConvexHullShapeSettings(hull_points, 8, 0.0f);
		Ref<Shape> hull_shape = hull_settings->Create().Get();

		float angle = 0.0f;
		for (int i = 0; i < 481; ++i)
		{
			// Slowly rotate both box and convex hull
			angle += DegreesToRadians(45.0f) / 60.0f;
			Mat44 hull_transform = Mat44::sRotationY(angle);
			const Mat44 box_local_translation = Mat44::sTranslation(Vec3(0.1f, 1.0f + box_separation_from_hull, -0.5f));
			const Mat44 box_local_rotation = Mat44::sRotationY(DegreesToRadians(-45.0f));
			const Mat44 box_local_transform = box_local_translation * box_local_rotation;
			const Mat44 box_transform = hull_transform * box_local_transform;

			CollideShapeSettings settings;
			settings.mMaxSeparationDistance = separation_distance;
			ClosestHitCollisionCollector<CollideShapeCollector> collector;
			CollisionDispatch::sCollideShapeVsShape(box_shape, hull_shape, Vec3::sOne(), Vec3::sOne(), box_transform, hull_transform, SubShapeIDCreator(), SubShapeIDCreator(), settings, collector);

			// Check that there was a hit and that the contact normal is correct
			CHECK(collector.HadHit());
			const CollideShapeResult &hit = collector.mHit;
			CHECK_APPROX_EQUAL(hit.mContactPointOn1.GetY(), hull_height + box_separation_from_hull, 1.0e-3f);
			CHECK_APPROX_EQUAL(hit.mContactPointOn2.GetY(), hull_height);
			CHECK_APPROX_EQUAL(hit.mPenetrationAxis.NormalizedOr(Vec3::sZero()), -Vec3::sAxisY(), 1.0e-3f);
		}

		CHECK(angle >= 2.0f * JPH_PI);
	}

	// This test checks extreme values of the max separation distance and how it affects ConvexShape::sCollideConvexVsConvex
	// See: https://github.com/jrouwe/JoltPhysics/discussions/1379
	TEST_CASE("TestBoxVsSphereLargeSeparationDistance")
	{
		constexpr float cRadius = 1.0f;
		constexpr float cHalfExtent = 10.0f;
		RefConst<Shape> sphere_shape = new SphereShape(cRadius);
		RefConst<Shape> box_shape = new BoxShape(Vec3::sReplicate(cHalfExtent));
		float distances[] = { 0.0f, 0.5f, 1.0f, 5.0f, 10.0f, 50.0f, 100.0f, 500.0f, 1000.0f, 5000.0f, 10000.0f };
		for (float x : distances)
			for (float max_separation : distances)
			{
				CollideShapeSettings collide_settings;
				collide_settings.mMaxSeparationDistance = max_separation;
				ClosestHitCollisionCollector<CollideShapeCollector> collector;
				CollisionDispatch::sCollideShapeVsShape(box_shape, sphere_shape, Vec3::sOne(), Vec3::sOne(), Mat44::sIdentity(), Mat44::sTranslation(Vec3(x, 0, 0)), SubShapeIDCreator(), SubShapeIDCreator(), collide_settings, collector);

				float expected_penetration = cHalfExtent - (x - cRadius);
				if (collector.HadHit())
					CHECK_APPROX_EQUAL(expected_penetration, collector.mHit.mPenetrationDepth, 1.0e-3f);
				else
					CHECK(expected_penetration < -max_separation);
			}
	}

	// This test case checks extreme values of the max separation distance and how it affects CollideConvexVsTriangles::Collide
	// See: https://github.com/jrouwe/JoltPhysics/discussions/1379
	TEST_CASE("TestTriangleVsBoxLargeSeparationDistance")
	{
		constexpr float cTriangleX = -0.1f;
		constexpr float cHalfExtent = 10.0f;
		RefConst<Shape> triangle_shape = new TriangleShape(Vec3(cTriangleX, -10, 10), Vec3(cTriangleX, -10, -10), Vec3(cTriangleX, 10, 0));
		RefConst<Shape> box_shape = new BoxShape(Vec3::sReplicate(cHalfExtent));
		float distances[] = { 0.0f, 0.5f, 1.0f, 5.0f, 10.0f, 50.0f, 100.0f, 500.0f, 1000.0f, 5000.0f, 10000.0f };
		for (float x : distances)
			for (float max_separation : distances)
			{
				CollideShapeSettings collide_settings;
				collide_settings.mMaxSeparationDistance = max_separation;
				ClosestHitCollisionCollector<CollideShapeCollector> collector;
				CollisionDispatch::sCollideShapeVsShape(triangle_shape, box_shape, Vec3::sOne(), Vec3::sOne(), Mat44::sIdentity(), Mat44::sTranslation(Vec3(x, 0, 0)), SubShapeIDCreator(), SubShapeIDCreator(), collide_settings, collector);

				float expected_penetration = cTriangleX - (x - cHalfExtent);
				if (collector.HadHit())
					CHECK_APPROX_EQUAL(expected_penetration, collector.mHit.mPenetrationDepth, 1.0e-3f);
				else
				{
					CHECK(expected_penetration < -max_separation);
					CHECK_APPROX_EQUAL(collector.mHit.mPenetrationAxis.NormalizedOr(Vec3::sZero()), Vec3::sAxisX(), 1.0e-5f);
				}
			}
	}

	TEST_CASE("TestCollideTriangleVsTriangle")
	{
		constexpr float cPenetration = 0.01f;

		// A triangle centered around the origin in the XZ plane
		RefConst<Shape> t1 = new TriangleShape(Vec3(-1, 0, 1), Vec3(1, 0, 1), Vec3(0, 0, -1));

		// A triangle in the XY plane with its tip just pointing in the origin
		RefConst<Shape> t2 = new TriangleShape(Vec3(-1, 1, 0), Vec3(1, 1, 0), Vec3(0, -cPenetration, 0));

		CollideShapeSettings collide_settings;
		ClosestHitCollisionCollector<CollideShapeCollector> collector;
		CollisionDispatch::sCollideShapeVsShape(t1, t2, Vec3::sOne(), Vec3::sOne(), Mat44::sIdentity(), Mat44::sIdentity(), SubShapeIDCreator(), SubShapeIDCreator(), collide_settings, collector);

		CHECK(collector.HadHit());
		CHECK_APPROX_EQUAL(collector.mHit.mContactPointOn1, Vec3::sZero());
		CHECK_APPROX_EQUAL(collector.mHit.mContactPointOn2, Vec3(0, -cPenetration, 0));
		CHECK_APPROX_EQUAL(collector.mHit.mPenetrationDepth, cPenetration);
		CHECK_APPROX_EQUAL(collector.mHit.mPenetrationAxis.Normalized(), Vec3(0, 1, 0));
	}
}
