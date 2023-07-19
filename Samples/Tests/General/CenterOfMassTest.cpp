// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/General/CenterOfMassTest.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(CenterOfMassTest)
{
	JPH_ADD_BASE_CLASS(CenterOfMassTest, Test)
}

void CenterOfMassTest::Initialize()
{
	// Floor
	CreateFloor();

	// Compound shape with center of mass offset
	Ref<StaticCompoundShapeSettings> compound_shape1 = new StaticCompoundShapeSettings;
	compound_shape1->AddShape(Vec3(10, 0, 0), Quat::sIdentity(), new SphereShape(2));
	Body &body1 = *mBodyInterface->CreateBody(BodyCreationSettings(compound_shape1, RVec3(0, 10.0f, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
	mBodyInterface->AddBody(body1.GetID(), EActivation::Activate);

	// Create box with center of mass offset
	Array<Vec3> box;
	box.push_back(Vec3(10, 10, 10));
	box.push_back(Vec3(5, 10, 10));
	box.push_back(Vec3(10, 5, 10));
	box.push_back(Vec3(5, 5, 10));
	box.push_back(Vec3(10, 10, 5));
	box.push_back(Vec3(5, 10, 5));
	box.push_back(Vec3(10, 5, 5));
	box.push_back(Vec3(5, 5, 5));
	Body &body2 = *mBodyInterface->CreateBody(BodyCreationSettings(new ConvexHullShapeSettings(box), RVec3(0, 10.0f, 20.0f), Quat::sRotation(Vec3::sAxisX(), 0.25f * JPH_PI), EMotionType::Dynamic, Layers::MOVING));
	mBodyInterface->AddBody(body2.GetID(), EActivation::Activate);

	// Compound
	Ref<StaticCompoundShapeSettings> compound_shape2 = new StaticCompoundShapeSettings;
	Quat rotation = Quat::sRotation(Vec3::sAxisX(), 0.25f * JPH_PI);
	compound_shape2->AddShape(Vec3(10, 0, 0), rotation, new CapsuleShape(5, 1));
	compound_shape2->AddShape(rotation * Vec3(10, -5, 0), Quat::sIdentity(), new SphereShape(4));
	compound_shape2->AddShape(rotation * Vec3(10, 5, 0), Quat::sIdentity(), new SphereShape(2));
	Body &body3 = *mBodyInterface->CreateBody(BodyCreationSettings(compound_shape2, RVec3(0, 10.0f, 40.0f), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
	mBodyInterface->AddBody(body3.GetID(), EActivation::Activate);
}
