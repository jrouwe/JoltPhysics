// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/General/AllowedDOFsTest.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Constraints/DistanceConstraint.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Renderer/DebugRendererImp.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(AllowedDOFsTest)
{
	JPH_ADD_BASE_CLASS(AllowedDOFsTest, Test)
}

void AllowedDOFsTest::Initialize()
{
	// Floor
	CreateFloor();

	Vec3 box_size(0.5f, 1.0f, 2.0f);
	RefConst<Shape> box_shape = new BoxShape(box_size);

	for (int allowed_dofs = 1; allowed_dofs <= 0b111111; ++allowed_dofs)
	{
		float x = -35.0f + 10.0f * (allowed_dofs & 0b111);
		float z = -35.0f + 10.0f * ((allowed_dofs >> 3) & 0b111);

		// Create body
		BodyCreationSettings bcs(box_shape, RVec3(x, 10, z), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
		bcs.mAllowedDOFs = (EAllowedDOFs)allowed_dofs;
		BodyID id = mBodyInterface->CreateAndAddBody(bcs, EActivation::Activate);
		mBodies.push_back(id);

		// Create a constraint
		DistanceConstraintSettings dcs;
		dcs.mPoint1 = bcs.mPosition + Vec3(5, 5, 5);
		dcs.mPoint2 = bcs.mPosition + box_size;
		dcs.mMinDistance = 0.0f;
		dcs.mMaxDistance = sqrt(3.0f) * 5.0f + 1.0f;
		mPhysicsSystem->AddConstraint(mBodyInterface->CreateConstraint(&dcs, BodyID(), id));
	}
}

void AllowedDOFsTest::PostPhysicsUpdate(float inDeltaTime)
{
	// Draw degrees of freedom
	for (BodyID id : mBodies)
	{
		BodyLockRead body_lock(mPhysicsSystem->GetBodyLockInterface(), id);
		if (body_lock.Succeeded())
		{
			const Body &body = body_lock.GetBody();
			String allowed_dofs_str = "";
			EAllowedDOFs allowed_dofs = body.GetMotionProperties()->GetAllowedDOFs();
			if ((allowed_dofs & EAllowedDOFs::TranslationX) == EAllowedDOFs::TranslationX)
				allowed_dofs_str += "X ";
			if ((allowed_dofs & EAllowedDOFs::TranslationY) == EAllowedDOFs::TranslationY)
				allowed_dofs_str += "Y ";
			if ((allowed_dofs & EAllowedDOFs::TranslationZ) == EAllowedDOFs::TranslationZ)
				allowed_dofs_str += "Z ";
			if ((allowed_dofs & EAllowedDOFs::RotationX) == EAllowedDOFs::RotationX)
				allowed_dofs_str += "RX ";
			if ((allowed_dofs & EAllowedDOFs::RotationY) == EAllowedDOFs::RotationY)
				allowed_dofs_str += "RY ";
			if ((allowed_dofs & EAllowedDOFs::RotationZ) == EAllowedDOFs::RotationZ)
				allowed_dofs_str += "RZ ";
			DebugRenderer::sInstance->DrawText3D(body.GetPosition(), allowed_dofs_str, Color::sWhite);
		}
	}
}
