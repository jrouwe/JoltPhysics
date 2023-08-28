// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/General/WallTest.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(WallTest)
{
	JPH_ADD_BASE_CLASS(WallTest, Test)
}

void WallTest::Initialize()
{
	// Floor
	CreateFloor();

	RefConst<Shape> box_shape = new BoxShape(Vec3(1.0f, 1.0f, 1.0f));

	// Wall
	for (int i = 0; i < 10; ++i)
		for (int j = i / 2; j < 50 - (i + 1) / 2; ++j)
		{
			RVec3 position(-50 + j * 2.0f + (i & 1? 1.0f : 0.0f), 1.0f + i * 3.0f, 0);
			Body &wall = *mBodyInterface->CreateBody(BodyCreationSettings(box_shape, position, Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
			mBodyInterface->AddBody(wall.GetID(), EActivation::Activate);
		}
}
