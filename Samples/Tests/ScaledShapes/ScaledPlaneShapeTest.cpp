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
	mBodyInterface->CreateAndAddBody(BodyCreationSettings(plane_shape, RVec3(-60, 10, 0), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);

	// Uniformly scaled shape < 1
	mBodyInterface->CreateAndAddBody(BodyCreationSettings(new ScaledShapeSettings(plane_shape, Vec3::sReplicate(0.5f)), RVec3(-40, 10, 0), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);

	// Uniformly scaled shape > 1
	mBodyInterface->CreateAndAddBody(BodyCreationSettings(new ScaledShapeSettings(plane_shape, Vec3::sReplicate(1.5f)), RVec3(-20, 10, 0), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);

	// Non-uniform scaled shape
	mBodyInterface->CreateAndAddBody(BodyCreationSettings(new ScaledShapeSettings(plane_shape, Vec3(0.5f, 1.0f, 1.5f)), RVec3(0, 10, 0), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);

	// Flipped in 2 axis
	mBodyInterface->CreateAndAddBody(BodyCreationSettings(new ScaledShapeSettings(plane_shape, Vec3(-0.5f, 1.0f, -1.5f)), RVec3(20, 10, 0), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);

	// Inside out
	mBodyInterface->CreateAndAddBody(BodyCreationSettings(new ScaledShapeSettings(plane_shape, Vec3(-0.5f, 1.0f, 1.5f)), RVec3(40, 10, 0), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);

	// Upside down
	mBodyInterface->CreateAndAddBody(BodyCreationSettings(new ScaledShapeSettings(plane_shape, Vec3(0.5f, -1.0f, 1.5f)), RVec3(60, 10, 0), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);

	// Create a number of balls above the planes
	RefConst<Shape> sphere_shape = new SphereShape(0.2f);
	RefConst<Shape> box_shape = new BoxShape(Vec3(0.2f, 0.2f, 0.4f), 0.01f);
	for (int i = 0; i < 7; ++i)
		for (int j = 0; j < 5; ++j)
			mBodyInterface->CreateAndAddBody(BodyCreationSettings((j & 1)? box_shape : sphere_shape, RVec3(-60.0f + 20.0f * i, 15.0f + 0.5f * j, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING), EActivation::Activate);
}
