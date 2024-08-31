// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Shapes/CapsuleShapeTest.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(CapsuleShapeTest)
{
	JPH_ADD_BASE_CLASS(CapsuleShapeTest, Test)
}

void CapsuleShapeTest::Initialize()
{
	// Floor
	CreateFloor();

	RefConst<Shape> big_capsule = new CapsuleShape(2.5f, 2);

	// Capsule on outer sphere
	mBodyInterface->CreateAndAddBody(BodyCreationSettings(big_capsule, RVec3(0, 10, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING), EActivation::Activate);

	// Capsule on cylinder
	mBodyInterface->CreateAndAddBody(BodyCreationSettings(big_capsule, RVec3(10, 10, 0), Quat::sRotation(Vec3::sAxisX(), 0.5f * JPH_PI), EMotionType::Dynamic, Layers::MOVING), EActivation::Activate);

	RefConst<Shape> long_capsule = new CapsuleShape(5, 1);

	// Tower of capsules
	for (int i = 0; i < 10; ++i)
	{
		for (int j = 0; j < 2; ++j)
		{
			RVec3 position;
			Quat rotation;
			if (i & 1)
			{
				position = RVec3(-4.0f + 8.0f * j, 2.0f + 3.0f * i, -20.0f);
				rotation = Quat::sRotation(Vec3::sAxisX(), 0.5f * JPH_PI);
			}
			else
			{
				position = RVec3(0, 2.0f + 3.0f * i, -20.0f - 4.0f + 8.0f * j);
				rotation = Quat::sRotation(Vec3::sAxisZ(), 0.5f * JPH_PI);
			}
			mBodyInterface->CreateAndAddBody(BodyCreationSettings(long_capsule, position, rotation, EMotionType::Dynamic, Layers::MOVING), EActivation::Activate);
		}
	}
}
