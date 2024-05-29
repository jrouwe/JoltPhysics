// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/ScaledShapes/ScaledSphereShapeTest.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(ScaledSphereShapeTest)
{
	JPH_ADD_BASE_CLASS(ScaledSphereShapeTest, Test)
}

void ScaledSphereShapeTest::Initialize()
{
	// Floor
	CreateFloor();

	// Create sphere
	RefConst<SphereShape> sphere_shape = new SphereShape(2.0f);

	// Original shape
	Body &body1 = *mBodyInterface->CreateBody(BodyCreationSettings(sphere_shape, RVec3(-20, 10, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
	mBodyInterface->AddBody(body1.GetID(), EActivation::Activate);

	// Uniformly scaled shape < 1
	Body &body2 = *mBodyInterface->CreateBody(BodyCreationSettings(new ScaledShape(sphere_shape, Vec3::sReplicate(0.25f)), RVec3(-10, 10, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
	mBodyInterface->AddBody(body2.GetID(), EActivation::Activate);

	// Uniformly scaled shape > 1
	Body &body3 = *mBodyInterface->CreateBody(BodyCreationSettings(new ScaledShape(sphere_shape, Vec3::sReplicate(2.0f)), RVec3(0, 10, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
	mBodyInterface->AddBody(body3.GetID(), EActivation::Activate);

	// Flipped in 2 axis
	Body &body4 = *mBodyInterface->CreateBody(BodyCreationSettings(new ScaledShape(sphere_shape, Vec3(-0.25f, 0.25f, -0.25f)), RVec3(10, 10, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
	mBodyInterface->AddBody(body4.GetID(), EActivation::Activate);

	// Inside out
	Body &body5 = *mBodyInterface->CreateBody(BodyCreationSettings(new ScaledShape(sphere_shape, Vec3::sReplicate(-0.25f)), RVec3(20, 10, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
	mBodyInterface->AddBody(body5.GetID(), EActivation::Activate);
}
