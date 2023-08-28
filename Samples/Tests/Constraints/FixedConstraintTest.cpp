// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
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

		RVec3 position(0, 25.0f, -randomness * 20.0f);
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
			settings.mAutoDetectPoint = true;
			Ref<Constraint> c = settings.Create(*prev, segment);
			mPhysicsSystem->AddConstraint(c);

			prev = &segment;
		}
	}

	{
		// Two light bodies attached to a heavy body (gives issues if the wrong anchor point is chosen)
		Body *light1 = mBodyInterface->CreateBody(BodyCreationSettings(new BoxShape(Vec3::sReplicate(0.1f)), RVec3(-5.0f, 7.0f, -5.2f), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
		mBodyInterface->AddBody(light1->GetID(), EActivation::Activate);
		Body *heavy = mBodyInterface->CreateBody(BodyCreationSettings(new BoxShape(Vec3::sReplicate(5.0f)), RVec3(-5.0f, 7.0f, 0.0f), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
		mBodyInterface->AddBody(heavy->GetID(), EActivation::Activate);
		Body *light2 = mBodyInterface->CreateBody(BodyCreationSettings(new BoxShape(Vec3::sReplicate(0.1f)), RVec3(-5.0f, 7.0f, 5.2f), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
		mBodyInterface->AddBody(light2->GetID(), EActivation::Activate);

		FixedConstraintSettings light1_heavy;
		light1_heavy.mAutoDetectPoint = true;
		mPhysicsSystem->AddConstraint(light1_heavy.Create(*light1, *heavy));

		FixedConstraintSettings heavy_light2;
		heavy_light2.mAutoDetectPoint = true;
		mPhysicsSystem->AddConstraint(heavy_light2.Create(*heavy, *light2));
	}

	{
		// A tower of beams and crossbeams (note that it is not recommended to make constructs with this many fixed constraints, this is not always stable)
		RVec3 base_position(0, 25, -40.0f);
		Quat base_rotation = Quat::sRotation(Vec3::sAxisZ(), -0.5f * JPH_PI);

		Ref<BoxShape> pillar = new BoxShape(Vec3(0.1f, 1.0f, 0.1f), 0.0f);
		Ref<BoxShape> beam = new BoxShape(Vec3(0.01f, 1.5f, 0.1f), 0.0f);

		Body *prev_pillars[4] = { &Body::sFixedToWorld, &Body::sFixedToWorld, &Body::sFixedToWorld, &Body::sFixedToWorld };

		Vec3 center = Vec3::sZero();
		for (int y = 0; y < 10; ++y)
		{
			// Create pillars
			Body *pillars[4];
			for (int i = 0; i < 4; ++i)
			{
				Quat rotation = Quat::sRotation(Vec3::sAxisY(), i * 0.5f * JPH_PI);

				pillars[i] = mBodyInterface->CreateBody(BodyCreationSettings(pillar, base_position + base_rotation * (center + rotation * Vec3(1.0f, 1.0f, 1.0f)), base_rotation, EMotionType::Dynamic, Layers::MOVING));
				pillars[i]->SetCollisionGroup(CollisionGroup(group_filter, 0, 0)); // For convenience, we disable collisions between all objects in the tower
				mBodyInterface->AddBody(pillars[i]->GetID(), EActivation::Activate);
			}

			for (int i = 0; i < 4; ++i)
			{
				Quat rotation = Quat::sRotation(Vec3::sAxisY(), i * 0.5f * JPH_PI);

				// Create cross beam
				Body *cross = mBodyInterface->CreateBody(BodyCreationSettings(beam, base_position + base_rotation * (center + rotation * Vec3(1.105f, 1.0f, 0.0f)), base_rotation * rotation * Quat::sRotation(Vec3::sAxisX(), 0.3f * JPH_PI), EMotionType::Dynamic, Layers::MOVING));
				cross->SetCollisionGroup(CollisionGroup(group_filter, 0, 0));
				mBodyInterface->AddBody(cross->GetID(), EActivation::Activate);

				// Attach cross beam to pillars
				for (int j = 0; j < 2; ++j)
				{
					FixedConstraintSettings constraint;
					constraint.mAutoDetectPoint = true;
					constraint.mNumVelocityStepsOverride = 64; // This structure needs more solver steps to be stable
					constraint.mNumPositionStepsOverride = JPH_IF_NOT_DEBUG(64) JPH_IF_DEBUG(8); // In debug mode use less steps to preserve framerate (at the cost of stability)
					mPhysicsSystem->AddConstraint(constraint.Create(*pillars[(i + j) % 4], *cross));
				}

				// Attach to previous pillar
				if (prev_pillars[i] != nullptr)
				{
					FixedConstraintSettings constraint;
					constraint.mAutoDetectPoint = true;
					constraint.mNumVelocityStepsOverride = 64;
					constraint.mNumPositionStepsOverride = JPH_IF_NOT_DEBUG(64) JPH_IF_DEBUG(8);
					mPhysicsSystem->AddConstraint(constraint.Create(*prev_pillars[i], *pillars[i]));
				}

				prev_pillars[i] = pillars[i];
			}

			center += Vec3(0.0f, 2.0f, 0.0f);
		}

		// Create top
		Body *top = mBodyInterface->CreateBody(BodyCreationSettings(new BoxShape(Vec3(1.2f, 0.1f, 1.2f)), base_position + base_rotation * (center + Vec3(0.0f, 0.1f, 0.0f)), base_rotation, EMotionType::Dynamic, Layers::MOVING));
		top->SetCollisionGroup(CollisionGroup(group_filter, 0, 0));
		mBodyInterface->AddBody(top->GetID(), EActivation::Activate);

		// Attach top to pillars
		for (int i = 0; i < 4; ++i)
		{
			FixedConstraintSettings constraint;
			constraint.mAutoDetectPoint = true;
			mPhysicsSystem->AddConstraint(constraint.Create(*prev_pillars[i], *top));
		}
	}
}
