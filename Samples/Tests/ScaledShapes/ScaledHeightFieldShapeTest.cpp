// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/ScaledShapes/ScaledHeightFieldShapeTest.h>
#include <Math/Perlin.h>
#include <Jolt/Physics/Collision/Shape/HeightFieldShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(ScaledHeightFieldShapeTest) 
{ 
	JPH_ADD_BASE_CLASS(ScaledHeightFieldShapeTest, Test) 
}

void ScaledHeightFieldShapeTest::Initialize()
{
	// Floor
	CreateFloor();

	const int n = 64;
	const float cell_size = 0.25f;
	const float max_height = 4.0f;

	// Create height samples
	float heights[n * n];
	for (int y = 0; y < n; ++y)
		for (int x = 0; x < n; ++x)
			heights[y * n + x] = max_height * PerlinNoise3(float(x) * 2.0f / n, 0, float(y) * 2.0f / n, 256, 256, 256);

	// Create 'wall' around height field
	for (int x = 0; x < n; ++x)
	{
		heights[x] += 2.0f;
		heights[x + n * (n - 1)] += 2.0f;
	}

	for (int y = 1; y < n - 1; ++y)
	{
		heights[n * y] += 2.0f;
		heights[n - 1 + n * y] += 2.0f;
	}

	// Create height field
	RefConst<ShapeSettings> height_field = new HeightFieldShapeSettings(heights, Vec3(-0.5f * cell_size * n, 0.0f, -0.5f * cell_size * n), Vec3(cell_size, 1.0f, cell_size), n);
		
	// Original shape
	Body &body1 = *mBodyInterface->CreateBody(BodyCreationSettings(height_field, Vec3(-60, 10, 0), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING));
	mBodyInterface->AddBody(body1.GetID(), EActivation::DontActivate);

	// Uniformly scaled shape < 1
	Body &body2 = *mBodyInterface->CreateBody(BodyCreationSettings(new ScaledShapeSettings(height_field, Vec3::sReplicate(0.5f)), Vec3(-40, 10, 0), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING));
	mBodyInterface->AddBody(body2.GetID(), EActivation::DontActivate);

	// Uniformly scaled shape > 1
	Body &body3 = *mBodyInterface->CreateBody(BodyCreationSettings(new ScaledShapeSettings(height_field, Vec3::sReplicate(1.5f)), Vec3(-20, 10, 0), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING));
	mBodyInterface->AddBody(body3.GetID(), EActivation::DontActivate);

	// Non-uniform scaled shape
	Body &body4 = *mBodyInterface->CreateBody(BodyCreationSettings(new ScaledShapeSettings(height_field, Vec3(0.5f, 1.0f, 1.5f)), Vec3(0, 10, 0), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING));
	mBodyInterface->AddBody(body4.GetID(), EActivation::DontActivate);

	// Flipped in 2 axis
	Body &body5 = *mBodyInterface->CreateBody(BodyCreationSettings(new ScaledShapeSettings(height_field, Vec3(-0.5f, 1.0f, -1.5f)), Vec3(20, 10, 0), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING));
	mBodyInterface->AddBody(body5.GetID(), EActivation::DontActivate);

	// Inside out
	Body &body6 = *mBodyInterface->CreateBody(BodyCreationSettings(new ScaledShapeSettings(height_field, Vec3(-0.5f, 1.0f, 1.5f)), Vec3(40, 10, 0), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING));
	mBodyInterface->AddBody(body6.GetID(), EActivation::DontActivate);

	// Upside down
	Body &body7 = *mBodyInterface->CreateBody(BodyCreationSettings(new ScaledShapeSettings(height_field, Vec3(0.5f, -1.0f, 1.5f)), Vec3(60, 10, 0), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING));
	mBodyInterface->AddBody(body7.GetID(), EActivation::DontActivate);

	// Create a number of balls above the height fields
	RefConst<Shape> sphere_shape = new SphereShape(0.2f);
	RefConst<Shape> box_shape = new BoxShape(Vec3(0.2f, 0.2f, 0.4f), 0.01f);
	for (int i = 0; i < 7; ++i)
		for (int j = 0; j < 5; ++j)
		{
			Body &dynamic = *mBodyInterface->CreateBody(BodyCreationSettings((j & 1)? box_shape : sphere_shape, Vec3(-60.0f + 20.0f * i, 10.0f + max_height + 0.5f * j, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
			mBodyInterface->AddBody(dynamic.GetID(), EActivation::Activate);
		}
}
