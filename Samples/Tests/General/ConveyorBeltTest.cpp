// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/General/ConveyorBeltTest.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(ConveyorBeltTest) 
{ 
	JPH_ADD_BASE_CLASS(ConveyorBeltTest, Test) 
}

void ConveyorBeltTest::Initialize()
{
	// Floor
	Body &floor = CreateFloor();
	floor.SetFriction(1.0f);

	const float cBeltWidth = 10.0f;
	const float cBeltLength = 50.0f;

	// Create conveyor belts
	BodyCreationSettings belt_settings(new BoxShape(Vec3(cBeltWidth, 0.1f, cBeltLength)), RVec3::sZero(), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);
	for (int i = 0; i < 4; ++i)
	{
		belt_settings.mFriction = 0.25f * (i + 1);
		belt_settings.mRotation = Quat::sRotation(Vec3::sAxisY(), 0.5f * JPH_PI * i) * Quat::sRotation(Vec3::sAxisX(), 0.005f);
		belt_settings.mPosition = belt_settings.mRotation * RVec3(cBeltLength, 6.0f, cBeltWidth);
		mBelts.push_back(mBodyInterface->CreateAndAddBody(belt_settings, EActivation::DontActivate));
	}

	// Bodies with decreasing friction
	BodyCreationSettings cargo_settings(new BoxShape(Vec3::sReplicate(2.0f)), RVec3::sZero(), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
	for (int i = 0; i <= 10; ++i)
	{
		cargo_settings.mPosition = RVec3(-cBeltLength + i * 10.0f, 10.0f, -cBeltLength);
		cargo_settings.mFriction = 1.0f - 0.1f * i;
		mBodyInterface->CreateAndAddBody(cargo_settings, EActivation::Activate);
	}
}

void ConveyorBeltTest::OnContactAdded(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings)
{
	// Body 1 should always be the belt because we create it before the other bodies so their ID will be lower
	// Note that normally you cannot make this assumption so you need to handle both the case that body 1 is the belt and the case that body 2 is the belt
	if (std::find(mBelts.begin(), mBelts.end(), inBody1.GetID()) != mBelts.end())
	{
		// Get the tangents of the contact manifold
		Vec3 tangent1, tangent2;
		inManifold.GetTangents(tangent1, tangent2);

		// Take the tangents to local space relative to body 1
		Quat world_to_body1 = inBody1.GetRotation().Conjugated();
		tangent1 = world_to_body1 * tangent1;
		tangent2 = world_to_body1 * tangent2;

		// We always move the body along our negative Z axis
		const Vec3 cLocalSpaceVelocity(0, 0, -10.0f);
		ioSettings.mSurfaceVelocity1 = tangent1.Dot(cLocalSpaceVelocity);
		ioSettings.mSurfaceVelocity2 = tangent2.Dot(cLocalSpaceVelocity);
	}
}

void ConveyorBeltTest::OnContactPersisted(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings)
{
	// Same behavior as contact added
	OnContactAdded(inBody1, inBody2, inManifold, ioSettings);
}
