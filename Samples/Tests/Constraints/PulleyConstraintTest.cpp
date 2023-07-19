// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
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
	// Variation 2: Min/max length
	// Variation 3: With ratio (block and tackle)
	for (int variation = 0; variation < 4; ++variation)
	{
		RVec3 position1(-10, 10, -10.0f * variation);
		Body &body1 = *mBodyInterface->CreateBody(BodyCreationSettings(new BoxShape(Vec3::sReplicate(0.5f)), position1, Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
		mBodyInterface->AddBody(body1.GetID(), EActivation::Activate);

		RVec3 position2(10, 10, -10.0f * variation);
		Body &body2 = *mBodyInterface->CreateBody(BodyCreationSettings(new BoxShape(Vec3::sReplicate(0.5f)), position2, Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
		mBodyInterface->AddBody(body2.GetID(), EActivation::Activate);

		PulleyConstraintSettings settings;
		settings.mBodyPoint1 = position1 + Vec3(0, 0.5f, 0); // Connect at the top of the block
		settings.mBodyPoint2 = position2 + Vec3(0, 0.5f, 0);
		settings.mFixedPoint1 = settings.mBodyPoint1 + Vec3(0, 10, 0);
		settings.mFixedPoint2 = settings.mBodyPoint2 + Vec3(0, 10, 0);

		switch (variation)
		{
		case 0:
			// Can't extend but can contract
			break;

		case 1:
			// Fixed size
			settings.mMinLength = settings.mMaxLength = -1;
			break;

		case 2:
			// With range
			settings.mMinLength = 18.0f;
			settings.mMaxLength = 22.0f;
			break;

		case 3:
			// With ratio
			settings.mRatio = 4.0f;
			break;
		}

		mPhysicsSystem->AddConstraint(settings.Create(body1, body2));
	}
}
