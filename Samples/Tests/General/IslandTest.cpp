// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/General/IslandTest.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(IslandTest) 
{ 
	JPH_ADD_BASE_CLASS(IslandTest, Test) 
}

void IslandTest::Initialize()
{
	// Floor
	CreateFloor();

	RefConst<Shape> box_shape = new BoxShape(Vec3(1.0f, 1.0f, 1.0f));

	// Walls
	for (int i = 0; i < 10; ++i)
		for (int j = i / 2; j < 10 - (i + 1) / 2; ++j)
			for (int k = 0; k < 8; ++k)
			{
				Vec3 position(-10 + j * 2.0f + (i & 1? 1.0f : 0.0f), 1.0f + i * 2.0f, 8.0f * (k - 4));
				Body &wall = *mBodyInterface->CreateBody(BodyCreationSettings(box_shape, position, Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
				mBodyInterface->AddBody(wall.GetID(), EActivation::Activate);
			}
}
