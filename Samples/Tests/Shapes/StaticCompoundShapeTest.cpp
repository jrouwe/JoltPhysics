// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Shapes/StaticCompoundShapeTest.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/TaperedCapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/CylinderShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(StaticCompoundShapeTest)
{
	JPH_ADD_BASE_CLASS(StaticCompoundShapeTest, Test)
}

void StaticCompoundShapeTest::Initialize()
{
	// Floor
	CreateFloor();

	// Simple compound
	Ref<StaticCompoundShapeSettings> compound_shape1 = new StaticCompoundShapeSettings;
	compound_shape1->AddShape(Vec3::sZero(), Quat::sIdentity(), new CapsuleShape(5, 1));
	compound_shape1->AddShape(Vec3(0, -5, 0), Quat::sIdentity(), new SphereShape(2));
	compound_shape1->AddShape(Vec3(0, 5, 0), Quat::sIdentity(), new SphereShape(2));

	// Compound with sub compound and rotation
	Ref<StaticCompoundShapeSettings> sub_compound = new StaticCompoundShapeSettings;
	sub_compound->AddShape(Vec3(0, 1.5f, 0), Quat::sRotation(Vec3::sAxisZ(), 0.5f * JPH_PI), new BoxShape(Vec3(1.5f, 0.25f, 0.2f)));
	sub_compound->AddShape(Vec3(1.5f, 0, 0), Quat::sRotation(Vec3::sAxisZ(), 0.5f * JPH_PI), new CylinderShape(1.5f, 0.2f));
	sub_compound->AddShape(Vec3(0, 0, 1.5f), Quat::sRotation(Vec3::sAxisX(), 0.5f * JPH_PI), new TaperedCapsuleShapeSettings(1.5f, 0.25f, 0.2f));

	Ref<StaticCompoundShapeSettings> compound_shape2 = new StaticCompoundShapeSettings;
	compound_shape2->AddShape(Vec3(0, 0, 0), Quat::sRotation(Vec3::sAxisX(), -0.25f * JPH_PI) * Quat::sRotation(Vec3::sAxisZ(), 0.25f * JPH_PI), sub_compound);
	compound_shape2->AddShape(Vec3(0, -0.1f, 0), Quat::sRotation(Vec3::sAxisX(), 0.25f * JPH_PI) * Quat::sRotation(Vec3::sAxisZ(), -0.75f * JPH_PI), sub_compound);

	// Compound with large amount of sub shapes
	Ref<StaticCompoundShapeSettings> compound_shape3 = new StaticCompoundShapeSettings;
	for (int y = -2; y <= 2; ++y)
		for (int x = -2; x <= 2; ++x)
			for (int z = -2; z <= 2; ++z)
				compound_shape3->AddShape(Vec3(0.5f * x, 0.5f * y, 0.5f * z), Quat::sRotation(Vec3::sAxisX(), -0.25f * JPH_PI) * Quat::sRotation(Vec3::sAxisZ(), 0.25f * JPH_PI), new BoxShape(Vec3::sReplicate(0.5f)));

	Ref<StaticCompoundShapeSettings> shapes[] = { compound_shape1, compound_shape2, compound_shape3 };

	for (int i = 0; i < 10; ++i)
		for (int j = 0; j < 3; ++j)
		{
			Quat rotation;
			if ((i & 1) == 0)
				rotation = Quat::sRotation(Vec3::sAxisX(), 0.5f * JPH_PI);
			else
				rotation = Quat::sRotation(Vec3::sAxisZ(), 0.5f * JPH_PI);
			Body &body = *mBodyInterface->CreateBody(BodyCreationSettings(shapes[j], RVec3(0, 10.0f + 4.0f * i, j * 20.0f), rotation, EMotionType::Dynamic, Layers::MOVING));
			mBodyInterface->AddBody(body.GetID(), EActivation::Activate);
		}
}
