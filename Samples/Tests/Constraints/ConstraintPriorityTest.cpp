// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Constraints/ConstraintPriorityTest.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>
#include <Renderer/DebugRendererImp.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(ConstraintPriorityTest)
{
	JPH_ADD_BASE_CLASS(ConstraintPriorityTest, Test)
}

void ConstraintPriorityTest::Initialize()
{
	float box_size = 1.0f;
	RefConst<Shape> box = new BoxShape(Vec3(0.5f * box_size, 0.2f, 0.2f));

	const int num_bodies = 20;

	// Bodies attached through fixed constraints
	for (int priority = 0; priority < 2; ++priority)
	{
		RVec3 position(0, 10.0f, 0.2f * priority);
		Body &top = *mBodyInterface->CreateBody(BodyCreationSettings(box, position, Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING));
		mBodyInterface->AddBody(top.GetID(), EActivation::DontActivate);

		Body *prev = &top;
		for (int i = 1; i < num_bodies; ++i)
		{
			position += Vec3(box_size, 0, 0);

			Body &segment = *mBodyInterface->CreateBody(BodyCreationSettings(box, position, Quat::sIdentity(), EMotionType::Dynamic, Layers::NON_MOVING)); // Putting all bodies in the NON_MOVING layer so they won't collide
			mBodyInterface->AddBody(segment.GetID(), EActivation::Activate);

			FixedConstraintSettings settings;
			settings.mAutoDetectPoint = true;
			settings.mConstraintPriority = priority == 0? i : num_bodies - i; // Priority is reversed for one chain compared to the other
			Ref<Constraint> c = settings.Create(*prev, segment);
			mPhysicsSystem->AddConstraint(c);
			mConstraints.push_back(StaticCast<FixedConstraint>(c));

			prev = &segment;
		}
	}
}

void ConstraintPriorityTest::PostPhysicsUpdate(float inDeltaTime)
{
	for (FixedConstraint *c : mConstraints)
		mDebugRenderer->DrawText3D(0.5f * (c->GetBody1()->GetCenterOfMassPosition() + c->GetBody2()->GetCenterOfMassPosition()), StringFormat("Priority: %d", c->GetConstraintPriority()), Color::sWhite, 0.2f);
}
