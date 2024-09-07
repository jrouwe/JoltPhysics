// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/ScaledShapes/ScaledTaperedCapsuleShapeTest.h>
#include <Jolt/Physics/Collision/Shape/TaperedCapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(ScaledTaperedCapsuleShapeTest)
{
	JPH_ADD_BASE_CLASS(ScaledTaperedCapsuleShapeTest, Test)
}

void ScaledTaperedCapsuleShapeTest::Initialize()
{
	// Floor
	CreateFloor();

	// Create tapered capsule
	RefConst<ShapeSettings> tapered_capsule_shape = new TaperedCapsuleShapeSettings(2.0f, 0.75f, 1.25f);

	// Original shape
	mBodyInterface->CreateAndAddBody(BodyCreationSettings(tapered_capsule_shape, RVec3(-20, 10, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING), EActivation::Activate);

	// Uniformly scaled shape < 1
	mBodyInterface->CreateAndAddBody(BodyCreationSettings(new ScaledShapeSettings(tapered_capsule_shape, Vec3::sReplicate(0.25f)), RVec3(-10, 10, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING), EActivation::Activate);

	// Uniformly scaled shape > 1
	mBodyInterface->CreateAndAddBody(BodyCreationSettings(new ScaledShapeSettings(tapered_capsule_shape, Vec3::sReplicate(2.0f)), RVec3(0, 10, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING), EActivation::Activate);

	// Flipped in 2 axis
	mBodyInterface->CreateAndAddBody(BodyCreationSettings(new ScaledShapeSettings(tapered_capsule_shape, Vec3(-1.5f, -1.5f, 1.5f)), RVec3(10, 10, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING), EActivation::Activate);

	// Inside out
	mBodyInterface->CreateAndAddBody(BodyCreationSettings(new ScaledShapeSettings(tapered_capsule_shape, Vec3::sReplicate(-0.75f)), RVec3(20, 10, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING), EActivation::Activate);
}
