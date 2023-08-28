// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Shapes/CylinderShapeTest.h>
#include <Jolt/Physics/Collision/Shape/CylinderShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(CylinderShapeTest)
{
	JPH_ADD_BASE_CLASS(CylinderShapeTest, Test)
}

void CylinderShapeTest::Initialize()
{
	// Floor
	CreateFloor();

	// Cylinder on flat part
	RefConst<Shape> big_cylinder = new CylinderShape(2.5f, 2);
	mBodyInterface->CreateAndAddBody(BodyCreationSettings(big_cylinder, RVec3(0, 10, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING), EActivation::Activate);

	// Cylinder on round part
	mBodyInterface->CreateAndAddBody(BodyCreationSettings(big_cylinder, RVec3(10, 10, 0), Quat::sRotation(Vec3::sAxisX(), 0.5f * JPH_PI), EMotionType::Dynamic, Layers::MOVING), EActivation::Activate);

	// Tower of cylinders
	RefConst<Shape> long_cylinder = new CylinderShape(5, 1);
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
			mBodyInterface->CreateAndAddBody(BodyCreationSettings(long_cylinder, position, rotation, EMotionType::Dynamic, Layers::MOVING), EActivation::Activate);
		}
	}

	// Tower of thin cylinders
	RefConst<Shape> thin_cylinder = new CylinderShape(0.1f, 5.0f);
	for (int i = 0; i < 10; ++i)
		mBodyInterface->CreateAndAddBody(BodyCreationSettings(thin_cylinder, RVec3(20.0f, 10.0f - 1.0f * i, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING), EActivation::Activate);
}
