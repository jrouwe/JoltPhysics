// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include "PhysicsTestContext.h"
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/TriangleShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/CollideShape.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Jolt/Physics/Collision/CollideConvexVsTriangles.h>
#include <Jolt/Physics/Collision/CollideSphereVsTriangles.h>
#include "Layers.h"

TEST_SUITE("ConvexVsTrianglesTest")
{
	static constexpr float cEdgeLength = 4.0f;

	template <class Collider>
	static void sCheckCollisionNoHit(const CollideShapeSettings &inSettings, Vec3Arg inCenter, float inRadius, uint8 inActiveEdges)
	{
		// Our sphere
		Ref<SphereShape> sphere = new SphereShape(inRadius);

		// Our default triangle
		Vec3 v1(0, 0, 0);
		Vec3 v2(0, 0, cEdgeLength);
		Vec3 v3(cEdgeLength, 0, 0);

		{
			// Collide sphere
			AllHitCollisionCollector<CollideShapeCollector> collector;
			Collider collider(sphere, Vec3::sReplicate(1.0f), Vec3::sReplicate(1.0f), Mat44::sTranslation(inCenter), Mat44::sIdentity(), SubShapeID(), inSettings, collector);
			collider.Collide(v1, v2, v3, inActiveEdges, SubShapeID());
			CHECK(!collector.HadHit());
		}

		// A triangle shape has all edges active, so only test if all edges are active
		if (inActiveEdges == 0b111)
		{
			// Create the triangle shape
			PhysicsTestContext context;
			context.CreateBody(new TriangleShapeSettings(v1, v2, v3), RVec3::sZero(), Quat::sIdentity(), EMotionType::Static, EMotionQuality::Discrete, Layers::NON_MOVING, EActivation::DontActivate);

			// Collide sphere
			AllHitCollisionCollector<CollideShapeCollector> collector;
			context.GetSystem()->GetNarrowPhaseQuery().CollideShape(sphere, Vec3::sReplicate(1.0f), RMat44::sTranslation(RVec3(inCenter)), inSettings, RVec3::sZero(), collector);
			CHECK(!collector.HadHit());
		}

		// A mesh shape with a single triangle has all edges active, so only test if all edges are active
		if (inActiveEdges == 0b111)
		{
			// Create a mesh with a single triangle
			TriangleList triangles;
			triangles.push_back(Triangle(v1, v2, v3));
			PhysicsTestContext context;
			context.CreateBody(new MeshShapeSettings(triangles), RVec3::sZero(), Quat::sIdentity(), EMotionType::Static, EMotionQuality::Discrete, Layers::NON_MOVING, EActivation::DontActivate);

			// Collide sphere
			AllHitCollisionCollector<CollideShapeCollector> collector;
			context.GetSystem()->GetNarrowPhaseQuery().CollideShape(sphere, Vec3::sReplicate(1.0f), RMat44::sTranslation(RVec3(inCenter)), inSettings, RVec3::sZero(), collector);
			CHECK(!collector.HadHit());
		}
	}

	template <class Collider>
	static void sCheckCollision(const CollideShapeSettings &inSettings, Vec3Arg inCenter, float inRadius, uint8 inActiveEdges, Vec3Arg inExpectedContactOn1, Vec3Arg inExpectedContactOn2, Vec3Arg inExpectedPenetrationAxis, float inExpectedPenetrationDepth)
	{
		// Our sphere
		Ref<SphereShape> sphere = new SphereShape(inRadius);

		// Our default triangle
		Vec3 v1(0, 0, 0);
		Vec3 v2(0, 0, cEdgeLength);
		Vec3 v3(cEdgeLength, 0, 0);

		// A semi random transform for the triangle
		Vec3 translation = Vec3(1, 2, 3);
		Quat rotation = Quat::sRotation(Vec3::sAxisX(), 0.25f * JPH_PI);
		Mat44 transform = Mat44::sRotationTranslation(rotation, translation);
		Mat44 inv_transform = transform.InversedRotationTranslation();

		// The transform for the sphere
		Mat44 sphere_transform = transform * Mat44::sTranslation(inCenter);

		// Transform incoming settings
		CollideShapeSettings settings = inSettings;
		settings.mActiveEdgeMovementDirection = transform.Multiply3x3(inSettings.mActiveEdgeMovementDirection);

		// Test the specified collider
		{
			SubShapeID sub_shape_id1, sub_shape_id2;
			sub_shape_id1.SetValue(123);
			sub_shape_id2.SetValue(456);

			// Collide sphere
			AllHitCollisionCollector<CollideShapeCollector> collector;
			Collider collider(sphere, Vec3::sReplicate(1.0f), Vec3::sReplicate(1.0f), sphere_transform, transform, sub_shape_id1, settings, collector);
			collider.Collide(v1, v2, v3, inActiveEdges, sub_shape_id2);

			// Test result
			CHECK(collector.mHits.size() == 1);
			const CollideShapeResult &hit = collector.mHits[0];
			CHECK(hit.mBodyID2 == BodyID());
			CHECK(hit.mSubShapeID1.GetValue() == sub_shape_id1.GetValue());
			CHECK(hit.mSubShapeID2.GetValue() == sub_shape_id2.GetValue());
			Vec3 contact1 = inv_transform * hit.mContactPointOn1;
			Vec3 contact2 = inv_transform * hit.mContactPointOn2;
			Vec3 pen_axis = transform.Multiply3x3Transposed(hit.mPenetrationAxis).Normalized();
			Vec3 expected_pen_axis = inExpectedPenetrationAxis.Normalized();
			CHECK_APPROX_EQUAL(contact1, inExpectedContactOn1, 1.0e-4f);
			CHECK_APPROX_EQUAL(contact2, inExpectedContactOn2, 1.0e-4f);
			CHECK_APPROX_EQUAL(pen_axis, expected_pen_axis, 1.0e-4f);
			CHECK_APPROX_EQUAL(hit.mPenetrationDepth, inExpectedPenetrationDepth, 1.0e-4f);
		}

		// A triangle shape has all edges active, so only test if all edges are active
		if (inActiveEdges == 0b111)
		{
			// Create the triangle shape
			PhysicsTestContext context;
			Body &body = context.CreateBody(new TriangleShapeSettings(v1, v2, v3), RVec3(translation), rotation, EMotionType::Static, EMotionQuality::Discrete, Layers::NON_MOVING, EActivation::DontActivate);

			// Collide sphere
			AllHitCollisionCollector<CollideShapeCollector> collector;
			context.GetSystem()->GetNarrowPhaseQuery().CollideShape(sphere, Vec3::sReplicate(1.0f), RMat44(sphere_transform), settings, RVec3::sZero(), collector);

			// Test result
			CHECK(collector.mHits.size() == 1);
			const CollideShapeResult &hit = collector.mHits[0];
			CHECK(hit.mBodyID2 == body.GetID());
			CHECK(hit.mSubShapeID1.GetValue() == SubShapeID().GetValue());
			CHECK(hit.mSubShapeID2.GetValue() == SubShapeID().GetValue());
			Vec3 contact1 = inv_transform * hit.mContactPointOn1;
			Vec3 contact2 = inv_transform * hit.mContactPointOn2;
			Vec3 pen_axis = transform.Multiply3x3Transposed(hit.mPenetrationAxis).Normalized();
			Vec3 expected_pen_axis = inExpectedPenetrationAxis.Normalized();
			CHECK_APPROX_EQUAL(contact1, inExpectedContactOn1, 1.0e-4f);
			CHECK_APPROX_EQUAL(contact2, inExpectedContactOn2, 1.0e-4f);
			CHECK_APPROX_EQUAL(pen_axis, expected_pen_axis, 1.0e-4f);
			CHECK_APPROX_EQUAL(hit.mPenetrationDepth, inExpectedPenetrationDepth, 1.0e-4f);
		}

		// A mesh shape with a single triangle has all edges active, so only test if all edges are active
		if (inActiveEdges == 0b111)
		{
			// Create a mesh with a single triangle
			TriangleList triangles;
			triangles.push_back(Triangle(v1, v2, v3));
			PhysicsTestContext context;
			Body &body = context.CreateBody(new MeshShapeSettings(triangles), RVec3(translation), rotation, EMotionType::Static, EMotionQuality::Discrete, Layers::NON_MOVING, EActivation::DontActivate);

			// Collide sphere
			AllHitCollisionCollector<CollideShapeCollector> collector;
			context.GetSystem()->GetNarrowPhaseQuery().CollideShape(sphere, Vec3::sReplicate(1.0f), RMat44(sphere_transform), settings, RVec3::sZero(), collector);

			// Test result
			CHECK(collector.mHits.size() == 1);
			const CollideShapeResult &hit = collector.mHits[0];
			CHECK(hit.mBodyID2 == body.GetID());
			CHECK(hit.mSubShapeID1.GetValue() == SubShapeID().GetValue());
			CHECK(hit.mSubShapeID2.GetValue() != SubShapeID().GetValue()); // We don't really know what SubShapeID a triangle in the mesh will get, but it should not be invalid
			Vec3 contact1 = inv_transform * hit.mContactPointOn1;
			Vec3 contact2 = inv_transform * hit.mContactPointOn2;
			Vec3 pen_axis = transform.Multiply3x3Transposed(hit.mPenetrationAxis).Normalized();
			Vec3 expected_pen_axis = inExpectedPenetrationAxis.Normalized();
			CHECK_APPROX_EQUAL(contact1, inExpectedContactOn1, 1.0e-4f);
			CHECK_APPROX_EQUAL(contact2, inExpectedContactOn2, 1.0e-4f);
			CHECK_APPROX_EQUAL(pen_axis, expected_pen_axis, 1.0e-4f);
			CHECK_APPROX_EQUAL(hit.mPenetrationDepth, inExpectedPenetrationDepth, 1.0e-4f);
		}
	}

	// Compares CollideShapeResult for two spheres with given positions and radii
	template <class Collider>
	static void sTestConvexVsTriangles()
	{
		const float cRadius = 0.5f;
		const float cRadiusRS2 = cRadius / sqrt(2.0f);
		const float cDistanceToTriangle = 0.1f;
		const float cDistanceToTriangleRS2 = cDistanceToTriangle / sqrt(2.0f);
		const float cEpsilon = 1.0e-6f; // A small epsilon to ensure we hit the front side
		const float cMaxSeparationDistance = 0.5f;
		const float cSeparationDistance = 0.1f;

		// Loop over all possible active edge combinations
		for (uint8 active_edges = 0; active_edges <= 0b111; ++active_edges)
		{
			// Create settings
			CollideShapeSettings settings;
			settings.mBackFaceMode = EBackFaceMode::CollideWithBackFaces;

			// Settings with ignore back faces
			CollideShapeSettings settings_no_bf;
			settings_no_bf.mBackFaceMode = EBackFaceMode::IgnoreBackFaces;

			// Settings with max seperation distance
			CollideShapeSettings settings_max_distance;
			settings_max_distance.mBackFaceMode = EBackFaceMode::CollideWithBackFaces;
			settings_max_distance.mMaxSeparationDistance = cMaxSeparationDistance;

			{
				// There should be no hit in front of the triangle
				Vec3 sphere_center(0.25f * cEdgeLength, cRadius + cSeparationDistance, 0.25f * cEdgeLength);
				sCheckCollisionNoHit<Collider>(settings, sphere_center, cRadius, active_edges);

				// But if there's a max separation distance there should be
				Vec3 expected1 = sphere_center + Vec3(0, -cRadius, 0);
				Vec3 expected2(0.25f * cEdgeLength, 0, 0.25f * cEdgeLength);
				Vec3 pen_axis(0, -1, 0);
				float pen_depth = -cSeparationDistance;
				sCheckCollision<Collider>(settings_max_distance, sphere_center, cRadius, active_edges, expected1, expected2, pen_axis, pen_depth);
			}

			{
				// But if we go beyond the separation distance we should again have no hit
				Vec3 sphere_center(0.25f * cEdgeLength, cRadius + cMaxSeparationDistance + cSeparationDistance, 0.25f * cEdgeLength);
				sCheckCollisionNoHit<Collider>(settings_max_distance, sphere_center, cRadius, active_edges);
			}

			{
				// There should be no hit in behind the triangle
				Vec3 sphere_center(0.25f * cEdgeLength, -cRadius - cSeparationDistance, 0.25f * cEdgeLength);
				sCheckCollisionNoHit<Collider>(settings, sphere_center, cRadius, active_edges);

				// But if there's a max separation distance there should be
				Vec3 expected1 = sphere_center + Vec3(0, cRadius, 0);
				Vec3 expected2(0.25f * cEdgeLength, 0, 0.25f * cEdgeLength);
				Vec3 pen_axis(0, 1, 0);
				float pen_depth = -cSeparationDistance;
				sCheckCollision<Collider>(settings_max_distance, sphere_center, cRadius, active_edges, expected1, expected2, pen_axis, pen_depth);
			}

			{
				// But if we go beyond the separation distance we should again have no hit
				Vec3 sphere_center(0.25f * cEdgeLength, -cRadius - cMaxSeparationDistance - cSeparationDistance, 0.25f * cEdgeLength);
				sCheckCollisionNoHit<Collider>(settings_max_distance, sphere_center, cRadius, active_edges);
			}

			{
				// Hit interior from front side
				Vec3 expected2(0.25f * cEdgeLength, 0, 0.25f * cEdgeLength);
				Vec3 sphere_center = expected2 + Vec3(0, cDistanceToTriangle, 0);
				Vec3 expected1 = sphere_center + Vec3(0, -cRadius, 0);
				Vec3 pen_axis(0, -1, 0);
				float pen_depth = cRadius - cDistanceToTriangle;
				sCheckCollision<Collider>(settings, sphere_center, cRadius, active_edges, expected1, expected2, pen_axis, pen_depth);

				// Ignore back faces should not matter
				sCheckCollision<Collider>(settings_no_bf, sphere_center, cRadius, active_edges, expected1, expected2, pen_axis, pen_depth);
			}

			{
				// Hit interior from back side
				Vec3 expected2(0.25f * cEdgeLength, 0, 0.25f * cEdgeLength);
				Vec3 sphere_center = expected2 + Vec3(0, -cDistanceToTriangle, 0);
				Vec3 expected1 = sphere_center + Vec3(0, cRadius, 0);
				Vec3 pen_axis(0, 1, 0);
				float pen_depth = cRadius - cDistanceToTriangle;
				sCheckCollision<Collider>(settings, sphere_center, cRadius, active_edges, expected1, expected2, pen_axis, pen_depth);

				// Back face hit should be filtered
				sCheckCollisionNoHit<Collider>(settings_no_bf, sphere_center, cRadius, active_edges);
			}

			// Loop over possibel active edge movement direction permutations
			for (int movement_direction = 0; movement_direction < 3; ++movement_direction)
			{
				switch (movement_direction)
				{
				case 0:
					// Disable the system
					settings.mActiveEdgeMovementDirection = Vec3::sZero();
					break;

				case 1:
					// Move into the triangle, this should always give us the normal from the edge
					settings.mActiveEdgeMovementDirection = Vec3(0, -1, 0);
					break;

				case 2:
					// Move out of the triangle, we should always get the normal of the triangle
					settings.mActiveEdgeMovementDirection = Vec3(0, 1, 0);
					break;
				}

				{
					// Hit edge 1
					Vec3 expected2(0, 0, 0.5f * cEdgeLength);
					Vec3 sphere_center = expected2 + Vec3(-cDistanceToTriangle, cEpsilon, 0);
					Vec3 expected1 = sphere_center + Vec3(cRadius, 0, 0);
					Vec3 pen_axis = (active_edges & 0b001) != 0 || movement_direction == 1? Vec3(1, 0, 0) : Vec3(0, -1, 0);
					float pen_depth = cRadius - cDistanceToTriangle;
					sCheckCollision<Collider>(settings, sphere_center, cRadius, active_edges, expected1, expected2, pen_axis, pen_depth);
				}

				{
					// Hit edge 2
					Vec3 expected2(0.5f * cEdgeLength, 0, 0.5f * cEdgeLength);
					Vec3 sphere_center = expected2 + Vec3(cDistanceToTriangleRS2, cEpsilon, cDistanceToTriangleRS2);
					Vec3 expected1 = sphere_center - Vec3(cRadiusRS2, 0, cRadiusRS2);
					Vec3 pen_axis = (active_edges & 0b010) != 0 || movement_direction == 1? Vec3(-1, 0, -1) : Vec3(0, -1, 0);
					float pen_depth = cRadius - cDistanceToTriangle;
					sCheckCollision<Collider>(settings, sphere_center, cRadius, active_edges, expected1, expected2, pen_axis, pen_depth);
				}

				{
					// Hit edge 3
					Vec3 expected2(0.5f * cEdgeLength, 0, 0);
					Vec3 sphere_center = expected2 + Vec3(0, cEpsilon, -cDistanceToTriangle);
					Vec3 expected1 = sphere_center + Vec3(0, 0, cRadius);
					Vec3 pen_axis = (active_edges & 0b100) != 0 || movement_direction == 1? Vec3(0, 0, 1) : Vec3(0, -1, 0);
					float pen_depth = cRadius - cDistanceToTriangle;
					sCheckCollision<Collider>(settings, sphere_center, cRadius, active_edges, expected1, expected2, pen_axis, pen_depth);
				}

				{
					// Hit vertex 1
					Vec3 expected2(0, 0, 0);
					Vec3 sphere_center = expected2 + Vec3(-cDistanceToTriangleRS2, cEpsilon, -cDistanceToTriangleRS2);
					Vec3 expected1 = sphere_center + Vec3(cRadiusRS2, 0, cRadiusRS2);
					Vec3 pen_axis = (active_edges & 0b101) != 0 || movement_direction == 1? Vec3(1, 0, 1) : Vec3(0, -1, 0);
					float pen_depth = cRadius - cDistanceToTriangle;
					sCheckCollision<Collider>(settings, sphere_center, cRadius, active_edges, expected1, expected2, pen_axis, pen_depth);
				}

				{
					// Hit vertex 2
					Vec3 expected2(0, 0, cEdgeLength);
					Vec3 sphere_center = expected2 + Vec3(-cDistanceToTriangleRS2, cEpsilon, cDistanceToTriangleRS2);
					Vec3 expected1 = sphere_center + Vec3(cRadiusRS2, 0, -cRadiusRS2);
					Vec3 pen_axis = (active_edges & 0b011) != 0 || movement_direction == 1? Vec3(1, 0, -1) : Vec3(0, -1, 0);
					float pen_depth = cRadius - cDistanceToTriangle;
					sCheckCollision<Collider>(settings, sphere_center, cRadius, active_edges, expected1, expected2, pen_axis, pen_depth);
				}

				{
					// Hit vertex 3
					Vec3 expected2(cEdgeLength, 0, 0);
					Vec3 sphere_center = expected2 + Vec3(cDistanceToTriangleRS2, cEpsilon, -cDistanceToTriangleRS2);
					Vec3 expected1 = sphere_center + Vec3(-cRadiusRS2, 0, cRadiusRS2);
					Vec3 pen_axis = (active_edges & 0b110) != 0 || movement_direction == 1? Vec3(-1, 0, 1) : Vec3(0, -1, 0);
					float pen_depth = cRadius - cDistanceToTriangle;
					sCheckCollision<Collider>(settings, sphere_center, cRadius, active_edges, expected1, expected2, pen_axis, pen_depth);
				}
			}
		}
	}

	TEST_CASE("TestConvexVsTriangles")
	{
		sTestConvexVsTriangles<CollideConvexVsTriangles>();
	}

	TEST_CASE("TestSphereVsTriangles")
	{
		sTestConvexVsTriangles<CollideSphereVsTriangles>();
	}
}
