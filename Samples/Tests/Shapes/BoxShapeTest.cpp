// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Shapes/BoxShapeTest.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(BoxShapeTest)
{
	JPH_ADD_BASE_CLASS(BoxShapeTest, Test)
}

void BoxShapeTest::Initialize()
{
	// Floor
	CreateFloor();

	// Different sized boxes
	mBodyInterface->CreateAndAddBody(BodyCreationSettings(new BoxShape(Vec3(20, 1, 1)), RVec3(0, 10, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING), EActivation::Activate);

	mBodyInterface->CreateAndAddBody(BodyCreationSettings(new BoxShape(Vec3(2, 3, 4)), RVec3(0, 10, 10), Quat::sRotation(Vec3::sAxisZ(), 0.25f * JPH_PI), EMotionType::Dynamic, Layers::MOVING), EActivation::Activate);

	mBodyInterface->CreateAndAddBody(BodyCreationSettings(new BoxShape(Vec3(0.5f, 0.75f, 1.0f)), RVec3(0, 10, 20), Quat::sRotation(Vec3::sAxisX(), 0.25f * JPH_PI) * Quat::sRotation(Vec3::sAxisZ(), 0.25f * JPH_PI), EMotionType::Dynamic, Layers::MOVING), EActivation::Activate);
}
