// SPDX-FileCopyrightText: 2022 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Constraints/PulleyConstraintTest.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Constraints/PulleyConstraint.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(PulleyConstraintTest) 
{ 
	JPH_ADD_BASE_CLASS(PulleyConstraintTest, Test) 
}

void PulleyConstraintTest::Initialize()
{
	// Floor
	CreateFloor();
		
	// Variation 0: Max length (rope)
	// Variation 1: Fixed length (rigid rod)
	// Variation 2: With ratio (block and tackle)
	for (int variation = 0; variation < 3; ++variation)
	{
		Vec3 position1(-10, 10, 10.0f * variation);
		Body &body1 = *mBodyInterface->CreateBody(BodyCreationSettings(new BoxShape(Vec3::sReplicate(0.5f)), position1, Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
		mBodyInterface->AddBody(body1.GetID(), EActivation::Activate);

		Vec3 position2(10, 10, 10.0f * variation);
		Body &body2 = *mBodyInterface->CreateBody(BodyCreationSettings(new BoxShape(Vec3::sReplicate(0.5f)), position2, Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
		mBodyInterface->AddBody(body2.GetID(), EActivation::Activate);

		PulleyConstraintSettings settings;
		settings.mBodyPoint1 = position1 + Vec3(0, 0.5f, 0); // Connect at the top of the block
		settings.mBodyPoint2 = position2 + Vec3(0, 0.5f, 0);
		settings.mFixedPoint1 = position1 + Vec3(0, 10, 0);
		settings.mFixedPoint2 = position2 + Vec3(0, 10, 0);

		switch (variation)			
		{
		case 1:
			// Fixed size
			settings.mMinLength = settings.mMaxLength = 20.0f;
			break;

		case 2:
			// With ratio
			settings.mRatio = 2.0f;
			break;			
		}

		mPhysicsSystem->AddConstraint(settings.Create(body1, body2));
	}
}
