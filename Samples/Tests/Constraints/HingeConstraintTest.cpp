// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Constraints/HingeConstraintTest.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/GroupFilterTable.h>
#include <Jolt/Physics/Constraints/HingeConstraint.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
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

	constexpr int cChainLength = 15;
	constexpr float cMinAngle = DegreesToRadians(-10.0f);
	constexpr float cMaxAngle = DegreesToRadians(20.0f);

	// Build a collision group filter that disables collision between adjacent bodies
	Ref<GroupFilterTable> group_filter = new GroupFilterTable(cChainLength);
	for (CollisionGroup::SubGroupID i = 0; i < cChainLength - 1; ++i)
		group_filter->DisableCollision(i, i + 1);

	// Bodies attached through hinge constraints
	for (int randomness = 0; randomness < 2; ++randomness)
	{
		CollisionGroup::GroupID group_id = CollisionGroup::GroupID(randomness);

		Vec3 position(0, 50, -randomness * 20.0f);
		Body &top = *mBodyInterface->CreateBody(BodyCreationSettings(box, position, Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING));
		top.SetCollisionGroup(CollisionGroup(group_filter, group_id, 0));
		mBodyInterface->AddBody(top.GetID(), EActivation::DontActivate);

		default_random_engine random;
		uniform_real_distribution<float> displacement(-1.0f, 1.0f);

		Body *prev = &top;
		for (int i = 1; i < cChainLength; ++i)
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
			settings.mLimitsMin = cMinAngle;
			settings.mLimitsMax = cMaxAngle;
			mPhysicsSystem->AddConstraint(settings.Create(*prev, segment));

			prev = &segment;
		}
	}
}
