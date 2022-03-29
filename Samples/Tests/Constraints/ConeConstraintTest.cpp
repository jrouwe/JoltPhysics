// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Constraints/ConeConstraintTest.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/GroupFilterTable.h>
#include <Jolt/Physics/Constraints/ConeConstraint.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(ConeConstraintTest) 
{ 
	JPH_ADD_BASE_CLASS(ConeConstraintTest, Test) 
}

void ConeConstraintTest::Initialize()
{
	// Floor
	CreateFloor();
		
	float half_cylinder_height = 2.5f;

	const int cChainLength = 5;

	// Build a collision group filter that disables collision between adjacent bodies
	Ref<GroupFilterTable> group_filter = new GroupFilterTable(cChainLength);
	for (CollisionGroup::SubGroupID i = 0; i < cChainLength - 1; ++i)
		group_filter->DisableCollision(i, i + 1);

	// Bodies attached through cone constraints
	for (int j = 0; j < 2; ++j)
	{
		Body *prev = nullptr;
		Quat rotation = Quat::sRotation(Vec3::sAxisZ(), 0.5f * JPH_PI);
		Vec3 position(0, 20.0f, 10.0f * j);
		for (int i = 0; i < cChainLength; ++i)
		{
			position += Vec3(2.0f * half_cylinder_height, 0, 0);

			Body &segment = *mBodyInterface->CreateBody(BodyCreationSettings(new CapsuleShape(half_cylinder_height, 1), position, Quat::sRotation(Vec3::sAxisX(), 0.25f * JPH_PI * i) * rotation, i == 0? EMotionType::Static : EMotionType::Dynamic, i == 0? Layers::NON_MOVING : Layers::MOVING));
			segment.SetCollisionGroup(CollisionGroup(group_filter, CollisionGroup::GroupID(j), CollisionGroup::SubGroupID(i)));
			mBodyInterface->AddBody(segment.GetID(), EActivation::Activate);

			if (prev != nullptr)
			{
				ConeConstraintSettings settings;
				settings.mPoint1 = settings.mPoint2 = position + Vec3(-half_cylinder_height, 0, 0);
				settings.mTwistAxis1 = settings.mTwistAxis2 = Vec3(1, 0, 0);
				if (j == 0)
					settings.mHalfConeAngle = 0.0f;
				else
					settings.mHalfConeAngle = DegreesToRadians(20);

				mPhysicsSystem->AddConstraint(settings.Create(*prev, segment));
			}

			prev = &segment;
		}
	}
}
