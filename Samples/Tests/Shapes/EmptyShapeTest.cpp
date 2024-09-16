// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Shapes/EmptyShapeTest.h>
#include <Jolt/Physics/Collision/Shape/EmptyShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(EmptyShapeTest)
{
	JPH_ADD_BASE_CLASS(EmptyShapeTest, Test)
}

void EmptyShapeTest::Initialize()
{
	// Floor
	CreateFloor();

	// Empty shape
	mBodyInterface->CreateAndAddBody(BodyCreationSettings(new EmptyShape(), RVec3(0, 10, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING), EActivation::Activate);
}
