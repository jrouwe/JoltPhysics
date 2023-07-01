// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/General/AxisLockTest.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Constraints/DistanceConstraint.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Renderer/DebugRendererImp.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(AxisLockTest) 
{ 
	JPH_ADD_BASE_CLASS(AxisLockTest, Test) 
}

void AxisLockTest::Initialize()
{
	// Floor
	CreateFloor();

	Vec3 box_size(0.5f, 1.0f, 2.0f);
	RefConst<Shape> box_shape = new BoxShape(box_size);

	for (int locked_axis = 0; locked_axis < 0b111111; ++locked_axis)
	{
		float x = -35.0f + 10.0f * (locked_axis & 0b111);
		float z = -35.0f + 10.0f * ((locked_axis >> 3) & 0b111);

		// Create body 
		BodyCreationSettings bcs(box_shape, RVec3(x, 10, z), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
		bcs.mLockedAxis = (ELockedAxis)locked_axis;
		BodyID id = mBodyInterface->CreateAndAddBody(bcs, EActivation::Activate);
		mBodies.push_back(id);

		// Create a constraint
		DistanceConstraintSettings dcs;
		dcs.mPoint1 = bcs.mPosition + Vec3(5, 5, 5);
		dcs.mPoint2 = bcs.mPosition + box_size;
		mPhysicsSystem->AddConstraint(mBodyInterface->CreateConstraint(&dcs, BodyID(), id));
	}
}

void AxisLockTest::PostPhysicsUpdate(float inDeltaTime)
{
	// Draw the mass scale
	for (BodyID id : mBodies)
	{
		BodyLockRead body_lock(mPhysicsSystem->GetBodyLockInterface(), id);
		if (body_lock.Succeeded())
		{
			const Body &body = body_lock.GetBody();
			String locked_axis_str = "";
			ELockedAxis locked_axis = body.GetMotionProperties()->GetLockedAxis();
			if ((locked_axis & ELockedAxis::TranslationX) == ELockedAxis::None)
				locked_axis_str += "X ";
			if ((locked_axis & ELockedAxis::TranslationY) == ELockedAxis::None)
				locked_axis_str += "Y ";
			if ((locked_axis & ELockedAxis::TranslationZ) == ELockedAxis::None)
				locked_axis_str += "Z ";
			if ((locked_axis & ELockedAxis::RotationX) == ELockedAxis::None)
				locked_axis_str += "RX ";
			if ((locked_axis & ELockedAxis::RotationY) == ELockedAxis::None)
				locked_axis_str += "RY ";
			if ((locked_axis & ELockedAxis::RotationZ) == ELockedAxis::None)
				locked_axis_str += "RZ ";
			DebugRenderer::sInstance->DrawText3D(body.GetPosition(), locked_axis_str, Color::sWhite);
		}
	}
}
