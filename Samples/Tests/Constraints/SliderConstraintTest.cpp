// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Constraints/SliderConstraintTest.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/GroupFilterTable.h>
#include <Jolt/Physics/Constraints/SliderConstraint.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
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
			settings.SetPoint(*prev, segment);
			settings.SetSliderAxis(Quat::sRotation(Vec3::sAxisZ(), -DegreesToRadians(10)).RotateAxisX());
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
			settings.SetPoint(*prev, segment);
			settings.SetSliderAxis(Quat::sRotation(Vec3::sAxisY(), displacement(random) * DegreesToRadians(20)) * Quat::sRotation(Vec3::sAxisZ(), -DegreesToRadians(10)).RotateAxisX());
			settings.mLimitsMin = -5.0f;
			settings.mLimitsMax = 10.0f;
			mPhysicsSystem->AddConstraint(settings.Create(*prev, segment));

			prev = &segment;
		}
	}

	// Two light bodies attached to a heavy body (gives issues if the wrong anchor point is chosen)
	Body *light1 = mBodyInterface->CreateBody(BodyCreationSettings(new BoxShape(Vec3::sReplicate(0.1f)), Vec3(-5.0f, 7.0f, -5.2f), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
	mBodyInterface->AddBody(light1->GetID(), EActivation::Activate);
	Body *heavy = mBodyInterface->CreateBody(BodyCreationSettings(new BoxShape(Vec3::sReplicate(5.0f)), Vec3(-5.0f, 7.0f, 0.0f), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
	mBodyInterface->AddBody(heavy->GetID(), EActivation::Activate);
	Body *light2 = mBodyInterface->CreateBody(BodyCreationSettings(new BoxShape(Vec3::sReplicate(0.1f)), Vec3(-5.0f, 7.0f, 5.2f), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
	mBodyInterface->AddBody(light2->GetID(), EActivation::Activate);

	// Note: This violates the recommendation that body 1 is heavier than body 2, therefore this constraint will not work well (the rotation constraint will not be solved accurately)
	SliderConstraintSettings slider1;
	slider1.SetPoint(*light1, *heavy);
	slider1.SetSliderAxis(Vec3::sAxisZ());
	slider1.mLimitsMin = 0.0f;
	slider1.mLimitsMax = 1.0f;
	mPhysicsSystem->AddConstraint(slider1.Create(*light1, *heavy));

	// This constraint has the heavy body as body 1 so will work fine
	SliderConstraintSettings slider2;
	slider2.SetPoint(*heavy, *light2);
	slider2.SetSliderAxis(Vec3::sAxisZ());
	slider2.mLimitsMin = 0.0f;
	slider2.mLimitsMax = 1.0f;
	mPhysicsSystem->AddConstraint(slider2.Create(*heavy, *light2));
}
