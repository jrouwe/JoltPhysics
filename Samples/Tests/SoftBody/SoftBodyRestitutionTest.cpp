// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/SoftBody/SoftBodyRestitutionTest.h>
#include <Jolt/Physics/SoftBody/SoftBodyCreationSettings.h>
#include <Utils/SoftBodyCreator.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(SoftBodyRestitutionTest)
{
	JPH_ADD_BASE_CLASS(SoftBodyRestitutionTest, Test)
}

void SoftBodyRestitutionTest::Initialize()
{
	// Floor
	Body &floor = CreateFloor();
	floor.SetRestitution(0.0f);

	// Bodies with increasing restitution
	SoftBodyCreationSettings sphere(SoftBodyCreator::CreateSphere(), RVec3::sZero(), Quat::sIdentity(), Layers::MOVING);
	sphere.mPressure = 2000.0f;

	for (int i = 0; i <= 10; ++i)
	{
		sphere.mPosition = RVec3(-50.0f + i * 10.0f, 10.0f, 0);
		sphere.mRestitution = 0.1f * i;
		mBodyInterface->CreateAndAddSoftBody(sphere, EActivation::Activate);
	}

	SoftBodyCreationSettings cube(SoftBodyCreator::CreateCube(), RVec3::sZero(), Quat::sIdentity(), Layers::MOVING);

	for (int i = 0; i <= 10; ++i)
	{
		cube.mPosition = RVec3(-50.0f + i * 10.0f, 10.0f, -5.0f);
		cube.mRestitution = 0.1f * i;
		mBodyInterface->CreateAndAddSoftBody(cube, EActivation::Activate);
	}
}
