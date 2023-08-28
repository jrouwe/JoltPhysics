// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/SoftBody/SoftBodyUpdatePositionTest.h>
#include <Jolt/Physics/SoftBody/SoftBodyCreationSettings.h>
#include <Utils/SoftBodyCreator.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(SoftBodyUpdatePositionTest)
{
	JPH_ADD_BASE_CLASS(SoftBodyUpdatePositionTest, Test)
}

void SoftBodyUpdatePositionTest::Initialize()
{
	// Floor
	CreateFloor();

	// Bodies with various settings for 'make rotation identity' and 'update position'
	SoftBodyCreationSettings sphere(SoftBodyCreator::CreateCube(), RVec3::sZero(), Quat::sRotation(Vec3::sReplicate(1.0f / sqrt(3.0f)), 0.25f * JPH_PI), Layers::MOVING);

	for (int update_position = 0; update_position < 2; ++update_position)
		for (int make_rotation_identity = 0; make_rotation_identity < 2; ++make_rotation_identity)
		{
			sphere.mPosition = RVec3(update_position * 10.0f, 10.0f, make_rotation_identity * 10.0f);
			sphere.mUpdatePosition = update_position != 0;
			sphere.mMakeRotationIdentity = make_rotation_identity != 0;
			mBodyInterface->CreateAndAddSoftBody(sphere, EActivation::Activate);
		}
}
