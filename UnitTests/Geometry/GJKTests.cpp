// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include <Jolt/Geometry/ConvexSupport.h>
#include <Jolt/Geometry/GJKClosestPoint.h>
#include <Jolt/Geometry/AABox.h>
#include <Jolt/Geometry/Sphere.h>
#include <Jolt/Geometry/RayTriangle.h>
#include <Jolt/Geometry/RaySphere.h>
#include <Jolt/Geometry/RayAABox.h>
#include <Jolt/Geometry/RayCapsule.h>
#include <Jolt/Geometry/RayCylinder.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/CylinderShape.h>
#include <random>

TEST_SUITE("GJKTests")
{
	TEST_CASE("TestGJKIntersectSphere")
	{
		GJKClosestPoint gjk;
			
		// Sphere 1 is centered around the origin
		Sphere s1(Vec3::sZero(), 1.0f);

		// Shere 2 is far away from s1
		Vec3 c2(10.0f, 10.0f, 10.0f);
		Sphere s2(c2, 1.0f);

		// Sphere 3 is exactly 2 away from s1
		float l = 2.0f / sqrt(3.0f);
		Vec3 c3(l, l, l);
		Sphere s3(c3, 1.0f);

		{
			// Test sphere s1 and s2, they should not collide
			Vec3 v = Vec3::sZero();
			CHECK_FALSE(gjk.Intersects(s1, s2, 1.0e-4f, v));
		}

		{
			// Test sphere s1 and s3, they should touch exactly
			Vec3 v = Vec3::sZero();
			CHECK(gjk.Intersects(s1, s3, 1.0e-4f, v));
		}

		{
			// Test sphere s1 and s2, they should not collide, verify their closest points
			Vec3 pa, pb, v = Vec3::sZero();
			float d = sqrt(gjk.GetClosestPoints(s1, s2, 1.0e-4f, FLT_MAX, v, pa, pb));
			CHECK_APPROX_EQUAL(c2.Length() - 2.0f, d, 1.0e-4f);
			CHECK_APPROX_EQUAL(c2.Normalized(), pa, 1.0e-4f);
			CHECK_APPROX_EQUAL(c2 - c2.Normalized(), pb, 1.0e-4f);
		}

		{
			// Test sphere s1 and s3, they should touch exactly, verify their closest points
			Vec3 pa, pb, v = Vec3::sZero();
			float d = sqrt(gjk.GetClosestPoints(s1, s3, 1.0e-4f, FLT_MAX, v, pa, pb));
			CHECK_APPROX_EQUAL(0.0f, d, 1.0e-4f);
			CHECK_APPROX_EQUAL(c2.Normalized(), pa, 1.0e-4f);
			CHECK_APPROX_EQUAL(c2.Normalized(), pb, 1.0e-4f);
		}
	}

	template <typename A, typename B>
	static void TestIntersect(
		A (*inCreateFuncA)(UnitTestRandom &), 
		B (*inCreateFuncB)(UnitTestRandom &), 
		bool (*inCompareFunc)(const A &inA, const B &inB, bool inIsIntersecting, float inTolerance))
	{
		UnitTestRandom random(12345);

		const int count = 10000;

		int hits = 0;

		GJKClosestPoint gjk;

		for (int i = 0; i < count; ++i)
		{
			A shape1 = inCreateFuncA(random);
			B shape2 = inCreateFuncB(random);

			// Use GJK to test for intersection
			Vec3 v = Vec3::sZero();
			const float cTolerance = 1.0e-4f;
			bool result_gjk = gjk.Intersects(shape1, shape2, cTolerance, v);

			// Compare with reference function and increase tolerance a bit to account for floating point imprecision
			CHECK(inCompareFunc(shape1, shape2, result_gjk, 2.0f * cTolerance));
		
			if (result_gjk)
				++hits;
		}

		// Check that there were enough hits so that the test is representative
		float hit_rate = 100.0f * hits / count;
		CHECK(hit_rate > 30.0f);
		CHECK(hit_rate < 70.0f);
	}

	TEST_CASE("TestGJKSphereVsSphereIntersect")
	{
		auto sphere_creator = [](UnitTestRandom &inRandom) {
				uniform_real_distribution<float> pos(-2.0f, 2.0f);
				uniform_real_distribution<float> rad(0.5f, 2.0f);
				return Sphere(Vec3(pos(inRandom), pos(inRandom), pos(inRandom)), rad(inRandom));
			};

		TestIntersect<Sphere, Sphere>(
			sphere_creator, 
			sphere_creator, 
			[](const Sphere &inSphereA, const Sphere &inSphereB, bool inIsIntersecting, float inTolerance) {

				// Test without and with tolerance if the results are equal
				return inSphereA.Overlaps(inSphereB) == inIsIntersecting
					|| Sphere(inSphereA.GetCenter(), inSphereA.GetRadius() + inTolerance).Overlaps(inSphereB) == inIsIntersecting;
			});
	}

	TEST_CASE("TestGJKSphereVsBoxIntersect")
	{
		auto sphere_creator = [](UnitTestRandom &inRandom) {
				uniform_real_distribution<float> pos(-2.0f, 2.0f);
				uniform_real_distribution<float> rad(0.5f, 2.0f);
				return Sphere(Vec3(pos(inRandom), pos(inRandom), pos(inRandom)), rad(inRandom));
			};

		auto box_creator = [](UnitTestRandom &inRandom) {
				uniform_real_distribution<float> pos(-2.0f, 2.0f);
				Vec3 p1 = Vec3(pos(inRandom), pos(inRandom), pos(inRandom));
				Vec3 p2 = Vec3(pos(inRandom), pos(inRandom), pos(inRandom));
				return AABox::sFromTwoPoints(p1, p2);
			};

		TestIntersect<Sphere, AABox>(
			sphere_creator, 
			box_creator, 
			[](const Sphere &inSphereA, const AABox &inBoxB, bool inIsIntersecting, float inTolerance) {
				
				// Test without and with tolerance if the results are equal
				return inSphereA.Overlaps(inBoxB) == inIsIntersecting
					|| Sphere(inSphereA.GetCenter(), inSphereA.GetRadius() + inTolerance).Overlaps(inBoxB) == inIsIntersecting;
			});
	}

	template <typename A, typename Context>
	static void TestRay(const A &inA, const Context &inContext, float (*inCompareFunc)(const Context &inContext, Vec3Arg inRayOrigin, Vec3Arg inRayDirection))
	{
		UnitTestRandom random(12345);
		uniform_real_distribution<float> random_scale(-2.0f, 2.0f);

		const int count = 1000;

		for (int i = 0; i < count; ++i)
		{
			Vec3 from(random_scale(random), random_scale(random), random_scale(random));
			Vec3 to(random_scale(random), random_scale(random), random_scale(random));
			Vec3 direction = to - from;

			// Use GJK to cast a ray
			float fraction1 = 1.0f + FLT_EPSILON;
			GJKClosestPoint gjk;
			if (!gjk.CastRay(from, direction, 1.0e-4f, inA, fraction1))
				fraction1 = FLT_MAX;

			// Use the comparison function
			float fraction2 = inCompareFunc(inContext, from, direction);

			// The comparison functions work with infinite rays, so a fraction > 1 means a miss
			if (fraction2 > 1.0f)
				fraction2 = FLT_MAX;

			CHECK_APPROX_EQUAL(fraction1, fraction2, 0.01f);
		}
	}

	TEST_CASE("TestGJKRaySphere")
	{
		Sphere sphere(Vec3(0.1f, 0.2f, 0.3f), 1.1f);
		TestRay<Sphere, Sphere>(sphere, sphere, [](const Sphere &inSphere, Vec3Arg inRayOrigin, Vec3Arg inRayDirection) { 
			return RaySphere(inRayOrigin, inRayDirection, inSphere.GetCenter(), inSphere.GetRadius()); 
		});
	}

	TEST_CASE("TestGJKRaySphereShape")
	{
		SphereShape sphere_shape(1.1f);
		ConvexShape::SupportBuffer buffer;
		const ConvexShape::Support *support = sphere_shape.GetSupportFunction(ConvexShape::ESupportMode::IncludeConvexRadius, buffer, Vec3::sReplicate(1.0f));
		TestRay<ConvexShape::Support, SphereShape>(*support, sphere_shape, [](const SphereShape &inSphere, Vec3Arg inRayOrigin, Vec3Arg inRayDirection) { 
			return RaySphere(inRayOrigin, inRayDirection, Vec3::sZero(), inSphere.GetRadius()); 
		});
	}

	TEST_CASE("TestGJKRayBox")
	{
		AABox box(Vec3(-0.9f, -1.0f, -1.1f), Vec3(0.8f, 0.9f, 1.0f));
		TestRay<AABox, AABox>(box, box, [](const AABox &inBox, Vec3Arg inRayOrigin, Vec3Arg inRayDirection) { 
			float fraction = RayAABox(inRayOrigin, RayInvDirection(inRayDirection), inBox.mMin, inBox.mMax); 
			return max(fraction, 0.0f);
		});
	}

	TEST_CASE("TestGJKRayBoxShape")
	{			
		BoxShape box_shape(Vec3(0.9f, 1.0f, 1.1f), 0.0f);
		ConvexShape::SupportBuffer buffer;
		const ConvexShape::Support *support = box_shape.GetSupportFunction(ConvexShape::ESupportMode::IncludeConvexRadius, buffer, Vec3::sReplicate(1.0f));
		TestRay<ConvexShape::Support, BoxShape>(*support, box_shape, [](const BoxShape &inBox, Vec3Arg inRayOrigin, Vec3Arg inRayDirection) { 
			float fraction = RayAABox(inRayOrigin, RayInvDirection(inRayDirection), -inBox.GetHalfExtent(), inBox.GetHalfExtent()); 
			return max(fraction, 0.0f);
		});
	}

	TEST_CASE("TestGJKRayCapsuleShape")
	{			
		CapsuleShape capsule_shape(1.1f, 0.6f);
		ConvexShape::SupportBuffer buffer;
		const ConvexShape::Support *support = capsule_shape.GetSupportFunction(ConvexShape::ESupportMode::IncludeConvexRadius, buffer, Vec3::sReplicate(1.0f));
		TestRay<ConvexShape::Support, CapsuleShape>(*support, capsule_shape, [](const CapsuleShape &inCapsule, Vec3Arg inRayOrigin, Vec3Arg inRayDirection) { 
			return RayCapsule(inRayOrigin, inRayDirection, inCapsule.GetHalfHeightOfCylinder(), inCapsule.GetRadius()); 
		});
	}

	TEST_CASE("TestGJKRayCylinderShape")
	{			
		CylinderShape cylinder_shape(1.5f, 0.6f, 0.0f);
		ConvexShape::SupportBuffer buffer;
		const ConvexShape::Support *support = cylinder_shape.GetSupportFunction(ConvexShape::ESupportMode::IncludeConvexRadius, buffer, Vec3::sReplicate(1.0f));
		TestRay<ConvexShape::Support, CylinderShape>(*support, cylinder_shape, [](const CylinderShape &inCylinder, Vec3Arg inRayOrigin, Vec3Arg inRayDirection) { 
			return RayCylinder(inRayOrigin, inRayDirection, inCylinder.GetHalfHeight(), inCylinder.GetRadius()); 
		});
	}

	TEST_CASE("TestGJKRayTriangle")
	{			
		TriangleConvexSupport triangle(Vec3(0.1f, 0.9f, 0.3f), Vec3(-0.9f, -0.5f, 0.2f), Vec3(0.7f, -0.3f, -0.1f));
		TestRay<TriangleConvexSupport, TriangleConvexSupport>(triangle, triangle, [](const TriangleConvexSupport &inTriangle, Vec3Arg inRayOrigin, Vec3Arg inRayDirection) { 
			return RayTriangle(inRayOrigin, inRayDirection, inTriangle.mV1, inTriangle.mV2, inTriangle.mV3);
		});
	}
}
