// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
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

// A scene that drops a number of convex shapes on a sloping terrain made out of a mesh shape
class ConvexVsMeshScene : public PerformanceTestScene
{
public:
	virtual const char *	GetName() const override
	{
		return "ConvexVsMesh";
	}

	virtual bool			Load() override
	{
		const int n = 100;
		const float cell_size = 3.0f;
		const float max_height = 5.0f;
		float center = n * cell_size / 2;

		// Create vertices
		const int num_vertices = (n + 1) * (n + 1);
		VertexList vertices;
		vertices.resize(num_vertices);
		for (int x = 0; x <= n; ++x)
			for (int z = 0; z <= n; ++z)
			{
				float height = Sin(float(x) * 50.0f / n) * Cos(float(z) * 50.0f / n);
				vertices[z * (n + 1) + x] = Float3(cell_size * x, max_height * height, cell_size * z);
			}

		// Create regular grid of triangles
		const int num_triangles = n * n * 2;
		IndexedTriangleList indices;
		indices.resize(num_triangles);
		IndexedTriangle *next = indices.data();
		for (int x = 0; x < n; ++x)
			for (int z = 0; z < n; ++z)
			{
				int start = (n + 1) * z + x;

				next->mIdx[0] = start;
				next->mIdx[1] = start + n + 1;
				next->mIdx[2] = start + 1;
				next++;

				next->mIdx[0] = start + 1;
				next->mIdx[1] = start + n + 1;
				next->mIdx[2] = start + n + 2;
				next++;
			}

		// Create mesh shape settings
		Ref<MeshShapeSettings> mesh_shape_settings = new MeshShapeSettings(vertices, indices);
		mesh_shape_settings->mMaxTrianglesPerLeaf = 4;

		// Create mesh shape creation settings
		mMeshSettings.mMotionType = EMotionType::Static;
		mMeshSettings.mObjectLayer = Layers::NON_MOVING;
		mMeshSettings.mPosition = RVec3(Real(-center), Real(max_height), Real(-center));
		mMeshSettings.mFriction = 0.5f;
		mMeshSettings.mRestitution = 0.6f;
		mMeshSettings.SetShapeSettings(mesh_shape_settings);

		// Create other shapes
		mShapes = {
			new BoxShape(Vec3(0.5f, 0.75f, 1.0f)),
			new SphereShape(0.5f),
			new CapsuleShape(0.75f, 0.5f),
			ConvexHullShapeSettings({ Vec3(0, 1, 0), Vec3(1, 0, 0), Vec3(-1, 0, 0), Vec3(0, 0, 1), Vec3(0, 0, -1) }).Create().Get(),
		};

		return true;
	}

	virtual void			StartTest(PhysicsSystem &inPhysicsSystem, EMotionQuality inMotionQuality) override
	{
		// Reduce the solver iteration count, the scene doesn't have any constraints so we don't need the default amount of iterations
		PhysicsSettings settings = inPhysicsSystem.GetPhysicsSettings();
		settings.mNumVelocitySteps = 4;
		settings.mNumPositionSteps = 1;
		inPhysicsSystem.SetPhysicsSettings(settings);

		// Create background
		BodyInterface &bi = inPhysicsSystem.GetBodyInterface();
		bi.CreateAndAddBody(mMeshSettings, EActivation::DontActivate);

		// Construct bodies
		for (int x = -10; x <= 10; ++x)
			for (int y = 0; y < (int)mShapes.size(); ++y)
				for (int z = -10; z <= 10; ++z)
				{
					BodyCreationSettings creation_settings;
					creation_settings.mMotionType = EMotionType::Dynamic;
					creation_settings.mMotionQuality = inMotionQuality;
					creation_settings.mObjectLayer = Layers::MOVING;
					creation_settings.mPosition = RVec3(7.5_r * x, 15.0_r + 2.0_r * y, 7.5_r * z);
					creation_settings.mFriction = 0.5f;
					creation_settings.mRestitution = 0.6f;
					creation_settings.SetShape(mShapes[y]);
					bi.CreateAndAddBody(creation_settings, EActivation::Activate);
				}
	}

private:
	BodyCreationSettings	mMeshSettings;
	Array<Ref<Shape>>		mShapes;
};
