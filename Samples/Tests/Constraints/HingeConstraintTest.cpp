// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Constraints/HingeConstraintTest.h>
#include <Physics/Collision/Shape/BoxShape.h>
#include <Physics/Collision/GroupFilterTable.h>
#include <Physics/Constraints/HingeConstraint.h>
#include <Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(HingeConstraintTest) 
{ 
	JPH_ADD_BASE_CLASS(HingeConstraintTest, Test) 
}

void HingeConstraintTest::Initialize()
{
	// Floor
	CreateFloor();
		
	float box_size = 4.0f;
	RefConst<Shape> box = new BoxShape(Vec3::sReplicate(0.5f * box_size));

	const int cChainLength = 15;

	// Build a collision group filter that disables collision between adjacent bodies
	Ref<GroupFilterTable> group_filter = new GroupFilterTable(cChainLength);
	for (CollisionGroup::SubGroupID i = 0; i < cChainLength - 1; ++i)
		group_filter->DisableCollision(i, i + 1);

	// Bodies attached through hinge constraints
	Vec3 position(0, 50, 0);
	Body &top = *mBodyInterface->CreateBody(BodyCreationSettings(box, position, Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING));
	top.SetCollisionGroup(CollisionGroup(group_filter, 0, 0));
	mBodyInterface->AddBody(top.GetID(), EActivation::DontActivate);

	constexpr float min_angle = DegreesToRadians(-10.0f);
	constexpr float max_angle = DegreesToRadians(20.0f);

	Body *prev = &top;
	for (int i = 1; i < cChainLength; ++i)
	{
		position += Vec3(box_size, 0, 0);

		Body &segment = *mBodyInterface->CreateBody(BodyCreationSettings(box, position, Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
		segment.SetCollisionGroup(CollisionGroup(group_filter, 0, CollisionGroup::SubGroupID(i)));
		mBodyInterface->AddBody(segment.GetID(), EActivation::Activate);

		HingeConstraintSettings settings;
		if ((i & 1) == 0)
		{
			settings.mPoint1 = settings.mPoint2 = position + Vec3(-0.5f * box_size, 0, 0.5f * box_size);
			settings.mHingeAxis1 = settings.mHingeAxis2 = Vec3::sAxisY();
			settings.mNormalAxis1 = settings.mNormalAxis2 = Vec3::sAxisX();
		}
		else
		{ 
			settings.mPoint1 = settings.mPoint2 = position + Vec3(-0.5f * box_size, -0.5f * box_size, 0);
			settings.mHingeAxis1 = settings.mHingeAxis2 = Vec3::sAxisZ();
			settings.mNormalAxis1 = settings.mNormalAxis2 = Vec3::sAxisX();
		}
		settings.mLimitsMin = min_angle;
		settings.mLimitsMax = max_angle;
		mPhysicsSystem->AddConstraint(settings.Create(*prev, segment));

		prev = &segment;
	}
}