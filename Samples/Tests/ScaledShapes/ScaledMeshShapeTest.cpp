// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/ScaledShapes/ScaledMeshShapeTest.h>
#include <Math/Perlin.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(ScaledMeshShapeTest) 
{ 
	JPH_ADD_BASE_CLASS(ScaledMeshShapeTest, Test) 
}

void ScaledMeshShapeTest::Initialize() 
{
	// Floor
	CreateFloor();

	const int n = 10;
	const float cell_size = 2.0f;
	const float max_height = 4.0f;

	// Create heights
	float heights[n + 1][n + 1];
	for (int x = 0; x <= n; ++x)
		for (int z = 0; z <= n; ++z)
			heights[x][z] = max_height * PerlinNoise3(float(x) / n, 0, float(z) / n, 256, 256, 256);

	// Create 'wall' around grid
	for (int x = 0; x <= n; ++x)
	{
		heights[x][0] += 2.0f;
		heights[x][n] += 2.0f;
	}

	for (int y = 1; y < n; ++y)
	{
		heights[0][y] += 2.0f;
		heights[n][y] += 2.0f;
	}

	// Create regular grid of triangles
	TriangleList triangles;
	for (int x = 0; x < n; ++x)
		for (int z = 0; z < n; ++z)
		{
			float center = n * cell_size / 2;

			float x1 = cell_size * x - center;
			float z1 = cell_size * z - center;
			float x2 = x1 + cell_size;
			float z2 = z1 + cell_size;

			Float3 v1 = Float3(x1, heights[x][z], z1);
			Float3 v2 = Float3(x2, heights[x + 1][z], z1);
			Float3 v3 = Float3(x1, heights[x][z + 1], z2);
			Float3 v4 = Float3(x2, heights[x + 1][z + 1], z2);

			triangles.push_back(Triangle(v1, v3, v4));
			triangles.push_back(Triangle(v1, v4, v2));
		}

	RefConst<ShapeSettings> mesh_shape = new MeshShapeSettings(triangles);
		
	// Original shape
	Body &body1 = *mBodyInterface->CreateBody(BodyCreationSettings(mesh_shape, Vec3(-60, 10, 0), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING));
	mBodyInterface->AddBody(body1.GetID(), EActivation::DontActivate);

	// Uniformly scaled shape < 1
	Body &body2 = *mBodyInterface->CreateBody(BodyCreationSettings(new ScaledShapeSettings(mesh_shape, Vec3::sReplicate(0.5f)), Vec3(-40, 10, 0), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING));
	mBodyInterface->AddBody(body2.GetID(), EActivation::DontActivate);

	// Uniformly scaled shape > 1
	Body &body3 = *mBodyInterface->CreateBody(BodyCreationSettings(new ScaledShapeSettings(mesh_shape, Vec3::sReplicate(1.5f)), Vec3(-20, 10, 0), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING));
	mBodyInterface->AddBody(body3.GetID(), EActivation::DontActivate);

	// Non-uniform scaled shape
	Body &body4 = *mBodyInterface->CreateBody(BodyCreationSettings(new ScaledShapeSettings(mesh_shape, Vec3(0.5f, 1.0f, 1.5f)), Vec3(0, 10, 0), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING));
	mBodyInterface->AddBody(body4.GetID(), EActivation::DontActivate);

	// Flipped in 2 axis
	Body &body5 = *mBodyInterface->CreateBody(BodyCreationSettings(new ScaledShapeSettings(mesh_shape, Vec3(-0.5f, 1.0f, -1.5f)), Vec3(20, 10, 0), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING));
	mBodyInterface->AddBody(body5.GetID(), EActivation::DontActivate);

	// Inside out
	Body &body6 = *mBodyInterface->CreateBody(BodyCreationSettings(new ScaledShapeSettings(mesh_shape, Vec3(-0.5f, 1.0f, 1.5f)), Vec3(40, 10, 0), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING));
	mBodyInterface->AddBody(body6.GetID(), EActivation::DontActivate);

	// Upside down
	Body &body7 = *mBodyInterface->CreateBody(BodyCreationSettings(new ScaledShapeSettings(mesh_shape, Vec3(0.5f, -1.0f, 1.5f)), Vec3(60, 10, 0), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING));
	mBodyInterface->AddBody(body7.GetID(), EActivation::DontActivate);

	// Create a number of balls above the meshes
	RefConst<Shape> sphere_shape = new SphereShape(0.2f);
	RefConst<Shape> box_shape = new BoxShape(Vec3(0.2f, 0.2f, 0.4f), 0.01f);
	for (int i = 0; i < 7; ++i)
		for (int j = 0; j < 5; ++j)
		{
			Body &dynamic = *mBodyInterface->CreateBody(BodyCreationSettings((j & 1)? box_shape : sphere_shape, Vec3(-60.0f + 20.0f * i, 10.0f + max_height + 0.5f * j, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
			mBodyInterface->AddBody(dynamic.GetID(), EActivation::Activate);
		}
}
