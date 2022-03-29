// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include "PhysicsTestContext.h"
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Jolt/Physics/Collision/TransformedShape.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/PhysicsMaterialSimple.h>

TEST_SUITE("TransformedShapeTests")
{
	TEST_CASE("TestTransformedShape")
	{
		const Vec3 half_extents(0.5f, 1.0f, 1.5f);
		const Vec3 scale(-2, 3, 4);
		const Vec3 rtshape_translation(1, 3, 5);
		const Quat rtshape_rotation = Quat::sRotation(Vec3(1, 2, 3).Normalized(), 0.25f * JPH_PI);
		const Vec3 translation(13, 9, 7);
		const Quat rotation = Quat::sRotation(Vec3::sAxisY(), 0.5f * JPH_PI); // A rotation of 90 degrees in order to not shear the shape

		PhysicsMaterialSimple *material = new PhysicsMaterialSimple("Test Material", Color::sRed);

		// Create a scaled, rotated and translated box
		BoxShapeSettings box_settings(half_extents, 0.0f, material);
		box_settings.SetEmbedded();
		ScaledShapeSettings scale_settings(&box_settings, scale);
		scale_settings.SetEmbedded();
		RotatedTranslatedShapeSettings rtshape_settings(rtshape_translation, rtshape_rotation, &scale_settings);
		rtshape_settings.SetEmbedded();

		// Create a body with this shape
		PhysicsTestContext c;
		Body &body = c.CreateBody(&rtshape_settings, translation, rotation, EMotionType::Static, EMotionQuality::Discrete, 0, EActivation::DontActivate);

		// Collect the leaf shape transform
		AllHitCollisionCollector<TransformedShapeCollector> collector;
		c.GetSystem()->GetNarrowPhaseQuery().CollectTransformedShapes(AABox::sBiggest(), collector);

		// Check that there is exactly 1 shape
		CHECK(collector.mHits.size() == 1);
		TransformedShape &ts = collector.mHits.front();

		// Check that we got the leaf shape: box
		CHECK(ts.mShape == box_settings.Create().Get());

		// Check that its transform matches the transform that we provided
		Mat44 calc_transform = Mat44::sRotationTranslation(rotation, translation) * Mat44::sRotationTranslation(rtshape_rotation, rtshape_translation) * Mat44::sScale(scale);
		CHECK_APPROX_EQUAL(calc_transform, ts.GetWorldTransform());

		// Check that all corner points are in the bounding box
		AABox aabox = ts.GetWorldSpaceBounds();
		Vec3 corners[] = {
			Vec3(-0.99f, -0.99f, -0.99f) * half_extents,
			Vec3( 0.99f, -0.99f, -0.99f) * half_extents,
			Vec3(-0.99f,  0.99f, -0.99f) * half_extents,
			Vec3( 0.99f,  0.99f, -0.99f) * half_extents,
			Vec3(-0.99f, -0.99f,  0.99f) * half_extents,
			Vec3( 0.99f, -0.99f,  0.99f) * half_extents,
			Vec3(-0.99f,  0.99f,  0.99f) * half_extents,
			Vec3( 0.99f,  0.99f,  0.99f) * half_extents
		};
		for (Vec3 corner : corners)
		{
			CHECK(aabox.Contains(calc_transform * corner));
			CHECK(!aabox.Contains(calc_transform * (2 * corner))); // Check that points twice as far away are not in the box
		}

		// Now pick a point on the box near the edge in local space, determine a raycast that hits it
		const Vec3 point_on_box(half_extents.GetX() - 0.01f, half_extents.GetY() - 0.01f, half_extents.GetZ());
		const Vec3 normal_on_box(0, 0, 1);
		const Vec3 ray_direction_local(1, 1, -1);
		
		// Transform to world space and do the raycast
		Vec3 ray_start_local = point_on_box - ray_direction_local;
		Vec3 ray_end_local = point_on_box + ray_direction_local;
		Vec3 ray_start_world = calc_transform * ray_start_local;
		Vec3 ray_end_world = calc_transform * ray_end_local;
		Vec3 ray_direction_world = ray_end_world - ray_start_world;
		RayCast ray_in_world { ray_start_world, ray_direction_world };
		RayCastResult hit;
		ts.CastRay(ray_in_world, hit);

		// Check the hit result
		CHECK_APPROX_EQUAL(hit.mFraction, 0.5f);
		CHECK(hit.mBodyID == body.GetID());
		CHECK(ts.GetMaterial(hit.mSubShapeID2) == material);
		Vec3 world_space_normal = ts.GetWorldSpaceSurfaceNormal(hit.mSubShapeID2, ray_in_world.GetPointOnRay(hit.mFraction));
		Vec3 expected_normal = (calc_transform.GetDirectionPreservingMatrix() * normal_on_box).Normalized();
		CHECK_APPROX_EQUAL(world_space_normal, expected_normal);

		// Reset the transform to identity and check that it worked
		ts.SetWorldTransform(Mat44::sIdentity());
		CHECK_APPROX_EQUAL(ts.GetWorldTransform(), Mat44::sIdentity());

		// Set the calculated world transform again to see if getting/setting a transform is symmetric
		ts.SetWorldTransform(calc_transform);
		CHECK_APPROX_EQUAL(calc_transform, ts.GetWorldTransform());
	}
}
