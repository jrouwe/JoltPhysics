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
	Body &body1 = *mBodyInterface->CreateBody(BodyCreationSettings(big_capsule, Vec3(0, 10, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
	mBodyInterface->AddBody(body1.GetID(), EActivation::Activate);

	// Capsule on cylinder
	Body &body2 = *mBodyInterface->CreateBody(BodyCreationSettings(big_capsule, Vec3(10, 10, 0), Quat::sRotation(Vec3::sAxisX(), 0.5f * JPH_PI), EMotionType::Dynamic, Layers::MOVING));
	mBodyInterface->AddBody(body2.GetID(), EActivation::Activate);

	RefConst<Shape> long_capsule = new CapsuleShape(5, 1);

	// Tower of capsules
	for (int i = 0; i < 10; ++i)
	{
		for (int j = 0; j < 2; ++j)
		{
			Vec3 position;
			Quat rotation;
			if (i & 1)
			{
				position = Vec3(-4.0f + 8.0f * j, 2.0f + 3.0f * i, -20.0f);
				rotation = Quat::sRotation(Vec3::sAxisX(), 0.5f * JPH_PI);
			}
			else
			{
				position = Vec3(0, 2.0f + 3.0f * i, -20.0f - 4.0f + 8.0f * j);
				rotation = Quat::sRotation(Vec3::sAxisZ(), 0.5f * JPH_PI);
			}
			Body &body = *mBodyInterface->CreateBody(BodyCreationSettings(long_capsule, position, rotation, EMotionType::Dynamic, Layers::MOVING));
			mBodyInterface->AddBody(body.GetID(), EActivation::Activate);
		}
	}
}
