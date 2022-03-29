// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Shapes/MeshShapeTest.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/PhysicsMaterialSimple.h>
#include <Jolt/Geometry/Triangle.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(MeshShapeTest) 
{ 
	JPH_ADD_BASE_CLASS(MeshShapeTest, Test) 
}

void MeshShapeTest::Initialize() 
{
	// Create regular grid of triangles
	uint32 max_material_index = 0;
	TriangleList triangles;
	for (int x = -10; x < 10; ++x)
		for (int z = -10; z < 10; ++z)
		{
			float x1 = 10.0f * x;
			float z1 = 10.0f * z;
			float x2 = x1 + 10.0f;
			float z2 = z1 + 10.0f;

			Float3 v1 = Float3(x1, 0, z1);
			Float3 v2 = Float3(x2, 0, z1);
			Float3 v3 = Float3(x1, 0, z2);
			Float3 v4 = Float3(x2, 0, z2);

			uint32 material_index = uint32((Vec3(v1) + Vec3(v2) + Vec3(v3) + Vec3(v4)).Length() / 40.0f);
			max_material_index = max(max_material_index, material_index);
			triangles.push_back(Triangle(v1, v3, v4, material_index));
			triangles.push_back(Triangle(v1, v4, v2, material_index));
		}

	// Create materials
	PhysicsMaterialList materials;
	for (uint i = 0; i <= max_material_index; ++i)
		materials.push_back(new PhysicsMaterialSimple("Material " + ConvertToString(i), Color::sGetDistinctColor(i)));

	// Floor
	Body &floor = *mBodyInterface->CreateBody(BodyCreationSettings(new MeshShapeSettings(triangles, materials), Vec3::sZero(), Quat::sRotation(Vec3::sAxisX(), 0.25f * JPH_PI), EMotionType::Static, Layers::NON_MOVING));
	mBodyInterface->AddBody(floor.GetID(), EActivation::DontActivate);

	// 1 body with zero friction to test active edge detection
	Ref<BoxShape> box_shape = new BoxShape(Vec3(2.0f, 2.0f, 2.0f), cDefaultConvexRadius, new PhysicsMaterialSimple("Box Material", Color::sYellow));
	Body &body = *mBodyInterface->CreateBody(BodyCreationSettings(box_shape, Vec3(0, 55.0f, -50.0f), Quat::sRotation(Vec3::sAxisX(), 0.25f * JPH_PI), EMotionType::Dynamic, Layers::MOVING));
	body.SetFriction(0.0f);
	mBodyInterface->AddBody(body.GetID(), EActivation::Activate);
}
