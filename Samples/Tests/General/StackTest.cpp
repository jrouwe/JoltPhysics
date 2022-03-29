// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/General/StackTest.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(StackTest) 
{ 
	JPH_ADD_BASE_CLASS(StackTest, Test) 
}

void StackTest::Initialize()
{
	// Floor
	CreateFloor();
		
	RefConst<Shape> box_shape = new BoxShape(Vec3(0.5f, 1.0f, 2.0f));

	// Dynamic body stack
	for (int i = 0; i < 10; ++i)
	{
		Quat rotation;
		if ((i & 1) != 0)
			rotation = Quat::sRotation(Vec3::sAxisY(), 0.5f * JPH_PI);
		else
			rotation = Quat::sIdentity();
		Body &stack = *mBodyInterface->CreateBody(BodyCreationSettings(box_shape, Vec3(10, 1.0f + i * 2.1f, 0), rotation, EMotionType::Dynamic, Layers::MOVING));
		mBodyInterface->AddBody(stack.GetID(), EActivation::Activate);
	}
}
