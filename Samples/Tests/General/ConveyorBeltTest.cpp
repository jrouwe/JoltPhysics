// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/General/ConveyorBeltTest.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/CylinderShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(ConveyorBeltTest) 
{ 
	JPH_ADD_BASE_CLASS(ConveyorBeltTest, Test) 
}

void ConveyorBeltTest::Initialize()
{
	// Floor
	CreateFloor();

	// Create conveyor belts
	const float cBeltWidth = 10.0f;
	const float cBeltLength = 50.0f;
	BodyCreationSettings belt_settings(new BoxShape(Vec3(cBeltWidth, 0.1f, cBeltLength)), RVec3::sZero(), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);
	for (int i = 0; i < 4; ++i)
	{
		belt_settings.mFriction = 0.25f * (i + 1);
		belt_settings.mRotation = Quat::sRotation(Vec3::sAxisY(), 0.5f * JPH_PI * i) * Quat::sRotation(Vec3::sAxisX(), DegreesToRadians(1.0f));
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

	// Create 2 cylinders
	BodyCreationSettings cylinder_settings(new CylinderShape(6.0f, 1.0f), RVec3(0, 1.0f, -20.0f), Quat::sRotation(Vec3::sAxisZ(), 0.5f * JPH_PI), EMotionType::Dynamic, Layers::MOVING);
	mBodyInterface->CreateAndAddBody(cylinder_settings, EActivation::Activate);
	cylinder_settings.mPosition.SetZ(20.0f);
	mBodyInterface->CreateAndAddBody(cylinder_settings, EActivation::Activate);

	// Let a dynamic belt rest on it
	BodyCreationSettings dynamic_belt(new BoxShape(Vec3(5.0f, 0.1f, 25.0f), 0.0f), RVec3(0, 3.0f, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
	mBelts.push_back(mBodyInterface->CreateAndAddBody(dynamic_belt, EActivation::Activate));

	// Create cargo on the dynamic belt
	cargo_settings.mPosition = RVec3(0, 6.0f, 15.0f);
	cargo_settings.mFriction = 1.0f;
	mBodyInterface->CreateAndAddBody(cargo_settings, EActivation::Activate);
}

void ConveyorBeltTest::OnContactAdded(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings)
{
	// Determine the world space surface velocity of both bodies
	const Vec3 cLocalSpaceVelocity(0, 0, -10.0f);
	bool body1_belt = std::find(mBelts.begin(), mBelts.end(), inBody1.GetID()) != mBelts.end();
	Vec3 body1_surface_velocity = body1_belt? inBody1.GetRotation() * cLocalSpaceVelocity : Vec3::sZero();
	bool body2_belt = std::find(mBelts.begin(), mBelts.end(), inBody2.GetID()) != mBelts.end();
	Vec3 body2_surface_velocity = body2_belt? inBody2.GetRotation() * cLocalSpaceVelocity : Vec3::sZero();

	// Calculate the relative surface velocity
	ioSettings.mRelativeSurfaceVelocity = body2_surface_velocity - body1_surface_velocity;
}

void ConveyorBeltTest::OnContactPersisted(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings)
{
	// Same behavior as contact added
	OnContactAdded(inBody1, inBody2, inManifold, ioSettings);
}
