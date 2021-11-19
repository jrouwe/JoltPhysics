// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include "PhysicsTestContext.h"
#include "Layers.h"
#include <Physics/Collision/Shape/MeshShape.h>
#include <Physics/Collision/CollisionCollectorImpl.h>
#include <Physics/Collision/CollidePointResult.h>

TEST_SUITE("CollidePointTests")
{
	// Test the test framework's helper functions
	TEST_CASE("TestCollidePointVsMesh")
	{
		// Vertices of a cube
		Vec3 vertices[] = {
			Vec3(-1.0f, -1.0f, -1.0f),
			Vec3( 1.0f, -1.0f, -1.0f),
			Vec3(-1.0f, -1.0f,  1.0f),
			Vec3( 1.0f, -1.0f,  1.0f),
			Vec3(-1.0f,  1.0f, -1.0f),
			Vec3( 1.0f,  1.0f, -1.0f),
			Vec3(-1.0f,  1.0f,  1.0f),
			Vec3( 1.0f,  1.0f,  1.0f)
		};

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

		// Probe directions in the direction of the faces
		Vec3 probes[] = {
			Vec3(-1.0f, 0, 0),
			Vec3(1.0f, 0, 0),
			Vec3(0, -1.0f, 0),
			Vec3(0, 1.0f, 0),
			Vec3(0, 0, -1.0f),
			Vec3(0, 0, 1.0f)
		};

		const int grid_size = 2;

		UnitTestRandom random;
		uniform_real_distribution<float> range(0.1f, 0.3f);

		// Create a grid of closed shapes
		MeshShapeSettings settings;
		settings.SetEmbedded();
		int num_cubes = Cubed(2 * grid_size + 1);
		settings.mTriangleVertices.reserve(num_cubes * size(vertices));
		settings.mIndexedTriangles.reserve(num_cubes * size(indices));
		for (int x = -grid_size; x <= grid_size; ++x)
			for (int y = -grid_size; y <= grid_size; ++y)
				for (int z = -grid_size; z <= grid_size; ++z)
				{
					Vec3 center((float)x, (float)y, (float)z);

					// Create vertices with randomness
					uint vtx = (uint)settings.mTriangleVertices.size();
					settings.mTriangleVertices.resize(vtx + size(vertices));
					for (uint i = 0; i < size(vertices); ++i)
					{
						Vec3 vertex(center + vertices[i] * Vec3(range(random), range(random), range(random)));
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
		Body &mesh_body = context.CreateBody(&settings, Vec3::sRandom(random), Quat::sRandom(random), EMotionType::Static, EMotionQuality::Discrete, Layers::NON_MOVING, EActivation::DontActivate);

		// Get the shape
		ShapeRefC mesh_shape = mesh_body.GetShape();

		// Get narrow phase
		const NarrowPhaseQuery &narrow_phase = context.GetSystem()->GetNarrowPhaseQuery();

		// Get transform
		Mat44 body_transform = mesh_body.GetWorldTransform();
		CHECK(body_transform != Mat44::sIdentity());

		// Test points
		for (int x = -grid_size; x <= grid_size; ++x)
			for (int y = -grid_size; y <= grid_size; ++y)
				for (int z = -grid_size; z <= grid_size; ++z)
				{
					Vec3 center((float)x, (float)y, (float)z);

					// The center point should hit
					{
						// Test shape directly
						AllHitCollisionCollector<CollidePointCollector> collector;
						mesh_shape->CollidePoint(center, SubShapeIDCreator(), collector);
						CHECK(collector.mHits.size() == 1);

						// Reset collector
						collector.Reset();
						CHECK(collector.mHits.empty());

						// Also not when going through the narrow phase
						narrow_phase.CollidePoint(body_transform * center, collector);
						CHECK(collector.mHits.size() == 1);
						CHECK(collector.mHits[0].mBodyID == mesh_body.GetID());
					}

					// Points outside the hull should not hit
					for (Vec3 probe : probes)
					{
						// Test shape directly
						AllHitCollisionCollector<CollidePointCollector> collector;
						Vec3 point = center + 0.4f * probe;
						mesh_shape->CollidePoint(point, SubShapeIDCreator(), collector);
						CHECK(collector.mHits.empty());

						// Reset collector
						collector.Reset();
						CHECK(collector.mHits.empty());

						// Also not when going through the narrow phase
						narrow_phase.CollidePoint(body_transform * point, collector);
						CHECK(collector.mHits.empty());
					}
				}
	}
}