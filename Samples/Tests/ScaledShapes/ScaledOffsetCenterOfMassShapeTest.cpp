// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/ScaledShapes/ScaledOffsetCenterOfMassShapeTest.h>
#include <Jolt/Physics/Collision/Shape/CylinderShape.h>
#include <Jolt/Physics/Collision/Shape/OffsetCenterOfMassShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(ScaledOffsetCenterOfMassShapeTest) 
{ 
	JPH_ADD_BASE_CLASS(ScaledOffsetCenterOfMassShapeTest, Test) 
}

void ScaledOffsetCenterOfMassShapeTest::Initialize() 
{
	// Floor
	Body &floor = CreateFloor();
	floor.SetFriction(1.0f);

	Ref<ShapeSettings> cylinder = new CylinderShapeSettings(1.0f, 0.1f);
	Ref<OffsetCenterOfMassShapeSettings> top = new OffsetCenterOfMassShapeSettings(Vec3(0, 1, 0), cylinder);
	Ref<OffsetCenterOfMassShapeSettings> bottom = new OffsetCenterOfMassShapeSettings(Vec3(0, -1, 0), cylinder);

	// Initial body rotation
	Quat rotation = Quat::sRotation(Vec3::sAxisZ(), 0.4f * JPH_PI);

	// Cylinder with center of mass moved to the top side
	Body &body_top = *mBodyInterface->CreateBody(BodyCreationSettings(new ScaledShapeSettings(top, Vec3(2.0f, 1.0f, 2.0f)), Vec3(-5, 5, 0), rotation, EMotionType::Dynamic, Layers::MOVING));
	body_top.SetFriction(1.0f);
	mBodyInterface->AddBody(body_top.GetID(), EActivation::Activate);

	// Cylinder with center of mass centered
	Body &body_center = *mBodyInterface->CreateBody(BodyCreationSettings(new ScaledShapeSettings(cylinder, Vec3(2.0f, 1.0f, 2.0f)), Vec3(0, 5, 0), rotation, EMotionType::Dynamic, Layers::MOVING));
	body_center.SetFriction(1.0f);
	mBodyInterface->AddBody(body_center.GetID(), EActivation::Activate);

	// Cylinder with center of mass moved to the bottom side
	Body &body_bottom = *mBodyInterface->CreateBody(BodyCreationSettings(new ScaledShapeSettings(bottom, Vec3(2.0f, 1.0f, 2.0f)), Vec3(5, 5, 0), rotation, EMotionType::Dynamic, Layers::MOVING));
	body_bottom.SetFriction(1.0f);
	mBodyInterface->AddBody(body_bottom.GetID(), EActivation::Activate);
}
