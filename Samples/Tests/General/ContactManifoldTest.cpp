// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/General/ContactManifoldTest.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(ContactManifoldTest)
{
	JPH_ADD_BASE_CLASS(ContactManifoldTest, Test)
}

void ContactManifoldTest::Initialize()
{
	// Floor
	CreateFloor();

	RefConst<Shape> big_box = new BoxShape(Vec3(4, 4, 4), 0.0f);
	RefConst<Shape> capsule = new CapsuleShape(5, 2);
	RefConst<Shape> long_box = new BoxShape(Vec3(2, 7, 2));

	for (int i = 0; i < 3; ++i)
		for (int j = 0; j < 2; ++j)
		{
			// Create a box
			Body &box = *mBodyInterface->CreateBody(BodyCreationSettings(big_box, RVec3(-20.0f + i * 10.0f, 4, -20.0f + j * 40.0f), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING));
			mBodyInterface->AddBody(box.GetID(), EActivation::DontActivate);

			// Place a dynamic body on it
			Body &body = *mBodyInterface->CreateBody(BodyCreationSettings(j == 0? capsule : long_box, RVec3(-20.0f + i * 10.0f, 12, -5.0f + i * 5.0f - 20.0f + j * 40.0f), Quat::sRotation(Vec3::sAxisY(), 0.1f * JPH_PI) * Quat::sRotation(Vec3::sAxisX(), 0.5f * JPH_PI), EMotionType::Dynamic, Layers::MOVING));
			mBodyInterface->AddBody(body.GetID(), EActivation::Activate);
		}
}
