// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Shapes/OffsetCenterOfMassShapeTest.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/OffsetCenterOfMassShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(OffsetCenterOfMassShapeTest) 
{ 
	JPH_ADD_BASE_CLASS(OffsetCenterOfMassShapeTest, Test) 
}

void OffsetCenterOfMassShapeTest::Initialize() 
{
	// Floor
	Body &floor = CreateFloor();
	floor.SetFriction(1.0f);

	Ref<ShapeSettings> sphere = new SphereShapeSettings(1.0f);
	Ref<OffsetCenterOfMassShapeSettings> left = new OffsetCenterOfMassShapeSettings(Vec3(-1, 0, 0), sphere);
	Ref<OffsetCenterOfMassShapeSettings> right = new OffsetCenterOfMassShapeSettings(Vec3(1, 0, 0), sphere);

	// Sphere with center of mass moved to the left side
	Body &body_left = *mBodyInterface->CreateBody(BodyCreationSettings(left, Vec3(-5, 5, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
	body_left.SetFriction(1.0f);
	mBodyInterface->AddBody(body_left.GetID(), EActivation::Activate);

	// Sphere with center of mass centered
	Body &body_center = *mBodyInterface->CreateBody(BodyCreationSettings(sphere, Vec3(0, 5, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
	body_center.SetFriction(1.0f);
	mBodyInterface->AddBody(body_center.GetID(), EActivation::Activate);

	// Sphere with center of mass moved to the right side
	Body &body_right = *mBodyInterface->CreateBody(BodyCreationSettings(right, Vec3(5, 5, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
	body_right.SetFriction(1.0f);
	mBodyInterface->AddBody(body_right.GetID(), EActivation::Activate);
}
