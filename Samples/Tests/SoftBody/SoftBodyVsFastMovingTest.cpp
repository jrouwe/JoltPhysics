// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/SoftBody/SoftBodyVsFastMovingTest.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/SoftBody/SoftBodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Utils/SoftBodyCreator.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(SoftBodyVsFastMovingTest)
{
	JPH_ADD_BASE_CLASS(SoftBodyVsFastMovingTest, Test)
}

void SoftBodyVsFastMovingTest::Initialize()
{
	// Floor
	CreateFloor();

	// Create sphere moving towards the cloth
	BodyCreationSettings bcs(new SphereShape(1.0f), RVec3(-2, 20, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
	bcs.mMotionQuality = EMotionQuality::LinearCast;
	bcs.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
	bcs.mMassPropertiesOverride.mMass = 25.0f;
	bcs.mLinearVelocity = Vec3(0, -250, 0);
	mBodyInterface->CreateAndAddBody(bcs, EActivation::Activate);

	// Create cloth that's fixated at the corners
	SoftBodyCreationSettings cloth(SoftBodyCreator::CreateClothWithFixatedCorners(), RVec3(0, 15, 0), Quat::sRotation(Vec3::sAxisX(), 0.1f * JPH_PI) * Quat::sRotation(Vec3::sAxisY(), 0.25f * JPH_PI), Layers::MOVING);
	cloth.mUpdatePosition = false; // Don't update the position of the cloth as it is fixed to the world
	cloth.mMakeRotationIdentity = false; // Test explicitly checks if soft bodies with a rotation collide with shapes properly
	mBodyInterface->CreateAndAddSoftBody(cloth, EActivation::Activate);

	// Create another body with a higher ID than the cloth
	bcs.mPosition = RVec3(2, 20, 0);
	mBodyInterface->CreateAndAddBody(bcs, EActivation::Activate);
}
