// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/SoftBody/SoftBodyShapesTest.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/SoftBody/SoftBodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/TaperedCapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/CylinderShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Utils/SoftBodyCreator.h>
#include <Renderer/DebugRendererImp.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(SoftBodyShapesTest)
{
	JPH_ADD_BASE_CLASS(SoftBodyShapesTest, Test)
}

void SoftBodyShapesTest::Initialize()
{
	const Quat cCubeOrientation = Quat::sRotation(Vec3::sReplicate(sqrt(1.0f / 3.0f)), DegreesToRadians(45.0f));

	// Floor
	CreateMeshTerrain();

	// Create cloth that's fixated at the corners
	SoftBodyCreationSettings cloth(SoftBodyCreator::CreateClothWithFixatedCorners(), RVec3(0, 10.0f, 0), Quat::sRotation(Vec3::sAxisY(), 0.25f * JPH_PI), Layers::MOVING);
	cloth.mUpdatePosition = false; // Don't update the position of the cloth as it is fixed to the world
	cloth.mMakeRotationIdentity = false; // Test explicitly checks if soft bodies with a rotation collide with shapes properly
	mBodyInterface->CreateAndAddSoftBody(cloth, EActivation::Activate);

	// Create cube
	SoftBodyCreationSettings cube(SoftBodyCreator::CreateCube(), RVec3(20.0f, 10.0f, 0.0f), cCubeOrientation, Layers::MOVING);
	cube.mRestitution = 0.0f;
	mBodyInterface->CreateAndAddSoftBody(cube, EActivation::Activate);

	// Create pressurized sphere
	SoftBodyCreationSettings sphere(SoftBodyCreator::CreateSphere(), RVec3(15.0f, 10.0f, 15.0f), Quat::sIdentity(), Layers::MOVING);
	sphere.mPressure = 2000.0f;
	mBodyInterface->CreateAndAddSoftBody(sphere, EActivation::Activate);

	// Sphere below pressurized sphere
	RefConst<Shape> sphere_shape = new SphereShape(1.0f);
	BodyCreationSettings bcs(sphere_shape, RVec3(15.5f, 7.0f, 15.0f), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
	bcs.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
	bcs.mMassPropertiesOverride.mMass = 100.0f;
	mBodyInterface->CreateAndAddBody(bcs, EActivation::Activate);

	// Various shapes above cloth
	ConvexHullShapeSettings tetrahedron({ Vec3(-2, -2, -2), Vec3(0, -2, 2), Vec3(2, -2, -2), Vec3(0, 2, 0) });
	tetrahedron.SetEmbedded();

	StaticCompoundShapeSettings compound_shape;
	compound_shape.SetEmbedded();
	Quat rotate_x = Quat::sRotation(Vec3::sAxisX(), 0.5f * JPH_PI);
	compound_shape.AddShape(Vec3::sZero(), rotate_x, new CapsuleShape(2, 0.5f));
	compound_shape.AddShape(Vec3(0, 0, -2), Quat::sIdentity(), new SphereShape(1));
	compound_shape.AddShape(Vec3(0, 0, 2), Quat::sIdentity(), new SphereShape(1));

	RefConst<Shape> shapes[] = {
		sphere_shape,
		new BoxShape(Vec3(0.75f, 1.0f, 1.25f)),
		new RotatedTranslatedShape(Vec3::sZero(), rotate_x, new CapsuleShape(1, 0.5f)),
		new RotatedTranslatedShape(Vec3::sZero(), rotate_x, TaperedCapsuleShapeSettings(1.0f, 1.0f, 0.5f).Create().Get()),
		new RotatedTranslatedShape(Vec3::sZero(), rotate_x, new CylinderShape(1, 0.5f)),
		tetrahedron.Create().Get(),
		compound_shape.Create().Get(),
	};
	int num_shapes = (int)std::size(shapes);

	for (int i = 0; i < num_shapes; ++i)
	{
		bcs.SetShape(shapes[i % num_shapes]);
		bcs.mPosition = RVec3(-float(num_shapes) + 2.0f * i, 15.0f, 0);
		mBodyInterface->CreateAndAddBody(bcs, EActivation::Activate);
	}
}
