// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/SoftBody/SoftBodyFrictionTest.h>
#include <Jolt/Physics/SoftBody/SoftBodyCreationSettings.h>
#include <Utils/SoftBodyCreator.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(SoftBodyFrictionTest)
{
	JPH_ADD_BASE_CLASS(SoftBodyFrictionTest, Test)
}

void SoftBodyFrictionTest::Initialize()
{
	// Floor
	Body &floor = CreateFloor();
	floor.SetFriction(1.0f);

	// Bodies with increasing friction
	Ref<SoftBodySharedSettings> sphere_settings = SoftBodyCreator::CreateSphere();
	for (SoftBodySharedSettings::Vertex &v : sphere_settings->mVertices)
		v.mVelocity = Float3(0, 0, 10);
	SoftBodyCreationSettings sphere(sphere_settings, RVec3::sZero(), Quat::sIdentity(), Layers::MOVING);
	sphere.mPressure = 2000.0f;

	for (int i = 0; i <= 10; ++i)
	{
		sphere.mPosition = RVec3(-50.0f + i * 10.0f, 1.0f, 0);
		sphere.mFriction = 0.1f * i;
		mBodyInterface->CreateAndAddSoftBody(sphere, EActivation::Activate);
	}

	Ref<SoftBodySharedSettings> cube_settings = SoftBodyCreator::CreateCube();
	for (SoftBodySharedSettings::Vertex &v : cube_settings->mVertices)
		v.mVelocity = Float3(0, 0, 10);
	SoftBodyCreationSettings cube(cube_settings, RVec3::sZero(), Quat::sIdentity(), Layers::MOVING);

	for (int i = 0; i <= 10; ++i)
	{
		cube.mPosition = RVec3(-50.0f + i * 10.0f, 1.0f, -5.0f);
		cube.mFriction = 0.1f * i;
		mBodyInterface->CreateAndAddSoftBody(cube, EActivation::Activate);
	}
}
