// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Shapes/SphereShapeTest.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(SphereShapeTest) 
{ 
	JPH_ADD_BASE_CLASS(SphereShapeTest, Test) 
}

void SphereShapeTest::Initialize() 
{
	// Floor
	CreateFloor();
	
	// Create different sized spheres
	Body &body1 = *mBodyInterface->CreateBody(BodyCreationSettings(new SphereShape(1.0f), Vec3(0, 10, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
	mBodyInterface->AddBody(body1.GetID(), EActivation::Activate);

	Body &body2 = *mBodyInterface->CreateBody(BodyCreationSettings(new SphereShape(2.0f), Vec3(0, 10, 10), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
	mBodyInterface->AddBody(body2.GetID(), EActivation::Activate);

	Body &body3 = *mBodyInterface->CreateBody(BodyCreationSettings(new SphereShape(0.5f), Vec3(0, 10, 20), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
	mBodyInterface->AddBody(body3.GetID(), EActivation::Activate);

	// Tower of spheres
	for (int i = 0; i < 10; ++i)
	{
		Body &body = *mBodyInterface->CreateBody(BodyCreationSettings(new SphereShape(0.5f), Vec3(10, 10 + 1.5f * i, 0), Quat::sRotation(Vec3::sAxisZ(), 0.25f * JPH_PI), EMotionType::Dynamic, Layers::MOVING));
		mBodyInterface->AddBody(body.GetID(), EActivation::Activate);
	}
}
