// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/ScaleHelpers.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Core/StreamWrapper.h>

TEST_SUITE("ConvexHullShapeTests")
{
	// Create a convex hull of a box centered around the origin
	static Ref<ConvexHullShape> sCreateBoxHull(Vec3Arg inHalfExtent, float inConvexRadius)
	{
		Array<Vec3> points;
		for (int i = 0; i < 8; ++i)
			points.push_back(inHalfExtent * Vec3(i & 1? 1.0f : -1.0f, i & 2? 1.0f : -1.0f, i & 4? 1.0f : -1.0f));
		ConvexHullShapeSettings settings(points, inConvexRadius);
		Shape::ShapeResult result = settings.Create();
		CHECK(result.IsValid());
		return StaticCast<ConvexHullShape>(result.Get());
	}

	// Create a convex hull that approximates a sphere
	static Ref<ConvexHullShape> sCreateSphereHull(float inRadius)
	{
		Array<Vec3> points;
		constexpr int cNumTheta = 8;
		constexpr int cNumPhi = 12;
		points.push_back(Vec3(0, inRadius, 0));
		points.push_back(Vec3(0, -inRadius, 0));
		for (int t = 1; t < cNumTheta; ++t)
			for (int p = 0; p < cNumPhi; ++p)
				points.push_back(Vec3::sUnitSpherical(JPH_PI * t / cNumTheta, 2.0f * JPH_PI * p / cNumPhi) * inRadius);
		ConvexHullShapeSettings settings(points);
		Shape::ShapeResult result = settings.Create();
		CHECK(result.IsValid());
		return StaticCast<ConvexHullShape>(result.Get());
	}

	TEST_CASE("TestConvexHullShapeInvalid")
	{
		Array<Vec3> box = { Vec3(-1, -1, -1), Vec3(1, -1, -1), Vec3(-1, 1, -1), Vec3(1, 1, -1), Vec3(-1, -1, 1), Vec3(1, -1, 1), Vec3(-1, 1, 1), Vec3(1, 1, 1) };

		// Negative convex radius
		{
			ConvexHullShapeSettings settings(box, -1.0f);
			Shape::ShapeResult result = settings.Create();
			CHECK(result.HasError());
		}

		// Not enough points to build a hull
		{
			ConvexHullShapeSettings settings(Array<Vec3>({ Vec3(0, 0, 0), Vec3(1, 0, 0) }));
			Shape::ShapeResult result = settings.Create();
			CHECK(result.HasError());
		}

		// Valid hull
		{
			ConvexHullShapeSettings settings(box);
			Shape::ShapeResult result = settings.Create();
			CHECK(result.IsValid());
		}
	}

	TEST_CASE("TestConvexHullShapeSurfaceNormal")
	{
		Ref<ConvexHullShape> hull = sCreateBoxHull(Vec3(1, 2, 3), 0.0f);

		CHECK_APPROX_EQUAL(hull->GetSurfaceNormal(SubShapeID(), Vec3(1, 0.5f, 0.5f)), Vec3::sAxisX());
		CHECK_APPROX_EQUAL(hull->GetSurfaceNormal(SubShapeID(), Vec3(-1, 0.5f, 0.5f)), -Vec3::sAxisX());
		CHECK_APPROX_EQUAL(hull->GetSurfaceNormal(SubShapeID(), Vec3(0.5f, 2, 0.5f)), Vec3::sAxisY());
		CHECK_APPROX_EQUAL(hull->GetSurfaceNormal(SubShapeID(), Vec3(0.5f, -2, 0.5f)), -Vec3::sAxisY());
		CHECK_APPROX_EQUAL(hull->GetSurfaceNormal(SubShapeID(), Vec3(0.5f, 0.5f, 3)), Vec3::sAxisZ());
		CHECK_APPROX_EQUAL(hull->GetSurfaceNormal(SubShapeID(), Vec3(0.5f, 0.5f, -3)), -Vec3::sAxisZ());
	}

	TEST_CASE("TestConvexHullShapeSupportFunction")
	{
		const Vec3 cHalfExtent(1, 2, 3);
		const Vec3 cDirection(1, 1, 1);

		for (float convex_radius : { 0.0f, 0.05f })
		{
			Ref<ConvexHullShape> hull = sCreateBoxHull(cHalfExtent, convex_radius);
			CHECK_APPROX_EQUAL(hull->GetConvexRadius(), convex_radius);

			for (Vec3 scale : { Vec3::sOne(), Vec3(2, 3, 0.5f), Vec3(-2, 3, 0.5f) })
			{
				Vec3 scaled_half_extent = scale.Abs() * cHalfExtent;
				float scaled_convex_radius = ScaleHelpers::ScaleConvexRadius(hull->GetConvexRadius(), scale);

				// Including the convex radius we should get the original box
				for (ConvexShape::ESupportMode mode : { ConvexShape::ESupportMode::IncludeConvexRadius, ConvexShape::ESupportMode::Default })
				{
					ConvexShape::SupportBuffer buffer;
					const ConvexShape::Support *support = hull->GetSupportFunction(mode, buffer, scale);
					CHECK(support->GetConvexRadius() == 0.0f);
					CHECK_APPROX_EQUAL(support->GetSupport(cDirection), scaled_half_extent, 1.0e-5f);
					CHECK_APPROX_EQUAL(support->GetSupport(-cDirection), -scaled_half_extent, 1.0e-5f);
				}

				// Excluding the convex radius we should get a shrunk box + the convex radius
				{
					ConvexShape::SupportBuffer buffer;
					const ConvexShape::Support *support = hull->GetSupportFunction(ConvexShape::ESupportMode::ExcludeConvexRadius, buffer, scale);
					CHECK_APPROX_EQUAL(support->GetConvexRadius(), scaled_convex_radius);
					Vec3 shrunk_half_extent = scaled_half_extent - Vec3::sReplicate(scaled_convex_radius);
					CHECK_APPROX_EQUAL(support->GetSupport(cDirection), shrunk_half_extent, 1.0e-5f);
					CHECK_APPROX_EQUAL(support->GetSupport(-cDirection), -shrunk_half_extent, 1.0e-5f);
				}
			}
		}
	}

	TEST_CASE("TestConvexHullShapeSupportingFace")
	{
		const Vec3 cHalfExtent(1, 2, 3);
		Ref<ConvexHullShape> hull = sCreateBoxHull(cHalfExtent, 0.0f);
		Ref<BoxShape> box = new BoxShape(cHalfExtent, 0.0f);
		const Mat44 cTransform = Mat44::sRotationTranslation(Quat::sRotation(Vec3::sAxisY(), 0.1f * JPH_PI), Vec3(1, 2, 3));

		for (Vec3 scale : { Vec3::sOne(), Vec3(2, 3, 0.5f), Vec3(-2, 3, 0.5f) })
			for (Vec3 direction : { Vec3::sAxisX(), -Vec3::sAxisX(), Vec3::sAxisY(), -Vec3::sAxisY(), Vec3(0.1f, 0.2f, 1.0f), Vec3(0.1f, 0.2f, -1.0f) })
			{
				Shape::SupportingFace hull_face, box_face;
				hull->GetSupportingFace(SubShapeID(), direction, scale, cTransform, hull_face);
				box->GetSupportingFace(SubShapeID(), direction, scale, cTransform, box_face);

				// Faces should contain the same vertices
				CHECK(hull_face.size() == 4);
				CHECK(hull_face.size() == box_face.size());
				for (Vec3 v : hull_face)
				{
					bool found = false;
					for (Vec3 v2 : box_face)
						if (v.IsClose(v2, 1.0e-8f))
						{
							found = true;
							break;
						}
					CHECK(found);
				}

				// Faces should have the same winding order
				Vec3 hull_normal = (hull_face[1] - hull_face[0]).Cross(hull_face[2] - hull_face[0]);
				Vec3 box_normal = (box_face[1] - box_face[0]).Cross(box_face[2] - box_face[0]);
				CHECK(hull_normal.Dot(box_normal) > 0.0f);
			}
	}

	TEST_CASE("TestConvexHullShapeSubmergedVolume")
	{
		const Vec3 cHalfExtent(1, 2, 3);
		Ref<ConvexHullShape> hull = sCreateBoxHull(cHalfExtent, 0.0f);
		Ref<BoxShape> box = new BoxShape(cHalfExtent, 0.0f);
		const Mat44 cTransform = Mat44::sRotationTranslation(Quat::sRotation(Vec3::sAxisZ(), 0.1f * JPH_PI), Vec3(0, 1, 0));

		for (Vec3 scale : { Vec3::sOne(), Vec3(2, 3, 0.5f), Vec3(-2, 3, 0.5f) })
			for (float surface_height : { -10.0f, 0.0f, 1.0f, 2.0f, 10.0f })
			{
				Plane surface = Plane::sFromPointAndNormal(Vec3(0, surface_height, 0), Vec3::sAxisY());

				float hull_total_volume, hull_submerged_volume;
				Vec3 hull_center_of_buoyancy;
				hull->GetSubmergedVolume(cTransform, scale, surface, hull_total_volume, hull_submerged_volume, hull_center_of_buoyancy JPH_IF_DEBUG_RENDERER(, RVec3::sZero()));

				float box_total_volume, box_submerged_volume;
				Vec3 box_center_of_buoyancy;
				box->GetSubmergedVolume(cTransform, scale, surface, box_total_volume, box_submerged_volume, box_center_of_buoyancy JPH_IF_DEBUG_RENDERER(, RVec3::sZero()));

				CHECK_APPROX_EQUAL(hull_total_volume, box_total_volume, 1.0e-5f * box_total_volume);
				CHECK_APPROX_EQUAL(hull_submerged_volume, box_submerged_volume, 1.0e-5f * box_total_volume);
				if (box_submerged_volume > 0.0f)
					CHECK_APPROX_EQUAL(hull_center_of_buoyancy, box_center_of_buoyancy, 1.0e-3f);
			}
	}

	TEST_CASE("TestConvexHullShapeFlatRayCast")
	{
		// Create a flat hull in the XZ plane
		Array<Vec3> points = { Vec3(-1, 0, -1), Vec3(1, 0, -1), Vec3(1, 0, 1), Vec3(-1, 0, 1) };
		ConvexHullShapeSettings settings(points, 0.0f);
		Shape::ShapeResult result = settings.Create();
		CHECK(result.IsValid());
		Ref<ConvexHullShape> hull = StaticCast<ConvexHullShape>(result.Get());
		CHECK(hull->GetNumFaces() == 2);

		// Hit from above and below
		for (float y : { 1.0f, -1.0f })
			for (Vec3 xz : { Vec3(0, 0, 0), Vec3(0.9f, 0, 0), Vec3(-0.9f, 0, 0), Vec3(0, 0, 0.9f), Vec3(0, 0, -0.9f), Vec3(0.9f, 0, 0.9f), Vec3(-0.9f, 0, -0.9f) })
			{
				RayCastResult hit;
				CHECK(hull->CastRay(RayCast(xz + Vec3(0, y, 0), Vec3(0, -2.0f * y, 0)), SubShapeIDCreator(), hit));
				CHECK_APPROX_EQUAL(hit.mFraction, 0.5f);
			}

		// Miss because the ray is outside of the polygon
		for (Vec3 xz : { Vec3(1.1f, 0, 0), Vec3(-1.1f, 0, 0), Vec3(0, 0, 1.1f), Vec3(0, 0, -1.1f), Vec3(1.1f, 0, 1.1f), Vec3(-1.1f, 0, -1.1f) })
		{
			RayCastResult hit;
			CHECK(!hull->CastRay(RayCast(xz + Vec3(0, 1, 0), Vec3(0, -2, 0)), SubShapeIDCreator(), hit));
		}

		// Miss because the ray doesn't reach the plane
		{
			RayCastResult hit;
			CHECK(!hull->CastRay(RayCast(Vec3(0, 1, 0), Vec3(0, -0.5f, 0)), SubShapeIDCreator(), hit));
			CHECK(!hull->CastRay(RayCast(Vec3(0, 1, 0), Vec3(0, 2, 0)), SubShapeIDCreator(), hit));
		}

		// Miss because the ray is parallel to the plane
		{
			RayCastResult hit;
			CHECK(!hull->CastRay(RayCast(Vec3(-2, 0, 0), Vec3(4, 0, 0)), SubShapeIDCreator(), hit));
		}
	}

	TEST_CASE("TestConvexHullShapeGetTriangles")
	{
		const Vec3 cHalfExtent(1, 2, 3);
		for (Ref<ConvexHullShape> hull : { sCreateBoxHull(cHalfExtent, 0.0f), sCreateSphereHull(2.0f) })
		{
			Shape::Stats stats = hull->GetStats();
			CHECK(stats.mSizeBytes > sizeof(ConvexHullShape));

			for (Vec3 scale : { Vec3::sOne(), Vec3(2, 3, 0.5f), Vec3(-2, 3, 0.5f) })
			{
				const Vec3 cPosition(1, 2, 3);
				const Quat cRotation = Quat::sRotation(Vec3::sAxisY(), 0.1f * JPH_PI);

				Shape::GetTrianglesContext context;
				hull->GetTrianglesStart(context, AABox::sBiggest(), cPosition, cRotation, scale);

				uint total_triangles = 0;
				float total_area = 0.0f;
				for (;;)
				{
					constexpr int cMaxTriangles = Shape::cGetTrianglesMinTrianglesRequested;
					Float3 vertices[cMaxTriangles * 3];
					const PhysicsMaterial *materials[cMaxTriangles];
					int count = hull->GetTrianglesNext(context, cMaxTriangles, vertices, materials);
					if (count == 0)
						break;
					CHECK(count <= cMaxTriangles);

					for (int t = 0; t < count; ++t)
					{
						CHECK(materials[t] == PhysicsMaterial::sDefault);

						// Check that the triangle is facing outwards
						Vec3 v1(vertices[3 * t]), v2(vertices[3 * t + 1]), v3(vertices[3 * t + 2]);
						Vec3 normal = (v2 - v1).Cross(v3 - v1);
						CHECK(normal.Dot((v1 + v2 + v3) / 3.0f - cPosition) > 0.0f);
						total_area += 0.5f * normal.Length();
					}
					total_triangles += count;
				}

				// Check that we got all triangles
				CHECK(total_triangles == stats.mNumTriangles);

				// Compare the area against the box area
				if (hull->GetNumPoints() == 8)
				{
					Vec3 size = 2.0f * scale.Abs() * cHalfExtent;
					float expected_area = 2.0f * (size.GetX() * size.GetY() + size.GetY() * size.GetZ() + size.GetZ() * size.GetX());
					CHECK_APPROX_EQUAL(total_area, expected_area, 1.0e-3f * expected_area);
				}
			}
		}
	}

	TEST_CASE("TestConvexHullShapeSaveRestore")
	{
		Ref<ConvexHullShape> hull = sCreateSphereHull(2.0f);

		stringstream data;

		// Write the shape
		StreamOutWrapper stream_out(data);
		hull->SaveBinaryState(stream_out);

		// Read it back
		StreamInWrapper stream_in(data);
		Shape::ShapeResult result = Shape::sRestoreFromBinaryState(stream_in);
		CHECK(result.IsValid());
		Ref<ConvexHullShape> restored = StaticCast<ConvexHullShape>(result.Get());

		// Check that the properties are the same
		CHECK(restored->GetCenterOfMass() == hull->GetCenterOfMass());
		CHECK(restored->GetLocalBounds() == hull->GetLocalBounds());
		CHECK(restored->GetConvexRadius() == hull->GetConvexRadius());
		CHECK(restored->GetVolume() == hull->GetVolume());
		CHECK(restored->GetInnerRadius() == hull->GetInnerRadius());
		CHECK(restored->GetMassProperties().mInertia == hull->GetMassProperties().mInertia);
		CHECK(restored->GetPlanes().size() == hull->GetPlanes().size());
		CHECK(restored->GetNumPoints() == hull->GetNumPoints());
		for (uint p = 0; p < hull->GetNumPoints(); ++p)
			CHECK(restored->GetPoint(p) == hull->GetPoint(p));
		CHECK(restored->GetNumFaces() == hull->GetNumFaces());
		for (uint f = 0; f < hull->GetNumFaces(); ++f)
		{
			CHECK(restored->GetPlanes()[f].GetNormal() == hull->GetPlanes()[f].GetNormal());
			CHECK(restored->GetPlanes()[f].GetConstant() == hull->GetPlanes()[f].GetConstant());
			uint num_vertices = hull->GetNumVerticesInFace(f);
			CHECK(restored->GetNumVerticesInFace(f) == num_vertices);
			static constexpr uint cMaxVertices = 256;
			CHECK(num_vertices <= cMaxVertices);
			uint v1[cMaxVertices], v2[cMaxVertices];
			hull->GetFaceVertices(f, cMaxVertices, v1);
			restored->GetFaceVertices(f, cMaxVertices, v2);
			for (uint v = 0; v < num_vertices; ++v)
				CHECK(v1[v] == v2[v]);
		}
	}
}
