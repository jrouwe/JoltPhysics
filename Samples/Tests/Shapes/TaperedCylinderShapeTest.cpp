// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Shapes/TaperedCylinderShapeTest.h>
#include <Jolt/Physics/Collision/Shape/TaperedCylinderShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(TaperedCylinderShapeTest)
{
	JPH_ADD_BASE_CLASS(TaperedCylinderShapeTest, Test)
}

void TaperedCylinderShapeTest::Initialize()
{
	// Floor
	CreateFloor();

	RefConst<ShapeSettings> big_taperedcylinder = new TaperedCylinderShapeSettings(2.0f, 1.0f, 3.0f);
	RefConst<ShapeSettings> big_taperedcylinder2 = new TaperedCylinderShapeSettings(2.0f, 3.0f, 1.0f);

	// Tapered cylinder on large radius
	mBodyInterface->CreateAndAddBody(BodyCreationSettings(big_taperedcylinder, RVec3(0, 10, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING), EActivation::Activate);

	// Tapered cylinder on small radius
	mBodyInterface->CreateAndAddBody(BodyCreationSettings(big_taperedcylinder2, RVec3(10, 10, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING), EActivation::Activate);

	// Tapered cylinder on side
	mBodyInterface->CreateAndAddBody(BodyCreationSettings(big_taperedcylinder, RVec3(20, 10, 0), Quat::sRotation(Vec3::sAxisX(), 0.5f * JPH_PI), EMotionType::Dynamic, Layers::MOVING), EActivation::Activate);

	RefConst<ShapeSettings> big_cone = new TaperedCylinderShapeSettings(2.0f, 0.0f, 3.0f, 0.0f);
	RefConst<ShapeSettings> big_cone2 = new TaperedCylinderShapeSettings(2.0f, 3.0f, 0.0f, 0.0f);

	// Cone on large radius
	mBodyInterface->CreateAndAddBody(BodyCreationSettings(big_cone, RVec3(0, 10, 10), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING), EActivation::Activate);

	// Cone on small radius
	mBodyInterface->CreateAndAddBody(BodyCreationSettings(big_cone2, RVec3(10, 10, 10), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING), EActivation::Activate);

	// Cone on side
	mBodyInterface->CreateAndAddBody(BodyCreationSettings(big_cone, RVec3(20, 10, 10), Quat::sRotation(Vec3::sAxisX(), 0.5f * JPH_PI), EMotionType::Dynamic, Layers::MOVING), EActivation::Activate);

	RefConst<ShapeSettings> long_taperedcylinder = new TaperedCylinderShapeSettings(5, 0.5f, 1.0f);

	// Tower of tapered cylinders
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
			mBodyInterface->CreateAndAddBody(BodyCreationSettings(long_taperedcylinder, position, rotation, EMotionType::Dynamic, Layers::MOVING), EActivation::Activate);
		}
	}
}
