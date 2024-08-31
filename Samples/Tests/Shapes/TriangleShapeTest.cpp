// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Shapes/TriangleShapeTest.h>
#include <Jolt/Physics/Collision/Shape/TriangleShape.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(TriangleShapeTest)
{
	JPH_ADD_BASE_CLASS(TriangleShapeTest, Test)
}

void TriangleShapeTest::Initialize()
{
	// Single triangle
	RefConst<TriangleShape> triangle_shape = new TriangleShape(Vec3(-10, -1, 0), Vec3(0, 1, 10), Vec3(10, -2, -10), 0.01f);
	mBodyInterface->CreateAndAddBody(BodyCreationSettings(triangle_shape, RVec3::sZero(), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);

	// Create a box above the triangle
	RefConst<Shape> box_shape = new BoxShape(Vec3(0.2f, 0.2f, 0.4f), 0.01f);
	mBodyInterface->CreateAndAddBody(BodyCreationSettings(box_shape, RVec3(0, 5, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING), EActivation::Activate);
}
