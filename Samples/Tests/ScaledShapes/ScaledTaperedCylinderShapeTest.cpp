// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/ScaledShapes/ScaledTaperedCylinderShapeTest.h>
#include <Jolt/Physics/Collision/Shape/TaperedCylinderShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(ScaledTaperedCylinderShapeTest)
{
	JPH_ADD_BASE_CLASS(ScaledTaperedCylinderShapeTest, Test)
}

void ScaledTaperedCylinderShapeTest::Initialize()
{
	// Floor
	CreateFloor();

	// Create tapered cylinder
	RefConst<Shape> tapered_cylinder_shape = TaperedCylinderShapeSettings(2.0f, 0.75f, 1.25f).Create().Get();

	// Original shape
	mBodyInterface->CreateAndAddBody(BodyCreationSettings(tapered_cylinder_shape, RVec3(-20, 10, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING), EActivation::Activate);

	// Uniformly scaled shape
	mBodyInterface->CreateAndAddBody(BodyCreationSettings(new ScaledShape(tapered_cylinder_shape, Vec3::sReplicate(0.25f)), RVec3(-10, 10, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING), EActivation::Activate);

	// Non-uniform scaled shape
	mBodyInterface->CreateAndAddBody(BodyCreationSettings(new ScaledShape(tapered_cylinder_shape, Vec3(0.25f, 0.5f, 0.25f)), RVec3(0, 10, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING), EActivation::Activate);

	// Flipped in 2 axis
	mBodyInterface->CreateAndAddBody(BodyCreationSettings(new ScaledShape(tapered_cylinder_shape, Vec3(-1.5f, -0.5f, 1.5f)), RVec3(10, 10, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING), EActivation::Activate);

	// Inside out
	mBodyInterface->CreateAndAddBody(BodyCreationSettings(new ScaledShape(tapered_cylinder_shape, Vec3(-0.25f, 1.5f, 0.25f)), RVec3(20, 10, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING), EActivation::Activate);
}
