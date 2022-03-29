// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/General/GravityFactorTest.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(GravityFactorTest) 
{ 
	JPH_ADD_BASE_CLASS(GravityFactorTest, Test) 
}

void GravityFactorTest::Initialize()
{
	// Floor
	CreateFloor();

	RefConst<Shape> box = new BoxShape(Vec3(2.0f, 2.0f, 2.0f));

	// Bodies with increasing gravity fraction
	for (int i = 0; i <= 10; ++i)
	{
		Body &body = *mBodyInterface->CreateBody(BodyCreationSettings(box, Vec3(-50.0f + i * 10.0f, 25.0f, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
		body.GetMotionProperties()->SetGravityFactor(0.1f * i);
		mBodyInterface->AddBody(body.GetID(), EActivation::Activate);
	}
}
