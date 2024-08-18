// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Shapes/PlaneShapeTest.h>
#include <Jolt/Physics/Collision/Shape/PlaneShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(PlaneShapeTest)
{
	JPH_ADD_BASE_CLASS(PlaneShapeTest, Test)
}

void PlaneShapeTest::Initialize()
{
	// Create a plane as floor
	mBodyInterface->CreateAndAddBody(BodyCreationSettings(new PlaneShape(Plane(Vec3(0.1f, 1.0f, 0.0f).Normalized(), 1.0f), nullptr, 100), RVec3(0, 0, 0), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);

	// Add some shapes
	mBodyInterface->CreateAndAddBody(BodyCreationSettings(new SphereShape(0.5f), RVec3(0, 1, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING), EActivation::Activate);
	mBodyInterface->CreateAndAddBody(BodyCreationSettings(new BoxShape(Vec3::sReplicate(0.5f)), RVec3(2, 1, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING), EActivation::Activate);
}
