// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Test.h>
#include <Math/Perlin.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/HeightFieldShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_ABSTRACT(Test)
{
}

Body &Test::CreateFloor(float inSize)
{
	const float scale = GetWorldScale();

	Body &floor = *mBodyInterface->CreateBody(BodyCreationSettings(new BoxShape(scale * Vec3(0.5f * inSize, 1.0f, 0.5f * inSize), 0.0f), RVec3(scale * Vec3(0.0f, -1.0f, 0.0f)), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING));
	mBodyInterface->AddBody(floor.GetID(), EActivation::DontActivate);
	return floor;
}

Body &Test::CreateLargeTriangleFloor()
{
	TriangleList triangles = {
		Triangle(Float3(427.941376f, 0.000027f, -456.470642f), Float3(427.941376f, 0.000024f, -399.411774f), Float3(512.0f, 0.000031f, -511.999969f)),
		Triangle(Float3(0.0f, 0.000031f, -511.999969f), Float3(28.529310f, 0.000027f, -456.470642f), Float3(427.941376f, 0.000027f, -456.470642f)),
		Triangle(Float3(427.941376f, 0.000027f, -456.470642f), Float3(512.0f, 0.000031f, -511.999969f), Float3(0.0f, 0.000031f, -511.999969f)),
		Triangle(Float3(285.294067f, 0.000027f, -456.470642f), Float3(399.411804f, 0.000024f, -399.411774f), Float3(313.823395f, 0.000027f, -456.470642f)),
		Triangle(Float3(313.823395f, 0.000027f, -456.470642f), Float3(399.411804f, 0.000024f, -399.411774f), Float3(342.352936f, 0.000027f, -456.470642f)),
		Triangle(Float3(342.352936f, 0.000027f, -456.470642f), Float3(399.411804f, 0.000024f, -399.411774f), Float3(370.882507f, 0.000027f, -456.470642f)),
		Triangle(Float3(399.411804f, 0.000024f, -399.411774f), Float3(427.941376f, 0.000024f, -399.411774f), Float3(370.882507f, 0.000027f, -456.470642f)),
		Triangle(Float3(370.882507f, 0.000027f, -456.470642f), Float3(427.941376f, 0.000024f, -399.411774f), Float3(399.411804f, 0.000027f, -456.470642f)),
		Triangle(Float3(399.411804f, 0.000027f, -456.470642f), Float3(427.941376f, 0.000024f, -399.411774f), Float3(427.941376f, 0.000027f, -456.470642f)),
		Triangle(Float3(256.764771f, 0.000027f, -456.470642f), Float3(399.411804f, 0.000024f, -399.411774f), Float3(285.294067f, 0.000027f, -456.470642f)),
		Triangle(Float3(85.588173f, 0.000027f, -456.470642f), Float3(399.411804f, 0.000024f, -399.411774f), Float3(114.117729f, 0.000027f, -456.470642f)),
		Triangle(Float3(114.117729f, 0.000027f, -456.470642f), Float3(399.411804f, 0.000024f, -399.411774f), Float3(142.647034f, 0.000027f, -456.470642f)),
		Triangle(Float3(142.647034f, 0.000027f, -456.470642f), Float3(399.411804f, 0.000024f, -399.411774f), Float3(171.176590f, 0.000027f, -456.470642f)),
		Triangle(Float3(171.176590f, 0.000027f, -456.470642f), Float3(399.411804f, 0.000024f, -399.411774f), Float3(199.705902f, 0.000027f, -456.470642f)),
		Triangle(Float3(199.705902f, 0.000027f, -456.470642f), Float3(399.411804f, 0.000024f, -399.411774f), Float3(228.235214f, 0.000027f, -456.470642f)),
		Triangle(Float3(228.235214f, 0.000027f, -456.470642f), Float3(399.411804f, 0.000024f, -399.411774f), Float3(256.764771f, 0.000027f, -456.470642f)),
		Triangle(Float3(85.588173f, 0.000024f, -399.411774f), Float3(399.411804f, 0.000024f, -399.411774f), Float3(85.588173f, 0.000027f, -456.470642f)),
		Triangle(Float3(427.941376f, 0.000024f, -399.411774f), Float3(512.0f, 0.000019f, -313.823364f), Float3(512.0f, 0.000031f, -511.999969f)),
		Triangle(Float3(399.411804f, 0.000024f, -399.411774f), Float3(512.0f, 0.000019f, -313.823364f), Float3(427.941376f, 0.000024f, -399.411774f)),
		Triangle(Float3(285.294067f, 0.000024f, -399.411774f), Float3(512.0f, 0.000019f, -313.823364f), Float3(313.823395f, 0.000024f, -399.411774f)),
		Triangle(Float3(313.823395f, 0.000024f, -399.411774f), Float3(512.0f, 0.000019f, -313.823364f), Float3(342.352936f, 0.000024f, -399.411774f)),
		Triangle(Float3(342.352936f, 0.000024f, -399.411774f), Float3(512.0f, 0.000019f, -313.823364f), Float3(370.882507f, 0.000024f, -399.411774f)),
		Triangle(Float3(370.882507f, 0.000024f, -399.411774f), Float3(512.0f, 0.000019f, -313.823364f), Float3(399.411804f, 0.000024f, -399.411774f)),
		Triangle(Float3(256.764771f, 0.000024f, -399.411774f), Float3(512.0f, 0.000019f, -313.823364f), Float3(285.294067f, 0.000024f, -399.411774f)),
		Triangle(Float3(228.235214f, 0.000024f, -399.411774f), Float3(512.0f, 0.000019f, -313.823364f), Float3(256.764771f, 0.000024f, -399.411774f)),
		Triangle(Float3(199.705902f, 0.000024f, -399.411774f), Float3(512.0f, 0.000019f, -313.823364f), Float3(228.235214f, 0.000024f, -399.411774f)),
		Triangle(Float3(228.235214f, 0.000019f, -313.823364f), Float3(512.0f, 0.000019f, -313.823364f), Float3(199.705902f, 0.000024f, -399.411774f)),
		Triangle(Float3(142.647034f, 0.000024f, -399.411774f), Float3(228.235214f, 0.000019f, -313.823364f), Float3(171.176590f, 0.000024f, -399.411774f)),
		Triangle(Float3(171.176590f, 0.000024f, -399.411774f), Float3(228.235214f, 0.000019f, -313.823364f), Float3(199.705902f, 0.000024f, -399.411774f)),
		Triangle(Float3(85.588173f, 0.000022f, -370.882477f), Float3(228.235214f, 0.000019f, -313.823364f), Float3(142.647034f, 0.000024f, -399.411774f)),
		Triangle(Float3(85.588173f, 0.000022f, -370.882477f), Float3(199.705902f, 0.000019f, -313.823364f), Float3(228.235214f, 0.000019f, -313.823364f)),
		Triangle(Float3(114.117729f, 0.000024f, -399.411774f), Float3(85.588173f, 0.000022f, -370.882477f), Float3(142.647034f, 0.000024f, -399.411774f)),
		Triangle(Float3(85.588173f, 0.000024f, -399.411774f), Float3(85.588173f, 0.000022f, -370.882477f), Float3(114.117729f, 0.000024f, -399.411774f)),
		Triangle(Float3(28.529310f, 0.000019f, -313.823364f), Float3(199.705902f, 0.000019f, -313.823364f), Float3(85.588173f, 0.000022f, -370.882477f)),
		Triangle(Float3(57.058865f, 0.000019f, -313.823364f), Float3(0.0f, 0.000017f, -285.294037f), Float3(85.588173f, 0.000019f, -313.823364f)),
		Triangle(Float3(28.529310f, 0.000019f, -313.823364f), Float3(0.0f, 0.000017f, -285.294037f), Float3(57.058865f, 0.000019f, -313.823364f)),
		Triangle(Float3(28.529310f, 0.000027f, -456.470642f), Float3(0.0f, 0.000017f, -285.294037f), Float3(57.058865f, 0.000027f, -456.470642f)),
		Triangle(Float3(0.0f, 0.000017f, -285.294037f), Float3(28.529310f, 0.000027f, -456.470642f), Float3(0.0f, 0.000031f, -511.999969f)),
		Triangle(Float3(0.0f, 0.000017f, -285.294037f), Float3(85.588173f, 0.000022f, -370.882477f), Float3(85.588173f, 0.000024f, -399.411774f)),
		Triangle(Float3(0.0f, 0.000017f, -285.294037f), Float3(85.588173f, 0.000024f, -399.411774f), Float3(57.058865f, 0.000027f, -456.470642f)),
		Triangle(Float3(57.058865f, 0.000027f, -456.470642f), Float3(85.588173f, 0.000024f, -399.411774f), Float3(85.588173f, 0.000027f, -456.470642f)),
		Triangle(Float3(399.411804f, 0.000019f, -313.823364f), Float3(512.0f, 0.000003f, -57.058861f), Float3(456.470673f, 0.000019f, -313.823364f)),
		Triangle(Float3(456.470673f, 0.000019f, -313.823364f), Float3(512.0f, 0.000003f, -57.058861f), Float3(512.0f, 0.000019f, -313.823364f)),
		Triangle(Float3(228.235214f, 0.000019f, -313.823364f), Float3(512.0f, 0.000003f, -57.058861f), Float3(256.764771f, 0.000019f, -313.823364f)),
		Triangle(Float3(256.764771f, 0.000019f, -313.823364f), Float3(512.0f, 0.000003f, -57.058861f), Float3(285.294067f, 0.000019f, -313.823364f)),
		Triangle(Float3(285.294067f, 0.000019f, -313.823364f), Float3(512.0f, 0.000003f, -57.058861f), Float3(313.823395f, 0.000019f, -313.823364f)),
		Triangle(Float3(313.823395f, 0.000019f, -313.823364f), Float3(512.0f, 0.000003f, -57.058861f), Float3(342.352936f, 0.000019f, -313.823364f)),
		Triangle(Float3(342.352936f, 0.000019f, -313.823364f), Float3(512.0f, 0.000003f, -57.058861f), Float3(370.882507f, 0.000019f, -313.823364f)),
		Triangle(Float3(370.882507f, 0.000019f, -313.823364f), Float3(512.0f, 0.000003f, -57.058861f), Float3(399.411804f, 0.000019f, -313.823364f)),
		Triangle(Float3(0.0f, 0.000017f, -285.294037f), Float3(0.0f, 0.000009f, -142.647018f), Float3(512.0f, 0.000003f, -57.058861f)),
		Triangle(Float3(199.705902f, 0.000019f, -313.823364f), Float3(512.0f, 0.000003f, -57.058861f), Float3(228.235214f, 0.000019f, -313.823364f)),
		Triangle(Float3(171.176590f, 0.000019f, -313.823364f), Float3(512.0f, 0.000003f, -57.058861f), Float3(199.705902f, 0.000019f, -313.823364f)),
		Triangle(Float3(0.0f, 0.000017f, -285.294037f), Float3(512.0f, 0.000003f, -57.058861f), Float3(85.588173f, 0.000019f, -313.823364f)),
		Triangle(Float3(85.588173f, 0.000019f, -313.823364f), Float3(512.0f, 0.000003f, -57.058861f), Float3(142.647034f, 0.000019f, -313.823364f)),
		Triangle(Float3(142.647034f, 0.000019f, -313.823364f), Float3(512.0f, 0.000003f, -57.058861f), Float3(171.176590f, 0.000019f, -313.823364f)),
		Triangle(Float3(485.0f, 0.000002f, -28.529308f), Float3(512.0f, 0.0f, 0.0f), Float3(512.0f, 0.000002f, -28.529308f)),
		Triangle(Float3(512.0f, 0.0f, 0.0f), Float3(427.941376f, 0.000002f, -28.529308f), Float3(285.294067f, 0.000002f, -28.529308f)),
		Triangle(Float3(456.470673f, 0.000002f, -28.529308f), Float3(512.0f, 0.0f, 0.0f), Float3(485.0f, 0.000002f, -28.529308f)),
		Triangle(Float3(427.941376f, 0.000002f, -28.529308f), Float3(512.0f, 0.0f, 0.0f), Float3(456.470673f, 0.000002f, -28.529308f)),
		Triangle(Float3(171.176590f, 0.0f, 0.0f), Float3(512.0f, 0.0f, 0.0f), Float3(285.294067f, 0.000002f, -28.529308f)),
		Triangle(Float3(285.294067f, 0.000002f, -28.529308f), Float3(512.0f, 0.000002f, -28.529308f), Float3(512.0f, 0.000003f, -57.058861f)),
		Triangle(Float3(0.0f, 0.000009f, -142.647018f), Float3(285.294067f, 0.000002f, -28.529308f), Float3(512.0f, 0.000003f, -57.058861f)),
		Triangle(Float3(0.0f, 0.000007f, -114.117722f), Float3(171.176590f, 0.0f, 0.0f), Float3(0.0f, 0.000009f, -142.647018f)),
		Triangle(Float3(0.0f, 0.0f, 0.0f), Float3(171.176590f, 0.0f, 0.0f), Float3(0.0f, 0.000007f, -114.117722f)),
		Triangle(Float3(0.0f, 0.000009f, -142.647018f), Float3(171.176590f, 0.0f, 0.0f), Float3(285.294067f, 0.000002f, -28.529308f))
	};
	MeshShapeSettings mesh_settings(triangles);
	mesh_settings.SetEmbedded();
	BodyCreationSettings floor_settings(&mesh_settings, RVec3(-256.0f, 0.0f, 256.0f), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);
	Body &floor = *mBodyInterface->CreateBody(floor_settings);
	mBodyInterface->AddBody(floor.GetID(), EActivation::DontActivate);
	return floor;
}

Body &Test::CreateMeshTerrain()
{
	const float scale = GetWorldScale();

#ifdef _DEBUG
	const int n = 50;
	const float cell_size = scale * 2.0f;
#else
	const int n = 100;
	const float cell_size = scale * 1.0f;
#endif
	const float max_height = scale * 3.0f;

	// Create heights
	float heights[n + 1][n + 1];
	for (int x = 0; x <= n; ++x)
		for (int z = 0; z <= n; ++z)
			heights[x][z] = max_height * PerlinNoise3(float(x) * 8.0f / n, 0, float(z) * 8.0f / n, 256, 256, 256);

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

	// Floor
	Body &floor = *mBodyInterface->CreateBody(BodyCreationSettings(new MeshShapeSettings(triangles), RVec3::sZero(), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING));
	mBodyInterface->AddBody(floor.GetID(), EActivation::DontActivate);
	return floor;
}

Body &Test::CreateHeightFieldTerrain()
{
	const float scale = GetWorldScale();

	const int n = 128;
	const float cell_size = scale * 1.0f;
	const float max_height = scale * 5.0f;

	// Create height samples
	float heights[n * n];
	for (int y = 0; y < n; ++y)
		for (int x = 0; x < n; ++x)
			heights[y * n + x] = max_height * PerlinNoise3(float(x) * 8.0f / n, 0, float(y) * 8.0f / n, 256, 256, 256);

	// Create height field
	RefConst<ShapeSettings> height_field = new HeightFieldShapeSettings(heights, Vec3(-0.5f * cell_size * n, 0.0f, -0.5f * cell_size * n), Vec3(cell_size, 1.0f, cell_size), n);

	// Floor
	Body &floor = *mBodyInterface->CreateBody(BodyCreationSettings(height_field, RVec3::sZero(), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING));
	mBodyInterface->AddBody(floor.GetID(), EActivation::DontActivate);
	return floor;
}
