// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/ConvexCollision/RandomRayTest.h>
#include <Jolt/Geometry/Sphere.h>
#include <Jolt/Geometry/AABox.h>
#include <Jolt/Geometry/GJKClosestPoint.h>
#include <Jolt/Geometry/RayTriangle.h>
#include <Jolt/Geometry/RaySphere.h>
#include <Jolt/Geometry/RayAABox.h>
#include <Jolt/Geometry/RayCapsule.h>
#include <Jolt/Geometry/RayCylinder.h>
#include <Jolt/Geometry/ConvexSupport.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/CylinderShape.h>
#include <Renderer/DebugRendererImp.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(RandomRayTest)
{
	JPH_ADD_BASE_CLASS(RandomRayTest, Test)
}

//-----------------------------------------------------------------------------
// Tests the CastRay function
//-----------------------------------------------------------------------------
template <typename A, typename Context>
void RandomRayTest::TestRay(const char *inTestName, RVec3Arg inRenderOffset, const A &inA, const Context &inContext, float (*inCompareFunc)(const Context &inContext, Vec3Arg inRayOrigin, Vec3Arg inRayDirection))
{
	default_random_engine random(12345);
	uniform_real_distribution<float> random_scale(-2.0f, 2.0f);

#ifdef JPH_DEBUG
	const int count = 1000;
#else
	const int count = 10000;
#endif

	int mismatches = 0;
	int nonzero_hits = 0;
	int zero_hits = 0;
	float total_error = 0;
	int total_error_count = 0;
	float min_error = FLT_MAX;
	float max_error = 0;

	GJKClosestPoint gjk;

	Trace("Starting: %s", inTestName);

	for (int i = 0; i < count; ++i)
	{
		Vec3 from(random_scale(random), random_scale(random), random_scale(random));
		Vec3 to(random_scale(random), random_scale(random), random_scale(random));
		Vec3 direction = to - from;

		// Use GJK to cast a ray
		float fraction1 = 1.0f + FLT_EPSILON;
		if (!gjk.CastRay(from, direction, 1.0e-4f, inA, fraction1))
			fraction1 = FLT_MAX;

		// Use the comparison function
		float fraction2 = inCompareFunc(inContext, from, direction);

		// The comparison functions work with infinite rays, so a fraction > 1 means a miss
		if (fraction2 > 1.0f)
			fraction2 = FLT_MAX;

		float error = abs(fraction1 - fraction2);
		if (error > 0.005f)
		{
			Trace("Mismatch iteration: %d (%f vs %f, diff: %f)", i, (double)fraction1, (double)fraction2, (double)abs(fraction2 - fraction1));
			++mismatches;

			Color c;
			if (fraction2 == FLT_MAX)
			{
				c = Color::sRed;
				mDebugRenderer->DrawMarker(inRenderOffset + from + fraction1 * direction, Color::sRed, 0.1f);
			}
			else if (fraction1 == FLT_MAX)
			{
				c = Color::sBlue;
				mDebugRenderer->DrawMarker(inRenderOffset + from + fraction2 * direction, Color::sBlue, 0.1f);
			}
			else
			{
				total_error += abs(fraction2 - fraction1);
				total_error_count++;
				c = Color::sGreen;
				mDebugRenderer->DrawMarker(inRenderOffset + from + fraction1 * direction, Color::sCyan, 0.1f);
				mDebugRenderer->DrawMarker(inRenderOffset + from + fraction2 * direction, Color::sGreen, 0.1f);
			}
			mDebugRenderer->DrawArrow(inRenderOffset + from, inRenderOffset + to, c, 0.1f);
		}
		else if (fraction1 != FLT_MAX)
		{
			mDebugRenderer->DrawMarker(inRenderOffset + from + fraction1 * direction, Color::sYellow, 0.02f);
		}

		if (fraction1 != FLT_MAX && fraction2 != FLT_MAX)
		{
			total_error += error;
			total_error_count++;
			min_error = min(min_error, error);
			max_error = max(max_error, error);
		}

		if (fraction2 == 0.0f)
			++zero_hits;
		else if (fraction2 > 0 && fraction2 <= 1.0f)
			++nonzero_hits;
	}

	Trace("Report for: %s", inTestName);
	Trace("Mismatches: %d (%.1f%%)", mismatches, 100.0 * mismatches / count);
	Trace("Hits (fraction = 0): %d (%.1f%%)", zero_hits, 100.0 * zero_hits / count);
	Trace("Hits (fraction > 0 and fraction <= 1): %d (%.1f%%)", nonzero_hits, 100.0 * nonzero_hits / count);
	Trace("Fraction error: Avg %f, Min %f, Max %f", total_error_count > 0? double(total_error / total_error_count) : 0.0, (double)min_error, (double)max_error);
}

void RandomRayTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	{
		RVec3 render_offset(0, 0, 0);
		Sphere sphere(Vec3(0.1f, 0.2f, 0.3f), 1.1f);
		mDebugRenderer->DrawSphere(render_offset + sphere.GetCenter(), sphere.GetRadius(), Color::sYellow);
		TestRay<Sphere, Sphere>("Sphere", render_offset, sphere, sphere, [](const Sphere &inSphere, Vec3Arg inRayOrigin, Vec3Arg inRayDirection) {
			return RaySphere(inRayOrigin, inRayDirection, inSphere.GetCenter(), inSphere.GetRadius());
		});
	}

	{
		RVec3 render_offset(5, 0, 0);
		SphereShape sphere_shape(1.1f);
	#ifdef JPH_DEBUG_RENDERER
		sphere_shape.Draw(mDebugRenderer, RMat44::sTranslation(render_offset), Vec3::sOne(), Color::sYellow, false, false);
	#endif // JPH_DEBUG_RENDERER
		ConvexShape::SupportBuffer buffer;
		const ConvexShape::Support *support = sphere_shape.GetSupportFunction(ConvexShape::ESupportMode::IncludeConvexRadius, buffer, Vec3::sOne());
		TestRay<ConvexShape::Support, SphereShape>("Sphere Shape", render_offset, *support, sphere_shape, [](const SphereShape &inSphere, Vec3Arg inRayOrigin, Vec3Arg inRayDirection) {
			return RaySphere(inRayOrigin, inRayDirection, Vec3::sZero(), inSphere.GetRadius());
		});
	}

	{
		RVec3 render_offset(10, 0, 0);
		AABox box(Vec3(-0.9f, -1.0f, -1.1f), Vec3(0.8f, 0.9f, 1.0f));
		mDebugRenderer->DrawBox(box.Transformed(Mat44::sTranslation(Vec3(render_offset))), Color::sYellow);
		TestRay<AABox, AABox>("Box", render_offset, box, box, [](const AABox &inBox, Vec3Arg inRayOrigin, Vec3Arg inRayDirection) {
			float fraction = RayAABox(inRayOrigin, RayInvDirection(inRayDirection), inBox.mMin, inBox.mMax);
			return max(fraction, 0.0f);
		});
	}

	{
		RVec3 render_offset(15, 0, 0);
		BoxShape box_shape(Vec3(0.9f, 1.0f, 1.1f), 0.0f);
	#ifdef JPH_DEBUG_RENDERER
		box_shape.Draw(mDebugRenderer, RMat44::sTranslation(render_offset), Vec3::sOne(), Color::sYellow, false, false);
	#endif // JPH_DEBUG_RENDERER
		ConvexShape::SupportBuffer buffer;
		const ConvexShape::Support *support = box_shape.GetSupportFunction(ConvexShape::ESupportMode::IncludeConvexRadius, buffer, Vec3::sOne());
		TestRay<ConvexShape::Support, BoxShape>("Box Shape", render_offset, *support, box_shape, [](const BoxShape &inBox, Vec3Arg inRayOrigin, Vec3Arg inRayDirection) {
			float fraction = RayAABox(inRayOrigin, RayInvDirection(inRayDirection), -inBox.GetHalfExtent(), inBox.GetHalfExtent());
			return max(fraction, 0.0f);
		});
	}

	{
		RVec3 render_offset(20, 0, 0);
		CapsuleShape capsule_shape(1.1f, 0.6f);
	#ifdef JPH_DEBUG_RENDERER
		capsule_shape.Draw(mDebugRenderer, RMat44::sTranslation(render_offset), Vec3::sOne(), Color::sYellow, false, false);
	#endif // JPH_DEBUG_RENDERER
		ConvexShape::SupportBuffer buffer;
		const ConvexShape::Support *support = capsule_shape.GetSupportFunction(ConvexShape::ESupportMode::IncludeConvexRadius, buffer, Vec3::sOne());
		TestRay<ConvexShape::Support, CapsuleShape>("Capsule Shape", render_offset, *support, capsule_shape, [](const CapsuleShape &inCapsule, Vec3Arg inRayOrigin, Vec3Arg inRayDirection) {
			return RayCapsule(inRayOrigin, inRayDirection, inCapsule.GetHalfHeightOfCylinder(), inCapsule.GetRadius());
		});
	}

	{
		RVec3 render_offset(25, 0, 0);
		CylinderShape cylinder_shape(1.5f, 0.6f, 0.0f);
	#ifdef JPH_DEBUG_RENDERER
		cylinder_shape.Draw(mDebugRenderer, RMat44::sTranslation(render_offset), Vec3::sOne(), Color::sYellow, false, false);
	#endif // JPH_DEBUG_RENDERER
		ConvexShape::SupportBuffer buffer;
		const ConvexShape::Support *support = cylinder_shape.GetSupportFunction(ConvexShape::ESupportMode::IncludeConvexRadius, buffer, Vec3::sOne());
		TestRay<ConvexShape::Support, CylinderShape>("Cylinder Shape", render_offset, *support, cylinder_shape, [](const CylinderShape &inCylinder, Vec3Arg inRayOrigin, Vec3Arg inRayDirection) {
			return RayCylinder(inRayOrigin, inRayDirection, inCylinder.GetHalfHeight(), inCylinder.GetRadius());
		});
	}

	{
		RVec3 render_offset(30, 0, 0);
		TriangleConvexSupport triangle(Vec3(0.1f, 0.9f, 0.3f), Vec3(-0.9f, -0.5f, 0.2f), Vec3(0.7f, -0.3f, -0.1f));
		mDebugRenderer->DrawTriangle(render_offset + triangle.mV1, render_offset + triangle.mV2, render_offset + triangle.mV3, Color::sYellow);
		TestRay<TriangleConvexSupport, TriangleConvexSupport>("Triangle", render_offset, triangle, triangle, [](const TriangleConvexSupport &inTriangle, Vec3Arg inRayOrigin, Vec3Arg inRayDirection) {
			return RayTriangle(inRayOrigin, inRayDirection, inTriangle.mV1, inTriangle.mV2, inTriangle.mV3);
		});
	}
}
