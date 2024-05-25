// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include <Jolt/Geometry/ConvexSupport.h>
#include <Jolt/Geometry/EPAPenetrationDepth.h>
#include <Jolt/Geometry/AABox.h>
#include <Jolt/Geometry/Sphere.h>
#include <random>

// Enable to trace accuracy of EPA algorithm
#define EPA_TESTS_TRACE(...)
//#define EPA_TESTS_TRACE(...) printf(__VA_ARGS__)

TEST_SUITE("EPATests")
{
	/// Helper function to return the angle between two vectors in degrees
	static float AngleBetweenVectors(Vec3Arg inV1, Vec3Arg inV2)
	{
		float dot = inV1.Dot(inV2);
		float len = inV1.Length() * inV2.Length();
		return RadiansToDegrees(ACos(dot / len));
	}

	/// Test box versus sphere and compare analytical solution with that of the EPA algorithm
	/// @return If a collision was detected
	static bool CollideBoxSphere(Mat44Arg inMatrix, const AABox &inBox, const Sphere &inSphere)
	{
		TransformedConvexObject<AABox> transformed_box(inMatrix, inBox);
		TransformedConvexObject<Sphere> transformed_sphere(inMatrix, inSphere);

		// Use EPA algorithm. Don't use convex radius to avoid EPA being skipped because the inner hulls are not touching.
		EPAPenetrationDepth epa;
		Vec3 v1 = Vec3::sAxisX(), pa1, pb1;
		bool intersect1 = epa.GetPenetrationDepth(transformed_box, transformed_box, 0.0f, transformed_sphere, transformed_sphere, 0.0f, 1.0e-2f, FLT_EPSILON, v1, pa1, pb1);

		// Analytical solution
		Vec3 pa2 = inBox.GetClosestPoint(inSphere.GetCenter());
		Vec3 v2 = inSphere.GetCenter() - pa2;
		bool intersect2 = v2.LengthSq() <= Square(inSphere.GetRadius());

		CHECK(intersect1 == intersect2);
		if (intersect1 && intersect2)
		{
			// Analytical solution of contact on B
			Vec3 pb2 = inSphere.GetCenter() - inSphere.GetRadius() * v2.NormalizedOr(Vec3::sZero());

			// Transform analytical solution
			v2 = inMatrix.Multiply3x3(v2);
			pa2 = inMatrix * pa2;
			pb2 = inMatrix * pb2;

			// Check angle between v1 and v2
			float angle = AngleBetweenVectors(v1, v2);
			CHECK(angle < 0.1f);
			EPA_TESTS_TRACE("Angle = %.9g\n", (double)angle);

			// Check delta between contact on A
			Vec3 dpa = pa2 - pa1;
			CHECK(dpa.Length() < 8.0e-4f);
			EPA_TESTS_TRACE("Delta A = %.9g\n", (double)dpa.Length());

			// Check delta between contact on B
			Vec3 dpb = pb2 - pb1;
			CHECK(dpb.Length() < 8.0e-4f);
			EPA_TESTS_TRACE("Delta B = %.9g\n", (double)dpb.Length());
		}

		return intersect1;
	}

	/// Test multiple boxes against spheres and transform both with inMatrix
	static void CollideBoxesWithSpheres(Mat44Arg inMatrix)
	{
		{
			// Sphere just missing face of box
			AABox box(Vec3(-2, -3, -4), Vec3(2, 3, 4));
			Sphere sphere(Vec3(4, 0, 0), 1.99f);
			CHECK(!CollideBoxSphere(inMatrix, box, sphere));
		}

		{
			// Sphere just touching face of box
			AABox box(Vec3(-2, -3, -4), Vec3(2, 3, 4));
			Sphere sphere(Vec3(4, 0, 0), 2.01f);
			CHECK(CollideBoxSphere(inMatrix, box, sphere));
		}

		{
			// Sphere deeply penetrating box on face
			AABox box(Vec3(-2, -3, -4), Vec3(2, 3, 4));
			Sphere sphere(Vec3(3, 0, 0), 2);
			CHECK(CollideBoxSphere(inMatrix, box, sphere));
		}

		{
			// Sphere just missing box on edge
			AABox box(Vec3(1, 1, -2), Vec3(2, 2, 2));
			Sphere sphere(Vec3(4, 4, 0), sqrt(8.0f) - 0.01f);
			CHECK(!CollideBoxSphere(inMatrix, box, sphere));
		}

		{
			// Sphere just penetrating box on edge
			AABox box(Vec3(1, 1, -2), Vec3(2, 2, 2));
			Sphere sphere(Vec3(4, 4, 0), sqrt(8.0f) + 0.01f);
			CHECK(CollideBoxSphere(inMatrix, box, sphere));
		}

		{
			// Sphere just missing box on vertex
			AABox box(Vec3(1, 1, 1), Vec3(2, 2, 2));
			Sphere sphere(Vec3(4, 4, 4), sqrt(12.0f) - 0.01f);
			CHECK(!CollideBoxSphere(inMatrix, box, sphere));
		}

		{
			// Sphere just penetrating box on vertex
			AABox box(Vec3(1, 1, 1), Vec3(2, 2, 2));
			Sphere sphere(Vec3(4, 4, 4), sqrt(12.0f) + 0.01f);
			CHECK(CollideBoxSphere(inMatrix, box, sphere));
		}
	}

	TEST_CASE("TestEPASphereBox")
	{
		// Test identity transform
		CollideBoxesWithSpheres(Mat44::sIdentity());

		// Test some random rotations/translations
		UnitTestRandom random;
		for (int i = 0; i < 10; ++i)
			CollideBoxesWithSpheres(Mat44::sRotationTranslation(Quat::sRandom(random), Vec3::sRandom(random)));
	}

	TEST_CASE("TestEPASphereSphereOverlapping")
	{
		// Worst case: Two spheres exactly overlapping
		// In this case the Minkowski sum is a sphere which means the EPA algorithm will be building a convex hull of a full sphere and run out of triangles resulting in a pretty bad approximation
		Sphere sphere(Vec3(1, 2, 3), 2.0f);
		EPAPenetrationDepth epa;
		Vec3 v = Vec3::sAxisX(), pa, pb;
		CHECK(epa.GetPenetrationDepth(sphere, sphere, 0.0f, sphere, sphere, 0.0f, 1.0e-4f, FLT_EPSILON, v, pa, pb));
		float delta_a = (pa - sphere.GetCenter()).Length() - sphere.GetRadius();
		CHECK(abs(delta_a) < 0.07f);
		float delta_b = (pb - sphere.GetCenter()).Length() - sphere.GetRadius();
		CHECK(abs(delta_b) < 0.07f);
		float delta_penetration = (pa - pb).Length() - 2.0f * sphere.GetRadius();
		CHECK(abs(delta_penetration) < 0.14f);
		float angle = AngleBetweenVectors(v, pa - pb);
		CHECK(angle < 0.02f);
	}

	TEST_CASE("TestEPASphereSphereNearOverlapping")
	{
		// Near worst case: Two spheres almost exactly overlapping
		// Still limited by amount of triangles in the hull but more precise
		Sphere sphere1(Vec3(1, 2, 3), 2.0f);
		Sphere sphere2(Vec3(1.1f, 2, 3), 1.8f);
		EPAPenetrationDepth epa;
		Vec3 v = Vec3::sAxisX(), pa, pb;
		CHECK(epa.GetPenetrationDepth(sphere1, sphere1, 0.0f, sphere2, sphere2, 0.0f, 1.0e-4f, FLT_EPSILON, v, pa, pb));
		float delta_a = (pa - sphere1.GetCenter()).Length() - sphere1.GetRadius();
		CHECK(abs(delta_a) < 0.05f);
		float delta_b = (pb - sphere2.GetCenter()).Length() - sphere2.GetRadius();
		CHECK(abs(delta_b) < 0.05f);
		float delta_penetration = (pa - pb).Length() - (sphere1.GetRadius() + sphere2.GetRadius() - (sphere1.GetCenter() - sphere2.GetCenter()).Length());
		CHECK(abs(delta_penetration) < 0.06f);
		float angle = AngleBetweenVectors(v, pa - pb);
		CHECK(angle < 0.02f);
	}

	TEST_CASE("TestEPACastSphereSphereMiss")
	{
		Sphere sphere(Vec3(0, 0, 0), 1.0f);
		EPAPenetrationDepth epa;
		float lambda = 1.0f + FLT_EPSILON;
		const Vec3 invalid(-999, -999, -999);
		Vec3 pa = invalid, pb = invalid, normal = invalid;
		CHECK(!epa.CastShape(Mat44::sTranslation(Vec3(-10, 2.1f, 0)), Vec3(20, 0, 0), 1.0e-4f, 1.0e-4f, sphere, sphere, 0.0f, 0.0f, true, lambda, pa, pb, normal));
		CHECK(lambda == 1.0f + FLT_EPSILON); // Check input values didn't change
		CHECK(pa == invalid);
		CHECK(pb == invalid);
		CHECK(normal == invalid);
	}

	TEST_CASE("TestEPACastSphereSphereInitialOverlap")
	{
		Sphere sphere(Vec3(0, 0, 0), 1.0f);
		EPAPenetrationDepth epa;
		float lambda = 1.0f + FLT_EPSILON;
		const Vec3 invalid(-999, -999, -999);
		Vec3 pa = invalid, pb = invalid, normal = invalid;
		CHECK(epa.CastShape(Mat44::sTranslation(Vec3(-1, 0, 0)), Vec3(10, 0, 0), 1.0e-4f, 1.0e-4f, sphere, sphere, 0.0f, 0.0f, true, lambda, pa, pb, normal));
		CHECK(lambda == 0.0f);
		CHECK_APPROX_EQUAL(pa, Vec3::sZero(), 5.0e-3f);
		CHECK_APPROX_EQUAL(pb, Vec3(-1, 0, 0), 5.0e-3f);
		CHECK_APPROX_EQUAL(normal.NormalizedOr(Vec3::sZero()), Vec3(1, 0, 0), 1.0e-2f);
	}

	TEST_CASE("TestEPACastSphereSphereHit")
	{
		Sphere sphere(Vec3(0, 0, 0), 1.0f);
		EPAPenetrationDepth epa;
		float lambda = 1.0f + FLT_EPSILON;
		const Vec3 invalid(-999, -999, -999);
		Vec3 pa = invalid, pb = invalid, normal = invalid;
		CHECK(epa.CastShape(Mat44::sTranslation(Vec3(-10, 0, 0)), Vec3(20, 0, 0), 1.0e-4f, 1.0e-4f, sphere, sphere, 0.0f, 0.0f, true, lambda, pa, pb, normal));
		CHECK_APPROX_EQUAL(lambda, 8.0f / 20.0f);
		CHECK_APPROX_EQUAL(pa, Vec3(-1, 0, 0));
		CHECK_APPROX_EQUAL(pb, Vec3(-1, 0, 0));
		CHECK_APPROX_EQUAL(normal.NormalizedOr(Vec3::sZero()), Vec3(1, 0, 0));
	}
}
