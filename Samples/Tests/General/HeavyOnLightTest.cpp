// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/General/HeavyOnLightTest.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(HeavyOnLightTest)
{
	JPH_ADD_BASE_CLASS(HeavyOnLightTest, Test)
}

void HeavyOnLightTest::Initialize()
{
	// Floor
	CreateFloor();

	Ref<BoxShape> box = new BoxShape(Vec3::sReplicate(5));
	box->SetDensity(1000.0f);

	for (int i = 1; i <= 10; ++i)
	{
		BodyID id = mBodyInterface->CreateAndAddBody(BodyCreationSettings(box, RVec3(-75.0f + i * 15.0f, 10.0f, 0.0f), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING), EActivation::Activate);
		SetBodyLabel(id, StringFormat("Mass: %g", double(box->GetMassProperties().mMass)));

		Ref<BoxShape> box2 = new BoxShape(Vec3::sReplicate(5));
		box2->SetDensity(5000.0f * i);

		id = mBodyInterface->CreateAndAddBody(BodyCreationSettings(box2, RVec3(-75.0f + i * 15.0f, 30.0f, 0.0f), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING), EActivation::Activate);
		SetBodyLabel(id, StringFormat("Mass: %g", double(box2->GetMassProperties().mMass)));
	}
}
