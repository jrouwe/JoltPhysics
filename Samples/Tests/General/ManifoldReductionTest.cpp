// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/General/ManifoldReductionTest.h>
#include <Math/Perlin.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/Collision/PhysicsMaterialSimple.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(ManifoldReductionTest) 
{ 
	JPH_ADD_BASE_CLASS(ManifoldReductionTest, Test) 
}

void ManifoldReductionTest::Initialize() 
{
	constexpr float cPerturbance = 0.02f;

	// Create mesh of regular grid of triangles
	TriangleList triangles;
	for (int x = -10; x < 10; ++x)
		for (int z = -10; z < 10; ++z)
		{
			float x1 = 0.1f * x;
			float z1 = 0.1f * z;
			float x2 = x1 + 0.1f;
			float z2 = z1 + 0.1f;

			Float3 v1 = Float3(x1, cPerturbance * PerlinNoise3(x1, 0, z1, 256, 256, 256), z1);
			Float3 v2 = Float3(x2, cPerturbance * PerlinNoise3(x2, 0, z1, 256, 256, 256), z1);
			Float3 v3 = Float3(x1, cPerturbance * PerlinNoise3(x1, 0, z2, 256, 256, 256), z2);
			Float3 v4 = Float3(x2, cPerturbance * PerlinNoise3(x2, 0, z2, 256, 256, 256), z2);

			triangles.push_back(Triangle(v1, v3, v4, 0));
			triangles.push_back(Triangle(v1, v4, v2, 0));
		}
	PhysicsMaterialList materials;
	materials.push_back(new PhysicsMaterialSimple());
	Ref<ShapeSettings> mesh_shape = new MeshShapeSettings(triangles, materials);

	// Floor
	Body &floor = *mBodyInterface->CreateBody(BodyCreationSettings(new ScaledShapeSettings(mesh_shape, Vec3::sReplicate(20)), Vec3::sZero(), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING));
	mBodyInterface->AddBody(floor.GetID(), EActivation::DontActivate);

	// Create a box made of meshes
	Ref<StaticCompoundShapeSettings> mesh_box_shape = new StaticCompoundShapeSettings;
	mesh_box_shape->AddShape(Vec3(0, -1, 0), Quat::sRotation(Vec3::sAxisX(), JPH_PI), mesh_shape);
	mesh_box_shape->AddShape(Vec3(0, 1, 0), Quat::sIdentity(), mesh_shape);
	mesh_box_shape->AddShape(Vec3(-1, 0, 0), Quat::sRotation(Vec3::sAxisZ(), 0.5f * JPH_PI), mesh_shape);
	mesh_box_shape->AddShape(Vec3(1, 0, 0), Quat::sRotation(Vec3::sAxisZ(), -0.5f * JPH_PI), mesh_shape);
	mesh_box_shape->AddShape(Vec3(0, 0, -1), Quat::sRotation(Vec3::sAxisX(), -0.5f * JPH_PI), mesh_shape);
	mesh_box_shape->AddShape(Vec3(0, 0, 1), Quat::sRotation(Vec3::sAxisX(), 0.5f * JPH_PI), mesh_shape);

	// A convex box
	RefConst<ShapeSettings> box_shape = new BoxShapeSettings(Vec3(1, 1, 1), 0.0f);

	{
		// Create a compound of 3 mesh boxes
		Ref<StaticCompoundShapeSettings> three_mesh_box_shape = new StaticCompoundShapeSettings;
		three_mesh_box_shape->AddShape(Vec3(-2.1f, 0, 0), Quat::sIdentity(), mesh_box_shape);
		three_mesh_box_shape->AddShape(Vec3(0, -1, 0), Quat::sIdentity(), mesh_box_shape);
		three_mesh_box_shape->AddShape(Vec3(2.1f, 0, 0), Quat::sIdentity(), mesh_box_shape);


		// Create a compound of 3 convex boxes
		Ref<StaticCompoundShapeSettings> three_box_shape = new StaticCompoundShapeSettings;
		three_box_shape->AddShape(Vec3(-2.1f, 0, 0), Quat::sIdentity(), box_shape);
		three_box_shape->AddShape(Vec3(0, -1.1f, 0), Quat::sIdentity(), box_shape);
		three_box_shape->AddShape(Vec3(2.1f, 0, 0), Quat::sIdentity(), box_shape);


		// A set of 3 mesh boxes to rest on
		Body &three_mesh_box = *mBodyInterface->CreateBody(BodyCreationSettings(three_mesh_box_shape, Vec3(0, 1, 0), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING));
		mBodyInterface->AddBody(three_mesh_box.GetID(), EActivation::DontActivate);

		// A set of 3 boxes that are dynamic where the middle one penetrates more than the other two
		BodyCreationSettings box_settings(three_box_shape, Vec3(0, 2.95f, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
		box_settings.mAllowSleeping = false;
		Body &box = *mBodyInterface->CreateBody(box_settings);
		mBodyInterface->AddBody(box.GetID(), EActivation::Activate);
	}

	{
		// Create a compound of 2 mesh boxes
		Ref<StaticCompoundShapeSettings> two_mesh_box_shape = new StaticCompoundShapeSettings;
		two_mesh_box_shape->AddShape(Vec3(-2.1f, 0, 0), Quat::sIdentity(), mesh_box_shape);
		two_mesh_box_shape->AddShape(Vec3(0, -1, 0), Quat::sIdentity(), mesh_box_shape);

		// Create a compound of 2 convex boxes
		Ref<StaticCompoundShapeSettings> two_box_shape = new StaticCompoundShapeSettings;
		two_box_shape->AddShape(Vec3(-2.1f, 0, 0), Quat::sIdentity(), box_shape);
		two_box_shape->AddShape(Vec3(0, -1, 0), Quat::sIdentity(), box_shape);


		// A set of 2 mesh boxes to rest on
		Body &two_mesh_box = *mBodyInterface->CreateBody(BodyCreationSettings(two_mesh_box_shape, Vec3(0, 1, 4), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING));
		mBodyInterface->AddBody(two_mesh_box.GetID(), EActivation::DontActivate);

		// A set of 2 boxes that are dynamic, one is lower than the other
		BodyCreationSettings box_settings(two_box_shape, Vec3(0, 4, 4), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
		box_settings.mAllowSleeping = false;
		Body &box = *mBodyInterface->CreateBody(box_settings);
		mBodyInterface->AddBody(box.GetID(), EActivation::Activate);
	}

	{
		// Create a compound of 2 meshes under small angle, small enough to combine the manifolds.
		Ref<StaticCompoundShapeSettings> two_mesh_shape = new StaticCompoundShapeSettings;
		two_mesh_shape->AddShape(Vec3(1, 0, 0), Quat::sRotation(Vec3::sAxisZ(), DegreesToRadians(2)), mesh_shape);
		two_mesh_shape->AddShape(Vec3(-1, 0, 0), Quat::sRotation(Vec3::sAxisZ(), DegreesToRadians(-2)), mesh_shape);

		// A set of 2 meshes to rest on
		Body &two_mesh_box = *mBodyInterface->CreateBody(BodyCreationSettings(two_mesh_shape, Vec3(0, 1, -4), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING));
		mBodyInterface->AddBody(two_mesh_box.GetID(), EActivation::DontActivate);

		// A box that is dynamic, resting on the slightly sloped surface. The surface normals are close enough so that the manifold should be merged.
		BodyCreationSettings box_settings(box_shape, Vec3(0, 4, -4), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
		box_settings.mAllowSleeping = false;
		Body &box = *mBodyInterface->CreateBody(box_settings);
		mBodyInterface->AddBody(box.GetID(), EActivation::Activate);
	}

	{
		// Create a compound of 2 meshes under small angle, but bigger than the limit to combine the manifolds.
		Ref<StaticCompoundShapeSettings> two_mesh_shape = new StaticCompoundShapeSettings();
		two_mesh_shape->AddShape(Vec3(1, 0, 0), Quat::sRotation(Vec3::sAxisZ(), DegreesToRadians(3)), mesh_shape);
		two_mesh_shape->AddShape(Vec3(-1, 0, 0), Quat::sRotation(Vec3::sAxisZ(), DegreesToRadians(-3)), mesh_shape);

		// A set of 2 meshes to rest on
		Body &two_mesh_box = *mBodyInterface->CreateBody(BodyCreationSettings(two_mesh_shape, Vec3(0, 1, -8), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING));
		mBodyInterface->AddBody(two_mesh_box.GetID(), EActivation::DontActivate);

		// A box that is dynamic, resting on the slightly sloped surface. The surface normals are not close enough so that the manifold should be merged.
		BodyCreationSettings box_settings(box_shape, Vec3(0, 4, -8), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
		box_settings.mAllowSleeping = false;
		Body &box = *mBodyInterface->CreateBody(box_settings);
		mBodyInterface->AddBody(box.GetID(), EActivation::Activate);
	}
}
