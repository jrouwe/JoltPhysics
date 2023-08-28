// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include "PhysicsTestContext.h"
#include "Layers.h"
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/CylinderShape.h>
#include <Jolt/Physics/Collision/Shape/TaperedCapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/Collision/Shape/OffsetCenterOfMassShape.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>
#include <Jolt/Physics/Collision/Shape/MutableCompoundShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Jolt/Physics/Collision/CollidePointResult.h>

TEST_SUITE("CollidePointTests")
{
	// Probe directions in the direction of the faces
	static Vec3 cube_probes[] = {
		Vec3(-1.0f, 0, 0),
		Vec3(1.0f, 0, 0),
		Vec3(0, -1.0f, 0),
		Vec3(0, 1.0f, 0),
		Vec3(0, 0, -1.0f),
		Vec3(0, 0, 1.0f)
	};

	// Probe directions in the direction of the faces
	static Vec3 cube_and_zero_probes[] = {
		Vec3(0, 0, 0),
		Vec3(-1.0f, 0, 0),
		Vec3(1.0f, 0, 0),
		Vec3(0, -1.0f, 0),
		Vec3(0, 1.0f, 0),
		Vec3(0, 0, -1.0f),
		Vec3(0, 0, 1.0f)
	};

	// Probes in the xy-plane
	static Vec3 xy_probes[] = {
		Vec3(-1.0f, 0, 0),
		Vec3(1.0f, 0, 0),
		Vec3(0, 0, -1.0f),
		Vec3(0, 0, 1.0f)
	};

	// Probes in the xy-plane and zero
	static Vec3 xy_and_zero_probes[] = {
		Vec3(0, 0, 0),
		Vec3(-1.0f, 0, 0),
		Vec3(1.0f, 0, 0),
		Vec3(0, 0, -1.0f),
		Vec3(0, 0, 1.0f)
	};

	// Vertices of a cube
	static Vec3 cube_vertices[] = {
		Vec3(-1.0f, -1.0f, -1.0f),
		Vec3( 1.0f, -1.0f, -1.0f),
		Vec3(-1.0f, -1.0f,  1.0f),
		Vec3( 1.0f, -1.0f,  1.0f),
		Vec3(-1.0f,  1.0f, -1.0f),
		Vec3( 1.0f,  1.0f, -1.0f),
		Vec3(-1.0f,  1.0f,  1.0f),
		Vec3( 1.0f,  1.0f,  1.0f)
	};

	static void sTestHit(const Shape *inShape, Vec3Arg inPosition)
	{
		AllHitCollisionCollector<CollidePointCollector> collector;
		inShape->CollidePoint(inPosition - inShape->GetCenterOfMass(), SubShapeIDCreator(), collector);
		CHECK(collector.mHits.size() == 1);
	}

	static void sTestHit(const NarrowPhaseQuery &inNarrowPhase, RVec3Arg inPosition, const BodyID &inBodyID)
	{
		AllHitCollisionCollector<CollidePointCollector> collector;
		inNarrowPhase.CollidePoint(inPosition, collector);
		CHECK(collector.mHits.size() == 1);
		CHECK(collector.mHits[0].mBodyID == inBodyID);
	}

	static void sTestMiss(const Shape *inShape, Vec3Arg inPosition)
	{
		AllHitCollisionCollector<CollidePointCollector> collector;
		inShape->CollidePoint(inPosition - inShape->GetCenterOfMass(), SubShapeIDCreator(), collector);
		CHECK(collector.mHits.empty());
	}

	static void sTestMiss(const NarrowPhaseQuery &inNarrowPhase, RVec3Arg inPosition)
	{
		AllHitCollisionCollector<CollidePointCollector> collector;
		inNarrowPhase.CollidePoint(inPosition, collector);
		CHECK(collector.mHits.empty());
	}

	TEST_CASE("TestCollidePointVsBox")
	{
		Vec3 half_box_size(0.1f, 0.2f, 0.3f);
		ShapeRefC shape = new BoxShape(half_box_size);

		// Hits
		for (Vec3 probe : cube_and_zero_probes)
			sTestHit(shape, 0.99f * half_box_size * probe);

		// Misses
		for (Vec3 probe : cube_probes)
			sTestMiss(shape, 1.01f * half_box_size * probe);
	}

	TEST_CASE("TestCollidePointVsSphere")
	{
		const float radius = 0.1f;
		ShapeRefC shape = new SphereShape(radius);

		// Hits
		for (Vec3 probe : cube_and_zero_probes)
			sTestHit(shape, 0.99f * Vec3::sReplicate(radius) * probe);

		// Misses
		for (Vec3 probe : cube_probes)
			sTestMiss(shape, 1.01f * Vec3::sReplicate(radius) * probe);
	}

	TEST_CASE("TestCollidePointVsCapsule")
	{
		const float half_height = 0.2f;
		const float radius = 0.1f;
		ShapeRefC shape = new CapsuleShape(half_height, radius);

		// Top hits
		for (Vec3 probe : xy_and_zero_probes)
			sTestHit(shape, 0.99f * radius * probe + Vec3(0, half_height, 0));

		// Center hit
		sTestHit(shape, Vec3::sZero());

		// Bottom hits
		for (Vec3 probe : xy_and_zero_probes)
			sTestHit(shape, 0.99f * radius * probe + Vec3(0, -half_height, 0));

		// Misses
		for (Vec3 probe : cube_probes)
			sTestMiss(shape, 1.01f * Vec3(radius, half_height + radius, radius) * probe);
	}

	TEST_CASE("TestCollidePointVsTaperedCapsule")
	{
		const float half_height = 0.4f;
		const float top_radius = 0.1f;
		const float bottom_radius = 0.2f;
		TaperedCapsuleShapeSettings settings(half_height, top_radius, bottom_radius);
		ShapeRefC shape = settings.Create().Get();

		// Top hits
		for (Vec3 probe : xy_and_zero_probes)
			sTestHit(shape, 0.99f * top_radius * probe + Vec3(0, half_height, 0));

		// Center hit
		sTestHit(shape, Vec3::sZero());

		// Bottom hits
		for (Vec3 probe : xy_and_zero_probes)
			sTestHit(shape, 0.99f * bottom_radius * probe + Vec3(0, -half_height, 0));

		// Top misses
		sTestMiss(shape, Vec3(0, half_height + top_radius + 0.01f, 0));
		for (Vec3 probe : xy_probes)
			sTestMiss(shape, 1.01f * top_radius * probe + Vec3(0, half_height, 0));

		// Bottom misses
		sTestMiss(shape, Vec3(0, -half_height - bottom_radius - 0.01f, 0));
		for (Vec3 probe : xy_probes)
			sTestMiss(shape, 1.01f * bottom_radius * probe + Vec3(0, -half_height, 0));
	}

	TEST_CASE("TestCollidePointVsCylinder")
	{
		const float half_height = 0.2f;
		const float radius = 0.1f;
		ShapeRefC shape = new CylinderShape(half_height, radius);

		// Top hits
		for (Vec3 probe : xy_and_zero_probes)
			sTestHit(shape, 0.99f * (radius * probe + Vec3(0, half_height, 0)));

		// Center hit
		sTestHit(shape, Vec3::sZero());

		// Bottom hits
		for (Vec3 probe : xy_and_zero_probes)
			sTestHit(shape, 0.99f * (radius * probe + Vec3(0, -half_height, 0)));

		// Misses
		for (Vec3 probe : cube_probes)
			sTestMiss(shape, 1.01f * Vec3(radius, half_height, radius) * probe);
	}

	TEST_CASE("TestCollidePointVsConvexHull")
	{
		Vec3 half_box_size(0.1f, 0.2f, 0.3f);
		Vec3 offset(10.0f, 11.0f, 12.0f);

		ConvexHullShapeSettings settings;
		for (uint i = 0; i < size(cube_vertices); ++i)
			settings.mPoints.push_back(offset + cube_vertices[i] * half_box_size);
		ShapeRefC shape = settings.Create().Get();

		// Hits
		for (Vec3 probe : cube_and_zero_probes)
			sTestHit(shape, offset + 0.99f * half_box_size * probe);

		// Misses
		for (Vec3 probe : cube_probes)
			sTestMiss(shape, offset + 1.01f * half_box_size * probe);
	}

	TEST_CASE("TestCollidePointVsRotatedTranslated")
	{
		Vec3 translation(10.0f, 11.0f, 12.0f);
		Quat rotation = Quat::sRotation(Vec3(1, 2, 3).Normalized(), 0.3f * JPH_PI);
		Mat44 transform = Mat44::sRotationTranslation(rotation, translation);

		Vec3 half_box_size(0.1f, 0.2f, 0.3f);
		RotatedTranslatedShapeSettings settings(translation, rotation, new BoxShape(half_box_size));
		ShapeRefC shape = settings.Create().Get();

		// Hits
		for (Vec3 probe : cube_and_zero_probes)
			sTestHit(shape, transform * (0.99f * half_box_size * probe));

		// Misses
		for (Vec3 probe : cube_probes)
			sTestMiss(shape, transform * (1.01f * half_box_size * probe));
	}

	TEST_CASE("TestCollidePointVsScaled")
	{
		Vec3 scale(2.0f, 3.0f, -4.0f);
		Vec3 half_box_size(0.1f, 0.2f, 0.3f);
		ShapeRefC shape = new ScaledShape(new BoxShape(half_box_size), scale);

		// Hits
		for (Vec3 probe : cube_and_zero_probes)
			sTestHit(shape, scale * (0.99f * half_box_size * probe));

		// Misses
		for (Vec3 probe : cube_probes)
			sTestMiss(shape, scale * (1.01f * half_box_size * probe));
	}

	TEST_CASE("TestCollidePointVsOffsetCenterOfMass")
	{
		Vec3 offset(10.0f, 11.0f, 12.0f);
		Vec3 half_box_size(0.1f, 0.2f, 0.3f);
		OffsetCenterOfMassShapeSettings settings(offset, new BoxShape(half_box_size));
		ShapeRefC shape = settings.Create().Get();

		// Hits
		for (Vec3 probe : cube_and_zero_probes)
			sTestHit(shape, 0.99f * half_box_size * probe);

		// Misses
		for (Vec3 probe : cube_probes)
			sTestMiss(shape, 1.01f * half_box_size * probe);
	}

	TEST_CASE("TestCollidePointVsStaticCompound")
	{
		Vec3 translation1(10.0f, 11.0f, 12.0f);
		Quat rotation1 = Quat::sRotation(Vec3(1, 2, 3).Normalized(), 0.3f * JPH_PI);
		Mat44 transform1 = Mat44::sRotationTranslation(rotation1, translation1);

		Vec3 translation2(-1.0f, -2.0f, -3.0f);
		Quat rotation2 = Quat::sRotation(Vec3(4, 5, 6).Normalized(), 0.2f * JPH_PI);
		Mat44 transform2 = Mat44::sRotationTranslation(rotation2, translation2);

		Vec3 half_box_size(0.1f, 0.2f, 0.3f);
		ShapeRefC box = new BoxShape(half_box_size);

		StaticCompoundShapeSettings settings;
		settings.AddShape(translation1, rotation1, box);
		settings.AddShape(translation2, rotation2, box);
		ShapeRefC shape = settings.Create().Get();

		// Hits
		for (Vec3 probe : cube_and_zero_probes)
		{
			Vec3 point = 0.99f * half_box_size * probe;
			sTestHit(shape, transform1 * point);
			sTestHit(shape, transform2 * point);
		}

		// Misses
		for (Vec3 probe : cube_probes)
		{
			Vec3 point = 1.01f * half_box_size * probe;
			sTestMiss(shape, transform1 * point);
			sTestMiss(shape, transform2 * point);
		}
	}

	TEST_CASE("TestCollidePointVsMutableCompound")
	{
		Vec3 translation1(10.0f, 11.0f, 12.0f);
		Quat rotation1 = Quat::sRotation(Vec3(1, 2, 3).Normalized(), 0.3f * JPH_PI);
		Mat44 transform1 = Mat44::sRotationTranslation(rotation1, translation1);

		Vec3 translation2(-1.0f, -2.0f, -3.0f);
		Quat rotation2 = Quat::sRotation(Vec3(4, 5, 6).Normalized(), 0.2f * JPH_PI);
		Mat44 transform2 = Mat44::sRotationTranslation(rotation2, translation2);

		Vec3 half_box_size(0.1f, 0.2f, 0.3f);
		ShapeRefC box = new BoxShape(half_box_size);

		MutableCompoundShapeSettings settings;
		settings.AddShape(translation1, rotation1, box);
		settings.AddShape(translation2, rotation2, box);
		ShapeRefC shape = settings.Create().Get();

		// Hits
		for (Vec3 probe : cube_and_zero_probes)
		{
			Vec3 point = 0.99f * half_box_size * probe;
			sTestHit(shape, transform1 * point);
			sTestHit(shape, transform2 * point);
		}

		// Misses
		for (Vec3 probe : cube_probes)
		{
			Vec3 point = 1.01f * half_box_size * probe;
			sTestMiss(shape, transform1 * point);
			sTestMiss(shape, transform2 * point);
		}
	}

	TEST_CASE("TestCollidePointVsMesh")
	{
		// Face indices of a cube
		int indices[][3] = {
			{ 0, 1, 3 },
			{ 0, 3, 2 },
			{ 4, 7, 5 },
			{ 4, 6, 7 },
			{ 2, 3, 6 },
			{ 3, 7, 6 },
			{ 1, 0, 4 },
			{ 1, 4, 5 },
			{ 1, 7, 3 },
			{ 1, 5, 7 },
			{ 0, 2, 6 },
			{ 0, 6, 4 }
		};

		const int grid_size = 2;

		UnitTestRandom random;
		uniform_real_distribution<float> range(0.1f, 0.3f);

		// Create a grid of closed shapes
		MeshShapeSettings settings;
		settings.SetEmbedded();
		int num_cubes = Cubed(2 * grid_size + 1);
		settings.mTriangleVertices.reserve(num_cubes * size(cube_vertices));
		settings.mIndexedTriangles.reserve(num_cubes * size(indices));
		for (int x = -grid_size; x <= grid_size; ++x)
			for (int y = -grid_size; y <= grid_size; ++y)
				for (int z = -grid_size; z <= grid_size; ++z)
				{
					Vec3 center((float)x, (float)y, (float)z);

					// Create vertices with randomness
					uint vtx = (uint)settings.mTriangleVertices.size();
					settings.mTriangleVertices.resize(vtx + size(cube_vertices));
					for (uint i = 0; i < size(cube_vertices); ++i)
					{
						Vec3 vertex(center + cube_vertices[i] * Vec3(range(random), range(random), range(random)));
						vertex.StoreFloat3(&settings.mTriangleVertices[vtx + i]);
					}

					// Flip inside out? (inside out shapes should act the same as normal shapes for CollidePoint)
					bool flip = (y & 1) == 0;

					// Create face indices
					uint idx = (uint)settings.mIndexedTriangles.size();
					settings.mIndexedTriangles.resize(idx + size(indices));
					for (uint i = 0; i < size(indices); ++i)
						settings.mIndexedTriangles[idx + i] = IndexedTriangle(vtx + indices[i][0], vtx + indices[i][flip? 2 : 1], vtx + indices[i][flip? 1 : 2]);
				}

		// Create body with random orientation
		PhysicsTestContext context;
		Body &mesh_body = context.CreateBody(&settings, RVec3(Vec3::sRandom(random)), Quat::sRandom(random), EMotionType::Static, EMotionQuality::Discrete, Layers::NON_MOVING, EActivation::DontActivate);

		// Get the shape
		ShapeRefC mesh_shape = mesh_body.GetShape();

		// Get narrow phase
		const NarrowPhaseQuery &narrow_phase = context.GetSystem()->GetNarrowPhaseQuery();

		// Get transform
		RMat44 body_transform = mesh_body.GetWorldTransform();
		CHECK(body_transform != RMat44::sIdentity());

		// Test points
		for (int x = -grid_size; x <= grid_size; ++x)
			for (int y = -grid_size; y <= grid_size; ++y)
				for (int z = -grid_size; z <= grid_size; ++z)
				{
					Vec3 center((float)x, (float)y, (float)z);

					// The center point should hit
					sTestHit(mesh_shape, center);
					sTestHit(narrow_phase, body_transform * center, mesh_body.GetID());

					// Points outside the hull should not hit
					for (Vec3 probe : cube_probes)
					{
						Vec3 point = center + 0.4f * probe;
						sTestMiss(mesh_shape, point);
						sTestMiss(narrow_phase, body_transform * point);
					}
				}
	}
}
