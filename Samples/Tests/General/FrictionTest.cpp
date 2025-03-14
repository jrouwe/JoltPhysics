// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/General/FrictionTest.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(FrictionTest)
{
	JPH_ADD_BASE_CLASS(FrictionTest, Test)
}

void FrictionTest::Initialize()
{
	// Floor
	Body &floor = *mBodyInterface->CreateBody(BodyCreationSettings(new BoxShape(Vec3(100.0f, 1.0f, 100.0f), 0.0f), RVec3::sZero(), Quat::sRotation(Vec3::sAxisX(), 0.25f * JPH_PI), EMotionType::Static, Layers::NON_MOVING));
	floor.SetFriction(1.0f);
	mBodyInterface->AddBody(floor.GetID(), EActivation::DontActivate);

	RefConst<Shape> box = new BoxShape(Vec3(2.0f, 2.0f, 2.0f));
	RefConst<Shape> sphere = new SphereShape(2.0f);

	// Bodies with increasing friction
	for (int i = 0; i <= 10; ++i)
	{
		Body &body = *mBodyInterface->CreateBody(BodyCreationSettings(box, RVec3(-50.0f + i * 10.0f, 55.0f, -50.0f), Quat::sRotation(Vec3::sAxisX(), 0.25f * JPH_PI), EMotionType::Dynamic, Layers::MOVING));
		body.SetFriction(0.1f * i);
		mBodyInterface->AddBody(body.GetID(), EActivation::Activate);
		SetBodyLabel(body.GetID(), StringFormat("Friction: %.1f", double(body.GetFriction())));
	}

	for (int i = 0; i <= 10; ++i)
	{
		Body &body = *mBodyInterface->CreateBody(BodyCreationSettings(sphere, RVec3(-50.0f + i * 10.0f, 47.0f, -40.0f), Quat::sRotation(Vec3::sAxisX(), 0.25f * JPH_PI), EMotionType::Dynamic, Layers::MOVING));
		body.SetFriction(0.1f * i);
		mBodyInterface->AddBody(body.GetID(), EActivation::Activate);
		SetBodyLabel(body.GetID(), StringFormat("Friction: %.1f", double(body.GetFriction())));
	}
}
