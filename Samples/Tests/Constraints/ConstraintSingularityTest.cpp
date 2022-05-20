// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Constraints/ConstraintSingularityTest.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/GroupFilterTable.h>
#include <Jolt/Physics/Constraints/HingeConstraint.h>
#include <Jolt/Physics/Constraints/FixedConstraint.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(ConstraintSingularityTest) 
{ 
	JPH_ADD_BASE_CLASS(ConstraintSingularityTest, Test) 
}

void ConstraintSingularityTest::Initialize()
{
	// Floor
	CreateFloor();
		
	float box_size = 4.0f;
	RefConst<Shape> box = new BoxShape(Vec3::sReplicate(0.5f * box_size));

	const int num_constraint_types = 2;
	const int num_configurations = 4;

	// Create group filter
	Ref<GroupFilterTable> group_filter = new GroupFilterTable;
	CollisionGroup::GroupID group_id = 0;

	for (int constraint_type = 0; constraint_type < num_constraint_types; ++constraint_type)
		for (int configuration = 0; configuration < num_configurations; ++configuration)
		{
			Vec3 test_position(10.0f * constraint_type, 10.0f + 10.0f * configuration, 0);

			Body &body1 = *mBodyInterface->CreateBody(BodyCreationSettings(box, test_position, Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING));
			body1.SetCollisionGroup(CollisionGroup(group_filter, group_id, 0));
			mBodyInterface->AddBody(body1.GetID(), EActivation::DontActivate);

			Body &body2 = *mBodyInterface->CreateBody(BodyCreationSettings(box, test_position + Vec3(box_size, 0, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
			body2.SetCollisionGroup(CollisionGroup(group_filter, group_id, 0));
			mBodyInterface->AddBody(body2.GetID(), EActivation::Activate);

			Constraint *constraint;
			switch (constraint_type)
			{
			case 0:
				{
					HingeConstraintSettings settings;
					settings.mPoint1 = settings.mPoint2 = test_position + Vec3(0.5f * box_size, 0, 0.5f * box_size);
					settings.mHingeAxis1 = settings.mHingeAxis2 = Vec3::sAxisY();
					settings.mNormalAxis1 = settings.mNormalAxis2 = Vec3::sAxisX();
					settings.mLimitsMin = -0.01f;
					settings.mLimitsMax = 0.01f;
					constraint = settings.Create(body1, body2);
					break;
				}

			default:
				{
					FixedConstraintSettings settings;
					settings.SetPoint(body1, body2);
					constraint = settings.Create(body1, body2);
					break;
				}
			}
				
			mPhysicsSystem->AddConstraint(constraint);

			Vec3 position;
			Quat orientation;
			switch (configuration)
			{
			case 0: 
				position = test_position + Vec3(0, 0, box_size);
				orientation = Quat::sRotation(Vec3::sAxisY(), DegreesToRadians(180.0f)); 
				break;

			case 1: 
				position = test_position + Vec3(0, 0, box_size);
				orientation = Quat::sRotation(Vec3::sAxisY(), DegreesToRadians(-90.0f)) * Quat::sRotation(Vec3::sAxisX(), DegreesToRadians(180.0f)); 
				break;

			case 2:
				position = test_position + Vec3(box_size, 0, 0);
				orientation = Quat::sRotation(Vec3::sAxisY(), DegreesToRadians(90.0f)) * Quat::sRotation(Vec3::sAxisZ(), DegreesToRadians(90.0f)); 
				break;

			default:
				JPH_ASSERT(configuration == 3);
				position = test_position + Vec3(-box_size, 0, 0);
				orientation = Quat::sRotation(Vec3::sAxisY(), DegreesToRadians(90.0f)) * Quat::sRotation(Vec3::sAxisZ(), DegreesToRadians(90.0f)); 
				break;
			}

			mBodyInterface->SetPositionAndRotation(body2.GetID(), position, orientation, EActivation::DontActivate);

			++group_id;
		}
}
