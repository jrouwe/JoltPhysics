// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/ScaledShapes/ScaledOffsetCenterOfMassShapeTest.h>
#include <Jolt/Physics/Collision/Shape/CylinderShape.h>
#include <Jolt/Physics/Collision/Shape/OffsetCenterOfMassShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
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
	Body &body_top = *mBodyInterface->CreateBody(BodyCreationSettings(new ScaledShapeSettings(top, Vec3(2.0f, 1.0f, 2.0f)), RVec3(-5, 5, 0), rotation, EMotionType::Dynamic, Layers::MOVING));
	body_top.SetFriction(1.0f);
	mBodyInterface->AddBody(body_top.GetID(), EActivation::Activate);

	// Cylinder with center of mass centered
	Body &body_center = *mBodyInterface->CreateBody(BodyCreationSettings(new ScaledShapeSettings(cylinder, Vec3(2.0f, 1.0f, 2.0f)), RVec3(0, 5, 0), rotation, EMotionType::Dynamic, Layers::MOVING));
	body_center.SetFriction(1.0f);
	mBodyInterface->AddBody(body_center.GetID(), EActivation::Activate);

	// Cylinder with center of mass moved to the bottom side
	Body &body_bottom = *mBodyInterface->CreateBody(BodyCreationSettings(new ScaledShapeSettings(bottom, Vec3(2.0f, 1.0f, 2.0f)), RVec3(5, 5, 0), rotation, EMotionType::Dynamic, Layers::MOVING));
	body_bottom.SetFriction(1.0f);
	mBodyInterface->AddBody(body_bottom.GetID(), EActivation::Activate);

	// Shape that is scaled before the offset center of mass offset is applied
	ShapeRefC pre_scaled = OffsetCenterOfMassShapeSettings(Vec3(0, 0, 5.0f), new ScaledShape(new SphereShape(1.0f), JPH::Vec3::sReplicate(2.0f))).Create().Get();
	Body &body_pre_scaled = *mBodyInterface->CreateBody(BodyCreationSettings(pre_scaled, RVec3(0, 5, -15), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
	mBodyInterface->AddBody(body_pre_scaled.GetID(), EActivation::Activate);

	// Shape that is scaled after the offset center of mass offset is applied
	ShapeRefC post_scaled = new ScaledShape(OffsetCenterOfMassShapeSettings(Vec3(0, 0, 5.0f), new SphereShape(1.0f)).Create().Get(), JPH::Vec3::sReplicate(2.0f));
	Body &body_post_scaled = *mBodyInterface->CreateBody(BodyCreationSettings(post_scaled, RVec3(5, 5, -15), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
	mBodyInterface->AddBody(body_post_scaled.GetID(), EActivation::Activate);
}
