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
#include <Jolt/Physics/Collision/CollideShape.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Jolt/Physics/Collision/CollisionDispatch.h>
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
		static const Vec3 cPosition1A(10.0f, 11.0f, 12.0f);
		static const Vec3 cPosition1B(10.0f, 21.0f, 12.0f);
		static const float cRadius1 = 2.0f;

		// Locations of sphere in the physics system
		static const Vec3 cPosition2A(13.0f, 11.0f, 12.0f);
		static const Vec3 cPosition2B(13.0f, 22.0f, 12.0f);
		static const float cRadius2 = 1.5f;

		// Create sphere to test with (shape 1)
		Ref<Shape> shape1 = new SphereShape(cRadius1);
		Mat44 shape1_com = Mat44::sTranslation(shape1->GetCenterOfMass());
		Mat44 shape1_transform = Mat44::sTranslation(cPosition1A) * Mat44::sRotationX(0.1f * JPH_PI) * shape1_com;

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
		c.GetSystem()->GetNarrowPhaseQuery().CollideShape(shape1, Vec3::sReplicate(1.0f), shape1_transform, settings, fail_collector, broadphase_moving_filter, object_moving_filter);

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
				sCompareCollideShapeResultSphere(cPosition1A, cRadius1, cPosition2A, cRadius2, inResult);
				mWasHit = true;
			}

			bool			mWasHit = false;

		private:
			const Body &	mBody2;
		};
		PositionACollideShapeCollector position_a_collector(body2);

		// Test collision against correct layer
		CHECK(!position_a_collector.mWasHit);
		c.GetSystem()->GetNarrowPhaseQuery().CollideShape(shape1, Vec3::sReplicate(1.0f), shape1_transform, settings, position_a_collector, broadphase_non_moving_filter, object_non_moving_filter);
		CHECK(position_a_collector.mWasHit);

		// Now move body to position B
		c.GetSystem()->GetBodyInterface().SetPositionAndRotation(body2.GetID(), cPosition2B, Quat::sRotation(Vec3::sAxisY(), 0.2f * JPH_PI), EActivation::DontActivate);

		// Test that original position doesn't collide anymore
		c.GetSystem()->GetNarrowPhaseQuery().CollideShape(shape1, Vec3::sReplicate(1.0f), shape1_transform, settings, fail_collector, broadphase_non_moving_filter, object_non_moving_filter);

		// Move test shape to position B
		shape1_transform = Mat44::sTranslation(cPosition1B) * Mat44::sRotationZ(0.3f * JPH_PI) * shape1_com;

		// Test against wrong layer
		c.GetSystem()->GetNarrowPhaseQuery().CollideShape(shape1, Vec3::sReplicate(1.0f), shape1_transform, settings, fail_collector, broadphase_moving_filter, object_moving_filter);

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
				sCompareCollideShapeResultSphere(cPosition1B, cRadius1, cPosition2B, cRadius2, inResult);
				mWasHit = true;
			}

			bool			mWasHit = false;

		private:
			const Body &	mBody2;
		};
		PositionBCollideShapeCollector position_b_collector(body2);

		// Test collision
		CHECK(!position_b_collector.mWasHit);
		c.GetSystem()->GetNarrowPhaseQuery().CollideShape(shape1, Vec3::sReplicate(1.0f), shape1_transform, settings, position_b_collector, broadphase_non_moving_filter, object_non_moving_filter);
		CHECK(position_b_collector.mWasHit);

		// Update the physics system (optimizes the broadphase)
		c.Simulate(c.GetDeltaTime());

		// Test against wrong layer
		c.GetSystem()->GetNarrowPhaseQuery().CollideShape(shape1, Vec3::sReplicate(1.0f), shape1_transform, settings, fail_collector, broadphase_moving_filter, object_moving_filter);

		// Test collision again
		position_b_collector.Reset();
		CHECK(!position_b_collector.mWasHit);
		c.GetSystem()->GetNarrowPhaseQuery().CollideShape(shape1, Vec3::sReplicate(1.0f), shape1_transform, settings, position_b_collector, broadphase_non_moving_filter, object_non_moving_filter);
		CHECK(position_b_collector.mWasHit);
	}

	// Test CollideShape function for a (scaled) sphere vs box
	TEST_CASE("TestCollideShapeSphereVsBox")
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
		CollideShapeSettings settings;
		settings.mActiveEdgeMode = EActiveEdgeMode::CollideWithAll;
		settings.mBackFaceMode = EBackFaceMode::CollideWithBackFaces;

		{
			// Create sphere
			Ref<Shape> normal_sphere = new SphereShape(1.0f);

			// Collect hit with normal sphere
			AllHitCollisionCollector<CollideShapeCollector> collector;
			c.GetSystem()->GetNarrowPhaseQuery().CollideShape(normal_sphere, Vec3::sReplicate(1.0f), Mat44::sTranslation(Vec3(0, 11, 0)), settings, collector);
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
			c.GetSystem()->GetNarrowPhaseQuery().CollideShape(scaled_sphere, Vec3::sReplicate(2.0f), Mat44::sTranslation(Vec3(0, 11, 0)), settings, collector);
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

	// Test colliding a very long capsule vs a box that is intersecting with the linesegment inside the capsule
	// This particular config reported the wrong penetration due to accuarcy problems before
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
		CollisionDispatch::sCollideShapeVsShape(capsule_shape, box_shape, Vec3::sReplicate(1.0f), Vec3::sReplicate(1.0f), capsule_transform, box_transform, SubShapeIDCreator(), SubShapeIDCreator(), settings, collector);
		
		// Check that there was a hit
		CHECK(collector.mHits.size() == 1);
		const CollideShapeResult &result = collector.mHits.front();
		
		// Now move the box 1% further than the returned penetration depth and check that it is no longer in collision
		Vec3 distance_to_move_box = result.mPenetrationAxis.Normalized() * result.mPenetrationDepth;
		collector.Reset();
		CHECK(!collector.HadHit());
		CollisionDispatch::sCollideShapeVsShape(capsule_shape, box_shape, Vec3::sReplicate(1.0f), Vec3::sReplicate(1.0f), capsule_transform, Mat44::sTranslation(1.01f * distance_to_move_box) * box_transform, SubShapeIDCreator(), SubShapeIDCreator(), settings, collector);
		CHECK(!collector.HadHit());

		// Now check that moving 1% less than the penetration distance makes the shapes still overlap
		CollisionDispatch::sCollideShapeVsShape(capsule_shape, box_shape, Vec3::sReplicate(1.0f), Vec3::sReplicate(1.0f), capsule_transform, Mat44::sTranslation(0.99f * distance_to_move_box) * box_transform, SubShapeIDCreator(), SubShapeIDCreator(), settings, collector);
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
		vector<Vec3> obox_points = {
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
		RefConst<ConvexShape> convex_hull = static_cast<const ConvexShape *>(hull_settings.Create().Get().GetPtr());

		// Create triangle support function
		TriangleConvexSupport triangle(v0, v1, v2);

		// Create the convex hull support function
		ConvexShape::SupportBuffer buffer;
		const ConvexShape::Support *support = convex_hull->GetSupportFunction(ConvexShape::ESupportMode::IncludeConvexRadius, buffer, Vec3::sReplicate(1.0f));

		// Triangle is close enough to make GJK report indeterminate
		Vec3 penetration_axis = Vec3::sAxisX(), point1, point2;
		EPAPenetrationDepth pen_depth;
		EPAPenetrationDepth::EStatus status = pen_depth.GetPenetrationDepthStepGJK(*support, support->GetConvexRadius(), triangle, 0.0f, cDefaultCollisionTolerance, penetration_axis, point1, point2);
		CHECK(status == EPAPenetrationDepth::EStatus::Indeterminate);

		// But there should not be an actual collision
		CHECK(!pen_depth.GetPenetrationDepthStepEPA(*support, triangle, cDefaultPenetrationTolerance, penetration_axis, point1, point2));
	}
}
