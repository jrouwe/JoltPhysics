// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/SoftBody/SoftBodyTest.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/SoftBody/SoftBodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>
#include <Utils/SoftBodyCreator.h>
#include <Renderer/DebugRendererImp.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(SoftBodyTest)
{
	JPH_ADD_BASE_CLASS(SoftBodyTest, Test)
}

void SoftBodyTest::Initialize()
{
	const Quat cCubeOrientation = Quat::sRotation(Vec3::sReplicate(sqrt(1.0f / 3.0f)), DegreesToRadians(45.0f));

	// Floor
	CreateMeshTerrain();

	// Create cloth that's fixated at the corners
	SoftBodyCreationSettings cloth(SoftBodyCreator::CreateCloth(), RVec3(0, 10.0f, 0), Quat::sRotation(Vec3::sAxisY(), 0.25f * JPH_PI));
	cloth.mObjectLayer = Layers::MOVING;
	cloth.mUpdatePosition = false; // Don't update the position of the cloth as it is fixed to the world
	mBodyInterface->CreateAndAddSoftBody(cloth, EActivation::Activate);

	// Create cube
	SoftBodyCreationSettings cube(SoftBodyCreator::CreateCube(), RVec3(15.0f, 10.0f, 0.0f), cCubeOrientation);
	cube.mObjectLayer = Layers::MOVING;
	cube.mRestitution = 0.0f;
	mBodyInterface->CreateAndAddSoftBody(cube, EActivation::Activate);

	// Create another cube that shares information with the first cube
	cube.mPosition = RVec3(25.0f, 10.0f, 0.0f);
	cube.mRestitution = 1.0f;
	cube.mGravityFactor = 0.5f;
	mBodyInterface->CreateAndAddSoftBody(cube, EActivation::Activate);

	// Create pressurized sphere
	SoftBodyCreationSettings sphere(SoftBodyCreator::CreateSphere(), RVec3(15.0f, 10.0f, 15.0f));
	sphere.mObjectLayer = Layers::MOVING;
	sphere.mPressure = 2000.0f;
	mBodyInterface->CreateAndAddSoftBody(sphere, EActivation::Activate);

	// Sphere below pressurized sphere
	RefConst<Shape> sphere_shape = new SphereShape(1.0f);
	BodyCreationSettings bcs(sphere_shape, RVec3(15.5f, 7.0f, 15.0f), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
	bcs.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
	bcs.mMassPropertiesOverride.mMass = 100.0f;
	mBodyInterface->CreateAndAddBody(bcs, EActivation::Activate);

	// Various shapes above cloth
	StaticCompoundShapeSettings compound_shape;
	compound_shape.SetEmbedded();
	compound_shape.AddShape(Vec3::sZero(), Quat::sRotation(Vec3::sAxisX(), 0.5f * JPH_PI), new CapsuleShape(2, 0.5f));
	compound_shape.AddShape(Vec3(0, 0, -2), Quat::sIdentity(), new SphereShape(1));
	compound_shape.AddShape(Vec3(0, 0, 2), Quat::sIdentity(), new SphereShape(1));

	RefConst<Shape> shapes[] = {
		sphere_shape,
		new BoxShape(Vec3(0.75f, 1.0f, 1.25f)),
		compound_shape.Create().Get(),
	};

	for (int i = 0; i < 6; ++i)
	{
		bcs.SetShape(shapes[i % std::size(shapes)]);
		bcs.mPosition = RVec3(0, 15.0f + 3.0f * i, 0);
		mBodyInterface->CreateAndAddBody(bcs, EActivation::Activate);
	}
}
