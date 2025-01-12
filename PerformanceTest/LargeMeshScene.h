// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

// Jolt includes
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>

// Local includes
#include "PerformanceTestScene.h"
#include "Layers.h"

// A scene that first finds the largest possible mesh and then simulates some objects on it
class LargeMeshScene : public PerformanceTestScene
{
public:
	virtual const char *	GetName() const override
	{
		return "LargeMeshScene";
	}

	virtual bool			Load(const String &inAssetPath) override
	{
		// Create mesh shape creation settings
		mMeshCreationSettings.mMotionType = EMotionType::Static;
		mMeshCreationSettings.mObjectLayer = Layers::NON_MOVING;
		mMeshCreationSettings.mPosition = RVec3::sZero();
		mMeshCreationSettings.mFriction = 0.5f;
		mMeshCreationSettings.mRestitution = 0.6f;

		Trace("Finding the largest possible mesh, this will take some time!");
		Trace("N, Num Triangles, Mesh Size, Size / Triangle, SubShapeID Bits, Time");
		for (int i = 1; ; ++i)
		{
			const int n = 500 * i;
			const float cell_size = 1.0f;
			const float max_height = 50.0f;

			// Create heights
			MeshShapeSettings settings;
			float center = n * cell_size / 2;
			settings.mTriangleVertices.reserve((n + 1)*(n + 1));
			for (int x = 0; x <= n; ++x)
				for (int z = 0; z <= n; ++z)
					settings.mTriangleVertices.push_back(Float3(cell_size * x - center, max_height * Sin(float(x) * 50.0f / n) * Cos(float(z) * 50.0f / n), cell_size * z - center));

			// Create regular grid of triangles
			settings.mIndexedTriangles.reserve(2 * n * n);
			for (int x = 0; x < n; ++x)
				for (int z = 0; z < n; ++z)
				{
					settings.mIndexedTriangles.push_back(IndexedTriangle(x + z * (n + 1), x + 1 + z * (n + 1), x + (z + 1)*(n + 1)));
					settings.mIndexedTriangles.push_back(IndexedTriangle(x + 1 + z * (n + 1), x + 1 + (z + 1)*(n + 1), x + (z + 1)*(n + 1)));
				}

			// Start measuring
			chrono::high_resolution_clock::time_point clock_start = chrono::high_resolution_clock::now();

			// Create the mesh shape
			Shape::ShapeResult result = settings.Create();

			// Stop measuring
			chrono::high_resolution_clock::time_point clock_end = chrono::high_resolution_clock::now();
			chrono::nanoseconds duration = chrono::duration_cast<chrono::nanoseconds>(clock_end - clock_start);

			if (result.HasError())
			{
				// Break when we get an error
				Trace("Mesh creation failed with error: %s", result.GetError().c_str());
				break;
			}
			else
			{
				// Trace stats
				RefConst<Shape> shape = result.Get();
				Shape::Stats stats = shape->GetStats();
				Trace("%u, %u, %llu, %.1f, %d, %.3f", n, stats.mNumTriangles, (uint64)stats.mSizeBytes, double(stats.mSizeBytes) / double(stats.mNumTriangles), shape->GetSubShapeIDBitsRecursive(), 1.0e-9 * double(duration.count()));

				// Set this shape as the best shape so far
				mMeshCreationSettings.SetShape(shape);
			}
		}

		return true;
	}

	virtual void			StartTest(PhysicsSystem &inPhysicsSystem, EMotionQuality inMotionQuality) override
	{
		// Create background
		BodyInterface &bi = inPhysicsSystem.GetBodyInterface();
		bi.CreateAndAddBody(mMeshCreationSettings, EActivation::DontActivate);

		// Construct bodies
		BodyCreationSettings creation_settings;
		creation_settings.mMotionType = EMotionType::Dynamic;
		creation_settings.mMotionQuality = inMotionQuality;
		creation_settings.mObjectLayer = Layers::MOVING;
		creation_settings.mFriction = 0.5f;
		creation_settings.mRestitution = 0.6f;
		creation_settings.SetShape(new BoxShape(Vec3(0.5f, 0.75f, 1.0f)));
		for (int x = -10; x <= 10; ++x)
			for (int y = 0; y < 10; ++y)
				for (int z = -10; z <= 10; ++z)
				{
					creation_settings.mPosition = RVec3(7.5_r * x, 55.0_r + 2.0_r * y, 7.5_r * z);
					bi.CreateAndAddBody(creation_settings, EActivation::Activate);
				}
	}

private:
	BodyCreationSettings	mMeshCreationSettings;
};
