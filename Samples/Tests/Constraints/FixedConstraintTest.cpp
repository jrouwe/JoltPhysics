// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Constraints/FixedConstraintTest.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/GroupFilterTable.h>
#include <Jolt/Physics/Constraints/FixedConstraint.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(FixedConstraintTest) 
{ 
	JPH_ADD_BASE_CLASS(FixedConstraintTest, Test) 
}

void FixedConstraintTest::Initialize()
{
	// Floor
	CreateFloor();
		
	float box_size = 4.0f;
	RefConst<Shape> box = new BoxShape(Vec3::sReplicate(0.5f * box_size));

	const int num_bodies = 10;

	// Build a collision group filter that disables collision between adjacent bodies
	Ref<GroupFilterTable> group_filter = new GroupFilterTable(num_bodies);
	for (CollisionGroup::SubGroupID i = 0; i < num_bodies - 1; ++i)
		group_filter->DisableCollision(i, i + 1);

	// Bodies attached through fixed constraints
	for (int randomness = 0; randomness < 2; ++randomness)
	{
		CollisionGroup::GroupID group_id = CollisionGroup::GroupID(randomness);

		Vec3 position(0, 25.0f, -randomness * 20.0f);
		Body &top = *mBodyInterface->CreateBody(BodyCreationSettings(box, position, Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING));
		top.SetCollisionGroup(CollisionGroup(group_filter, group_id, 0));
		mBodyInterface->AddBody(top.GetID(), EActivation::DontActivate);

		default_random_engine random;
		uniform_real_distribution<float> displacement(-1.0f, 1.0f);

		Body *prev = &top;
		for (int i = 1; i < num_bodies; ++i)
		{
			Quat rotation;
			if (randomness == 0)
			{
				position += Vec3(box_size, 0, 0);
				rotation = Quat::sIdentity();
			}
			else
			{
				position += Vec3(box_size + abs(displacement(random)), displacement(random), displacement(random));
				rotation = Quat::sRandom(random);
			}

			Body &segment = *mBodyInterface->CreateBody(BodyCreationSettings(box, position, rotation, EMotionType::Dynamic, Layers::MOVING));
			segment.SetCollisionGroup(CollisionGroup(group_filter, group_id, CollisionGroup::SubGroupID(i)));
			mBodyInterface->AddBody(segment.GetID(), EActivation::Activate);

			FixedConstraintSettings settings;
			settings.SetPoint(*prev, segment);
			Ref<Constraint> c = settings.Create(*prev, segment);
			mPhysicsSystem->AddConstraint(c);
					
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

	FixedConstraintSettings light1_heavy;
	light1_heavy.SetPoint(*light1, *heavy);
	mPhysicsSystem->AddConstraint(light1_heavy.Create(*light1, *heavy));

	FixedConstraintSettings heavy_light2;
	heavy_light2.SetPoint(*heavy, *light2);
	mPhysicsSystem->AddConstraint(heavy_light2.Create(*heavy, *light2));
}
