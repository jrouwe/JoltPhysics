// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/General/ActivateDuringUpdateTest.h>
#include <Layers.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(ActivateDuringUpdateTest)
{
	JPH_ADD_BASE_CLASS(ActivateDuringUpdateTest, Test)
}

void ActivateDuringUpdateTest::Initialize()
{
	// Floor
	CreateFloor();

	BodyCreationSettings settings;
	settings.SetShape(new BoxShape(Vec3::sReplicate(0.5f)));
	settings.mMotionType = EMotionType::Dynamic;
	settings.mObjectLayer = Layers::MOVING;

	const int cNumBodies = 3;
	const float cPenetrationSlop = mPhysicsSystem->GetPhysicsSettings().mPenetrationSlop;

	for (int i = 0; i < cNumBodies; ++i)
	{
		settings.mPosition = RVec3(i * (1.0f - cPenetrationSlop), 2.0f, 0);
		BodyID body_id = mBodyInterface->CreateBody(settings)->GetID();
		mBodyInterface->AddBody(body_id, EActivation::DontActivate);
		if (i == 0)
			mBodyInterface->SetLinearVelocity(body_id, Vec3(500, 0, 0));
	}

	for (int i = 0; i < cNumBodies; ++i)
	{
		settings.mPosition = RVec3(i * (1.0f - cPenetrationSlop), 2.0f, 2.0f);
		BodyID body_id = mBodyInterface->CreateBody(settings)->GetID();
		mBodyInterface->AddBody(body_id, EActivation::DontActivate);
		if (i == cNumBodies - 1)
			mBodyInterface->SetLinearVelocity(body_id, Vec3(-500, 0, 0));
	}
}
