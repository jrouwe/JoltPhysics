// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/General/RestitutionTest.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(RestitutionTest)
{
	JPH_ADD_BASE_CLASS(RestitutionTest, Test)
}

void RestitutionTest::Initialize()
{
	// Floor
	CreateFloor();

	RefConst<Shape> sphere = new SphereShape(2.0f);
	RefConst<Shape> box = new BoxShape(Vec3(2.0f, 2.0f, 2.0f));

	// Bodies with increasing restitution
	for (int i = 0; i <= 10; ++i)
	{
		BodyCreationSettings settings(sphere, RVec3(-50.0f + i * 10.0f, 20.0f, -20.0f), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
		settings.mRestitution = 0.1f * i;
		settings.mLinearDamping = 0.0f;
		Body &body = *mBodyInterface->CreateBody(settings);
		mBodyInterface->AddBody(body.GetID(), EActivation::Activate);
	}

	for (int i = 0; i <= 10; ++i)
	{
		BodyCreationSettings settings(box, RVec3(-50.0f + i * 10.0f, 20.0f, 20.0f), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
		settings.mRestitution = 0.1f * i;
		settings.mLinearDamping = 0.0f;
		Body &body = *mBodyInterface->CreateBody(settings);
		mBodyInterface->AddBody(body.GetID(), EActivation::Activate);
	}
}
