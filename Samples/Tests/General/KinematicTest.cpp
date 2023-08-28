// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/General/KinematicTest.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(KinematicTest)
{
	JPH_ADD_BASE_CLASS(KinematicTest, Test)
}

void KinematicTest::Initialize()
{
	// Floor
	CreateFloor();

	// Wall
	RefConst<Shape> box_shape = new BoxShape(Vec3(1.0f, 1.0f, 1.0f));
	for (int i = 0; i < 3; ++i)
		for (int j = i / 2; j < 10 - (i + 1) / 2; ++j)
		{
			RVec3 position(-10.0f + j * 2.0f + (i & 1? 1.0f : 0.0f), 1.0f + i * 2.0f, 0);
			Body &wall = *mBodyInterface->CreateBody(BodyCreationSettings(box_shape, position, Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
			mBodyInterface->AddBody(wall.GetID(), EActivation::DontActivate);
		}

	// Kinematic object
	for (int i = 0; i < 2; ++i)
	{
		mKinematic[i] = mBodyInterface->CreateBody(BodyCreationSettings(new SphereShape(1.0f), RVec3(-10.0f, 2.0f, i == 0? 5.0f : -5.0f), Quat::sIdentity(), EMotionType::Kinematic, Layers::MOVING));
		mBodyInterface->AddBody(mKinematic[i]->GetID(), EActivation::Activate);
	}
}

void KinematicTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	for (int i = 0; i < 2; ++i)
	{
		RVec3 com = mKinematic[i]->GetCenterOfMassPosition();
		if (com.GetZ() >= 5.0f)
			mKinematic[i]->SetLinearVelocity(Vec3(2.0f, 0, -10.0f));
		else if (com.GetZ() <= -5.0f)
			mKinematic[i]->SetLinearVelocity(Vec3(2.0f, 0, 10.0f));
	}
}
