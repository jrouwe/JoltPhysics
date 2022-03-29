// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/ScaledShapes/ScaledCapsuleShapeTest.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(ScaledCapsuleShapeTest) 
{ 
	JPH_ADD_BASE_CLASS(ScaledCapsuleShapeTest, Test) 
}

void ScaledCapsuleShapeTest::Initialize()
{
	// Floor
	CreateFloor();

	// Create capsule
	RefConst<CapsuleShape> capsule_shape = new CapsuleShape(2.0f, 0.5f);

	// Original shape
	Body &body1 = *mBodyInterface->CreateBody(BodyCreationSettings(capsule_shape, Vec3(-20, 10, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
	mBodyInterface->AddBody(body1.GetID(), EActivation::Activate);

	// Uniformly scaled shape < 1
	Body &body2 = *mBodyInterface->CreateBody(BodyCreationSettings(new ScaledShape(capsule_shape, Vec3::sReplicate(0.25f)), Vec3(-10, 10, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
	mBodyInterface->AddBody(body2.GetID(), EActivation::Activate);

	// Uniformly scaled shape > 1
	Body &body3 = *mBodyInterface->CreateBody(BodyCreationSettings(new ScaledShape(capsule_shape, Vec3::sReplicate(2.0f)), Vec3(0, 10, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
	mBodyInterface->AddBody(body3.GetID(), EActivation::Activate);

	// Flipped in 2 axis
	Body &body4 = *mBodyInterface->CreateBody(BodyCreationSettings(new ScaledShape(capsule_shape, Vec3(-1.5f, -1.5f, 1.5f)), Vec3(10, 10, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
	mBodyInterface->AddBody(body4.GetID(), EActivation::Activate);

	// Inside out
	Body &body5 = *mBodyInterface->CreateBody(BodyCreationSettings(new ScaledShape(capsule_shape, Vec3(-0.75f, 0.75f, 0.75f)), Vec3(20, 10, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
	mBodyInterface->AddBody(body5.GetID(), EActivation::Activate);
}
