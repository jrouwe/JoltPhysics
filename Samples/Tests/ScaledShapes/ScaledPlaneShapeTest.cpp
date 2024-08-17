// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/ScaledShapes/ScaledPlaneShapeTest.h>
#include <Jolt/Physics/Collision/Shape/PlaneShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(ScaledPlaneShapeTest)
{
	JPH_ADD_BASE_CLASS(ScaledPlaneShapeTest, Test)
}

void ScaledPlaneShapeTest::Initialize()
{
	// Floor
	CreateFloor();

	RefConst<ShapeSettings> plane_shape = new PlaneShapeSettings(Plane(Vec3(0.1f, 1.0f, 0.1f).Normalized(), -0.5f), nullptr, 5.0f);

	// Original shape
	Body &body1 = *mBodyInterface->CreateBody(BodyCreationSettings(plane_shape, RVec3(-60, 10, 0), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING));
	mBodyInterface->AddBody(body1.GetID(), EActivation::DontActivate);

	// Uniformly scaled shape < 1
	Body &body2 = *mBodyInterface->CreateBody(BodyCreationSettings(new ScaledShapeSettings(plane_shape, Vec3::sReplicate(0.5f)), RVec3(-40, 10, 0), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING));
	mBodyInterface->AddBody(body2.GetID(), EActivation::DontActivate);

	// Uniformly scaled shape > 1
	Body &body3 = *mBodyInterface->CreateBody(BodyCreationSettings(new ScaledShapeSettings(plane_shape, Vec3::sReplicate(1.5f)), RVec3(-20, 10, 0), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING));
	mBodyInterface->AddBody(body3.GetID(), EActivation::DontActivate);

	// Non-uniform scaled shape
	Body &body4 = *mBodyInterface->CreateBody(BodyCreationSettings(new ScaledShapeSettings(plane_shape, Vec3(0.5f, 1.0f, 1.5f)), RVec3(0, 10, 0), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING));
	mBodyInterface->AddBody(body4.GetID(), EActivation::DontActivate);

	// Flipped in 2 axis
	Body &body5 = *mBodyInterface->CreateBody(BodyCreationSettings(new ScaledShapeSettings(plane_shape, Vec3(-0.5f, 1.0f, -1.5f)), RVec3(20, 10, 0), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING));
	mBodyInterface->AddBody(body5.GetID(), EActivation::DontActivate);

	// Inside out
	Body &body6 = *mBodyInterface->CreateBody(BodyCreationSettings(new ScaledShapeSettings(plane_shape, Vec3(-0.5f, 1.0f, 1.5f)), RVec3(40, 10, 0), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING));
	mBodyInterface->AddBody(body6.GetID(), EActivation::DontActivate);

	// Upside down
	Body &body7 = *mBodyInterface->CreateBody(BodyCreationSettings(new ScaledShapeSettings(plane_shape, Vec3(0.5f, -1.0f, 1.5f)), RVec3(60, 10, 0), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING));
	mBodyInterface->AddBody(body7.GetID(), EActivation::DontActivate);

	// Create a number of balls above the planes
	RefConst<Shape> sphere_shape = new SphereShape(0.2f);
	RefConst<Shape> box_shape = new BoxShape(Vec3(0.2f, 0.2f, 0.4f), 0.01f);
	for (int i = 0; i < 7; ++i)
		for (int j = 0; j < 5; ++j)
		{
			Body &dynamic = *mBodyInterface->CreateBody(BodyCreationSettings((j & 1)? box_shape : sphere_shape, RVec3(-60.0f + 20.0f * i, 15.0f + 0.5f * j, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
			mBodyInterface->AddBody(dynamic.GetID(), EActivation::Activate);
		}
}
