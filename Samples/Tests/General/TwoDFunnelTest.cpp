// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2022 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/General/TwoDFunnelTest.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(TwoDFunnelTest)
{
	JPH_ADD_BASE_CLASS(TwoDFunnelTest, Test)
}

void TwoDFunnelTest::Initialize()
{
	// Floor
	CreateFloor();

	RefConst<Shape> wall = new BoxShape(Vec3(0.1f, 10, 1));

	// 2D funnel
	mBodyInterface->CreateAndAddBody(BodyCreationSettings(wall, RVec3(-12, 8, -5), Quat::sRotation(Vec3::sAxisZ(), 0.2f * JPH_PI), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);
	mBodyInterface->CreateAndAddBody(BodyCreationSettings(wall, RVec3(12, 8, -5), Quat::sRotation(Vec3::sAxisZ(), -0.2f * JPH_PI), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);

	// Shapes falling in 2D funnel
	Ref<Shape> shapes[] = {
		new SphereShape(0.5f),
		new BoxShape(Vec3::sReplicate(0.5f)),
		new CapsuleShape(0.2f, 0.3f)
	};
	BodyCreationSettings bcs(shapes[0], RVec3::sZero(), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
	bcs.mAllowedDOFs = EAllowedDOFs::Plane2D;
	for (int x = 0; x < 20; ++x)
		for (int y = 0; y < 10; ++y)
		{
			bcs.SetShape(shapes[(x * y) % size(shapes)]);
			bcs.mPosition = RVec3(-10.0_r + x, 10.0_r + y, -5.0_r);
			mBodyInterface->CreateAndAddBody(bcs, EActivation::Activate);
		}
}
