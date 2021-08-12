// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Constraints/SliderConstraintTest.h>
#include <Physics/Collision/Shape/BoxShape.h>
#include <Physics/Collision/GroupFilterTable.h>
#include <Physics/Constraints/SliderConstraint.h>
#include <Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(SliderConstraintTest) 
{ 
	JPH_ADD_BASE_CLASS(SliderConstraintTest, Test) 
}

void SliderConstraintTest::Initialize()
{
	// Floor
	CreateFloor();
		
	const int cChainLength = 10;

	// Create group filter
	Ref<GroupFilterTable> group_filter = new GroupFilterTable(cChainLength);
	for (CollisionGroup::SubGroupID i = 0; i < cChainLength - 1; ++i)
		group_filter->DisableCollision(i, i + 1);

	// Create box
	float box_size = 4.0f;
	RefConst<Shape> box = new BoxShape(Vec3::sReplicate(0.5f * box_size));

	// Bodies attached through slider constraints
	{
		Vec3 position(0, 25, 0);
		Body &top = *mBodyInterface->CreateBody(BodyCreationSettings(box, position, Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING));
		top.SetCollisionGroup(CollisionGroup(group_filter, 0, 0));
		mBodyInterface->AddBody(top.GetID(), EActivation::DontActivate);

		Body *prev = &top;
		for (int i = 1; i < cChainLength; ++i)
		{
			position += Vec3(box_size, 0, 0);

			Body &segment = *mBodyInterface->CreateBody(BodyCreationSettings(box, position, Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
			segment.SetCollisionGroup(CollisionGroup(group_filter, 0, CollisionGroup::SubGroupID(i)));
			mBodyInterface->AddBody(segment.GetID(), EActivation::Activate);

			SliderConstraintSettings settings;
			settings.mSliderAxis = Quat::sRotation(Vec3::sAxisZ(), -DegreesToRadians(10)) * Vec3::sAxisX();
			settings.mLimitsMin = -5.0f;
			settings.mLimitsMax = 10.0f;
			mPhysicsSystem->AddConstraint(settings.Create(*prev, segment));

			prev = &segment;
		}
	}

	// Bodies attached through slider constraints with random rotations and translations
	{
		Vec3 position(0, 25, -20);
		Body &top = *mBodyInterface->CreateBody(BodyCreationSettings(box, position, Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING));
		top.SetCollisionGroup(CollisionGroup(group_filter, 1, 0));
		mBodyInterface->AddBody(top.GetID(), EActivation::DontActivate);

		default_random_engine random;
		uniform_real_distribution<float> displacement(-1.0f, 1.0f);
		Body *prev = &top;
		for (int i = 1; i < cChainLength; ++i)
		{
			position += Vec3(box_size + abs(displacement(random)), displacement(random), displacement(random));

			Body &segment = *mBodyInterface->CreateBody(BodyCreationSettings(box, position, Quat::sRandom(random), EMotionType::Dynamic, Layers::MOVING));
			segment.SetCollisionGroup(CollisionGroup(group_filter, 1, CollisionGroup::SubGroupID(i)));
			mBodyInterface->AddBody(segment.GetID(), EActivation::Activate);

			SliderConstraintSettings settings;
			settings.mSliderAxis = Quat::sRotation(Vec3::sAxisY(), displacement(random) * DegreesToRadians(20)) * Quat::sRotation(Vec3::sAxisZ(), -DegreesToRadians(10)) * Vec3::sAxisX();
			settings.mLimitsMin = -5.0f;
			settings.mLimitsMax = 10.0f;
			mPhysicsSystem->AddConstraint(settings.Create(*prev, segment));

			prev = &segment;
		}
	}
}