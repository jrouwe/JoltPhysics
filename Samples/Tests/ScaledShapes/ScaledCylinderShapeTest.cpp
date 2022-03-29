// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/ScaledShapes/ScaledCylinderShapeTest.h>
#include <Jolt/Physics/Collision/Shape/CylinderShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(ScaledCylinderShapeTest) 
{ 
	JPH_ADD_BASE_CLASS(ScaledCylinderShapeTest, Test) 
}

void ScaledCylinderShapeTest::Initialize()
{
	// Floor
	CreateFloor();

	// Create cylinder
	RefConst<CylinderShape> cylinder_shape = new CylinderShape(3.0f, 2.0f);

	// Original shape
	Body &body1 = *mBodyInterface->CreateBody(BodyCreationSettings(cylinder_shape, Vec3(-20, 10, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
	mBodyInterface->AddBody(body1.GetID(), EActivation::Activate);

	// Uniformly scaled shape
	Body &body2 = *mBodyInterface->CreateBody(BodyCreationSettings(new ScaledShape(cylinder_shape, Vec3::sReplicate(0.25f)), Vec3(-10, 10, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
	mBodyInterface->AddBody(body2.GetID(), EActivation::Activate);

	// Non-uniform scaled shape
	Body &body3 = *mBodyInterface->CreateBody(BodyCreationSettings(new ScaledShape(cylinder_shape, Vec3(0.25f, 0.5f, 0.25f)), Vec3(0, 10, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
	mBodyInterface->AddBody(body3.GetID(), EActivation::Activate);

	// Flipped in 2 axis
	Body &body4 = *mBodyInterface->CreateBody(BodyCreationSettings(new ScaledShape(cylinder_shape, Vec3(-1.5f, -0.5f, 1.5f)), Vec3(10, 10, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
	mBodyInterface->AddBody(body4.GetID(), EActivation::Activate);

	// Inside out
	Body &body5 = *mBodyInterface->CreateBody(BodyCreationSettings(new ScaledShape(cylinder_shape, Vec3(-0.25f, 1.5f, 0.25f)), Vec3(20, 10, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
	mBodyInterface->AddBody(body5.GetID(), EActivation::Activate);
}
