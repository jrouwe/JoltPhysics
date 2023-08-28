// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Shapes/TaperedCapsuleShapeTest.h>
#include <Jolt/Physics/Collision/Shape/TaperedCapsuleShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(TaperedCapsuleShapeTest)
{
	JPH_ADD_BASE_CLASS(TaperedCapsuleShapeTest, Test)
}

void TaperedCapsuleShapeTest::Initialize()
{
	// Floor
	CreateFloor();

	RefConst<ShapeSettings> big_taperedcapsule = new TaperedCapsuleShapeSettings(2.0f, 1.0f, 3.0f);
	RefConst<ShapeSettings> big_taperedcapsule2 = new TaperedCapsuleShapeSettings(2.0f, 3.0f, 1.0f);

	// Tapered capsule on outer sphere
	Body &body1 = *mBodyInterface->CreateBody(BodyCreationSettings(big_taperedcapsule, RVec3(0, 10, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
	mBodyInterface->AddBody(body1.GetID(), EActivation::Activate);

	// Tapered capsule on other outer sphere
	Body &body2 = *mBodyInterface->CreateBody(BodyCreationSettings(big_taperedcapsule2, RVec3(10, 10, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
	mBodyInterface->AddBody(body2.GetID(), EActivation::Activate);

	// Tapered capsule on side
	Body &body3 = *mBodyInterface->CreateBody(BodyCreationSettings(big_taperedcapsule, RVec3(20, 10, 0), Quat::sRotation(Vec3::sAxisX(), 0.5f * JPH_PI), EMotionType::Dynamic, Layers::MOVING));
	mBodyInterface->AddBody(body3.GetID(), EActivation::Activate);

	RefConst<ShapeSettings> long_taperedcapsule = new TaperedCapsuleShapeSettings(5, 0.5f, 1.0f);

	// Tower of tapered capsules
	for (int i = 0; i < 10; ++i)
	{
		for (int j = 0; j < 2; ++j)
		{
			RVec3 position;
			Quat rotation;
			if (i & 1)
			{
				position = RVec3(-4.0f + 8.0f * j, 2.0f + 3.0f * i, -20.0f);
				rotation = Quat::sRotation(Vec3::sAxisX(), 0.5f * JPH_PI + (j & 1) * JPH_PI);
			}
			else
			{
				position = RVec3(0, 2.0f + 3.0f * i, -20.0f - 4.0f + 8.0f * j);
				rotation = Quat::sRotation(Vec3::sAxisZ(), 0.5f * JPH_PI + (j & 1) * JPH_PI);
			}
			Body &body = *mBodyInterface->CreateBody(BodyCreationSettings(long_taperedcapsule, position, rotation, EMotionType::Dynamic, Layers::MOVING));
			mBodyInterface->AddBody(body.GetID(), EActivation::Activate);
		}
	}
}
