// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Constraints/DistanceConstraintTest.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Constraints/DistanceConstraint.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(DistanceConstraintTest) 
{ 
	JPH_ADD_BASE_CLASS(DistanceConstraintTest, Test) 
}

void DistanceConstraintTest::Initialize()
{
	// Floor
	CreateFloor();
		
	float half_cylinder_height = 2.5f;

	// Variation 0: Fixed distance
	// Variation 1: Min/max distance
	for (int variation = 0; variation < 2; ++variation)
	{
		// Bodies attached through distance constraints
		Quat rotation = Quat::sRotation(Vec3::sAxisZ(), 0.5f * JPH_PI);
		Vec3 position(0, 75, 10.0f * variation);
		Body &top = *mBodyInterface->CreateBody(BodyCreationSettings(new CapsuleShape(half_cylinder_height, 1), position, rotation, EMotionType::Static, Layers::NON_MOVING));
		mBodyInterface->AddBody(top.GetID(), EActivation::DontActivate);

		Body *prev = &top;
		for (int i = 1; i < 15; ++i)
		{
			position += Vec3(5.0f + 2.0f * half_cylinder_height, 0, 0);

			Body &segment = *mBodyInterface->CreateBody(BodyCreationSettings(new CapsuleShape(half_cylinder_height, 1), position, rotation, EMotionType::Dynamic, Layers::MOVING));
			mBodyInterface->AddBody(segment.GetID(), EActivation::Activate);

			DistanceConstraintSettings settings;
			settings.mPoint1 = position - Vec3(5.0f + half_cylinder_height, 0, 0);
			settings.mPoint2 = position - Vec3(half_cylinder_height, 0, 0);

			if (variation == 1)
			{
				// Default distance is 5, override min/max range
				settings.mMinDistance = 4.0f;
				settings.mMaxDistance = 8.0f;
			}

			mPhysicsSystem->AddConstraint(settings.Create(*prev, segment));

			prev = &segment;
		}
	}
}
