// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
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
	CollisionGroup::GroupID group_id = 0;

	// Create box
	float box_size = 4.0f;
	RefConst<Shape> box = new BoxShape(Vec3::sReplicate(0.5f * box_size));

	// Bodies attached through slider constraints
	for (int randomness = 0; randomness < 2; ++randomness)
	{
		RVec3 position(0, 25.0f, -randomness * 20.0f);
		Body &top = *mBodyInterface->CreateBody(BodyCreationSettings(box, position, Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING));
		top.SetCollisionGroup(CollisionGroup(group_filter, group_id, 0));
		mBodyInterface->AddBody(top.GetID(), EActivation::DontActivate);

		default_random_engine random;
		uniform_real_distribution<float> displacement(-1.0f, 1.0f);

		Body *prev = &top;
		for (int i = 1; i < cChainLength; ++i)
		{
			Quat rotation;
			Vec3 slider_axis;
			if (randomness == 0)
			{
				position += Vec3(box_size, 0, 0);
				rotation = Quat::sIdentity();
				slider_axis = Quat::sRotation(Vec3::sAxisZ(), -DegreesToRadians(10)).RotateAxisX();
			}
			else
			{
				position += Vec3(box_size + abs(displacement(random)), displacement(random), displacement(random));
				rotation = Quat::sRandom(random);
				slider_axis = Quat::sRotation(Vec3::sAxisY(), displacement(random) * DegreesToRadians(20)) * Quat::sRotation(Vec3::sAxisZ(), -DegreesToRadians(10)).RotateAxisX();
			}

			Body &segment = *mBodyInterface->CreateBody(BodyCreationSettings(box, position, rotation, EMotionType::Dynamic, Layers::MOVING));
			segment.SetCollisionGroup(CollisionGroup(group_filter, group_id, CollisionGroup::SubGroupID(i)));
			mBodyInterface->AddBody(segment.GetID(), EActivation::Activate);

			SliderConstraintSettings settings;
			settings.mAutoDetectPoint = true;
			settings.SetSliderAxis(slider_axis);
			settings.mLimitsMin = -5.0f;
			settings.mLimitsMax = 10.0f;
			mPhysicsSystem->AddConstraint(settings.Create(*prev, segment));

			prev = &segment;
		}

		group_id++;
	}

	{
		// Two light bodies attached to a heavy body (gives issues if the wrong anchor point is chosen)
		Body *light1 = mBodyInterface->CreateBody(BodyCreationSettings(new BoxShape(Vec3::sReplicate(0.1f)), RVec3(-5.0f, 7.0f, -5.2f), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
		light1->SetCollisionGroup(CollisionGroup(group_filter, group_id, 0));
		mBodyInterface->AddBody(light1->GetID(), EActivation::Activate);
		Body *heavy = mBodyInterface->CreateBody(BodyCreationSettings(new BoxShape(Vec3::sReplicate(5.0f)), RVec3(-5.0f, 7.0f, 0.0f), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
		heavy->SetCollisionGroup(CollisionGroup(group_filter, group_id, 1));
		mBodyInterface->AddBody(heavy->GetID(), EActivation::Activate);
		Body *light2 = mBodyInterface->CreateBody(BodyCreationSettings(new BoxShape(Vec3::sReplicate(0.1f)), RVec3(-5.0f, 7.0f, 5.2f), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
		light2->SetCollisionGroup(CollisionGroup(group_filter, group_id, 2));
		mBodyInterface->AddBody(light2->GetID(), EActivation::Activate);
		++group_id;

		// Note: This violates the recommendation that body 1 is heavier than body 2, therefore this constraint will not work well (the rotation constraint will not be solved accurately)
		SliderConstraintSettings slider1;
		slider1.mAutoDetectPoint = true;
		slider1.SetSliderAxis(Vec3::sAxisZ());
		slider1.mLimitsMin = 0.0f;
		slider1.mLimitsMax = 1.0f;
		mPhysicsSystem->AddConstraint(slider1.Create(*light1, *heavy));

		// This constraint has the heavy body as body 1 so will work fine
		SliderConstraintSettings slider2;
		slider2.mAutoDetectPoint = true;
		slider2.SetSliderAxis(Vec3::sAxisZ());
		slider2.mLimitsMin = 0.0f;
		slider2.mLimitsMax = 1.0f;
		mPhysicsSystem->AddConstraint(slider2.Create(*heavy, *light2));
	}

	{
		// Two bodies vertically stacked with a slider constraint
		Body *vert1 = mBodyInterface->CreateBody(BodyCreationSettings(new BoxShape(Vec3::sOne()), RVec3(5, 9, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
		vert1->SetCollisionGroup(CollisionGroup(group_filter, group_id, 0));
		mBodyInterface->AddBody(vert1->GetID(), EActivation::Activate);
		Body *vert2 = mBodyInterface->CreateBody(BodyCreationSettings(new BoxShape(Vec3::sOne()), RVec3(5, 3, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
		vert2->SetCollisionGroup(CollisionGroup(group_filter, group_id, 1));
		mBodyInterface->AddBody(vert2->GetID(), EActivation::Activate);
		++group_id;

		SliderConstraintSettings slider;
		slider.mAutoDetectPoint = true;
		slider.SetSliderAxis(Vec3::sAxisY());
		slider.mLimitsMin = 0.0f;
		slider.mLimitsMax = 2.0f;
		mPhysicsSystem->AddConstraint(slider.Create(*vert1, *vert2));
	}

	{
		// Two bodies vertically stacked with a slider constraint using soft limits
		Body *vert1 = mBodyInterface->CreateBody(BodyCreationSettings(new BoxShape(Vec3::sOne()), RVec3(10, 9, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
		vert1->SetCollisionGroup(CollisionGroup(group_filter, group_id, 0));
		mBodyInterface->AddBody(vert1->GetID(), EActivation::Activate);
		Body *vert2 = mBodyInterface->CreateBody(BodyCreationSettings(new BoxShape(Vec3::sOne()), RVec3(10, 3, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
		vert2->SetCollisionGroup(CollisionGroup(group_filter, group_id, 1));
		mBodyInterface->AddBody(vert2->GetID(), EActivation::Activate);
		++group_id;

		SliderConstraintSettings slider;
		slider.mAutoDetectPoint = true;
		slider.SetSliderAxis(Vec3::sAxisY());
		slider.mLimitsMin = 0.0f;
		slider.mLimitsMax = 2.0f;
		slider.mLimitsSpringSettings.mFrequency = 1.0f;
		slider.mLimitsSpringSettings.mDamping = 0.5f;
		mPhysicsSystem->AddConstraint(slider.Create(*vert1, *vert2));
	}
}
