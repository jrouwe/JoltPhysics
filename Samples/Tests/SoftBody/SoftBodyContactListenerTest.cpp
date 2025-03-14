// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/SoftBody/SoftBodyContactListenerTest.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/SoftBody/SoftBodyCreationSettings.h>
#include <Jolt/Physics/SoftBody/SoftBodyManifold.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Utils/SoftBodyCreator.h>
#include <Renderer/DebugRendererImp.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(SoftBodyContactListenerTest)
{
	JPH_ADD_BASE_CLASS(SoftBodyContactListenerTest, Test)
}

void SoftBodyContactListenerTest::Initialize()
{
	// Install contact listener for soft bodies
	mPhysicsSystem->SetSoftBodyContactListener(this);

	// Floor
	CreateFloor();

	// Start the 1st cycle
	StartCycle();
}

void SoftBodyContactListenerTest::UpdateLabel()
{
	// Draw current state
	const char *cycle_names[] = { "Accept contact", "Sphere 10x mass", "Cloth 10x mass", "Sphere infinite mass", "Cloth infinite mass", "Sensor contact", "Reject contact", "Kinematic Sphere", "Kinematic Sphere, cloth infinite mass", "Kinematic sphere, sensor contact", "Kinematic Sphere, reject contact" };
	SetBodyLabel(mOtherBodyID, cycle_names[mCycle]);
}

void SoftBodyContactListenerTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	mTime += inParams.mDeltaTime;
	if (mTime > 2.5f)
	{
		// Next cycle
		mCycle = (mCycle + 1) % 10;
		mTime = 0.0f;

		// Remove the old scene
		mBodyInterface->RemoveBody(mOtherBodyID);
		mBodyInterface->DestroyBody(mOtherBodyID);
		mBodyInterface->RemoveBody(mSoftBodyID);
		mBodyInterface->DestroyBody(mSoftBodyID);

		// Start the new
		StartCycle();
	}

	UpdateLabel();
}

void SoftBodyContactListenerTest::StartCycle()
{
	// Create the cloth
	Ref<SoftBodySharedSettings> cloth_settings = SoftBodyCreator::CreateClothWithFixatedCorners(15, 15, 0.75f);

	// Create cloth that's fixated at the corners
	SoftBodyCreationSettings cloth(cloth_settings, RVec3(0, 5, 0), Quat::sRotation(Vec3::sAxisY(), 0.25f * JPH_PI), Layers::MOVING);
	cloth.mUpdatePosition = false; // Don't update the position of the cloth as it is fixed to the world
	cloth.mMakeRotationIdentity = false; // Test explicitly checks if soft bodies with a rotation collide with shapes properly
	mSoftBodyID = mBodyInterface->CreateAndAddSoftBody(cloth, EActivation::Activate);

	// If we want a kinematic sphere
	bool kinematic = mCycle > 6;

	// Create sphere
	BodyCreationSettings bcs(new SphereShape(1.0f), RVec3(0, 7, 0), Quat::sIdentity(), kinematic? EMotionType::Kinematic : EMotionType::Dynamic, Layers::MOVING);
	bcs.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
	bcs.mMassPropertiesOverride.mMass = 100.0f;
	if (kinematic)
		bcs.mLinearVelocity = Vec3(0, -2.5f, 0);
	mOtherBodyID = mBodyInterface->CreateAndAddBody(bcs, EActivation::Activate);

	UpdateLabel();
}

SoftBodyValidateResult SoftBodyContactListenerTest::OnSoftBodyContactValidate(const Body &inSoftBody, const Body &inOtherBody, SoftBodyContactSettings &ioSettings)
{
	switch (mCycle)
	{
	case 0:
		// Normal
		return SoftBodyValidateResult::AcceptContact;

	case 1:
		// Makes the sphere 10x as heavy
		ioSettings.mInvMassScale2 = 0.1f;
		ioSettings.mInvInertiaScale2 = 0.1f;
		return SoftBodyValidateResult::AcceptContact;

	case 2:
		// Makes the cloth 10x as heavy
		ioSettings.mInvMassScale1 = 0.1f;
		return SoftBodyValidateResult::AcceptContact;

	case 3:
		// Makes the sphere have infinite mass
		ioSettings.mInvMassScale2 = 0.0f;
		ioSettings.mInvInertiaScale2 = 0.0f;
		return SoftBodyValidateResult::AcceptContact;

	case 4:
		// Makes the cloth have infinite mass
		ioSettings.mInvMassScale1 = 0.0f;
		return SoftBodyValidateResult::AcceptContact;

	case 5:
		// Sensor contact
		ioSettings.mIsSensor = true;
		return SoftBodyValidateResult::AcceptContact;

	case 6:
		// No contacts
		return SoftBodyValidateResult::RejectContact;

	case 7:
		// Kinematic sphere
		return SoftBodyValidateResult::AcceptContact;

	case 8:
		// Kinematic sphere, cloth infinite mass
		ioSettings.mInvMassScale1 = 0.0f;
		return SoftBodyValidateResult::AcceptContact;

	case 9:
		// Kinematic sphere, sensor contact
		ioSettings.mIsSensor = true;
		return SoftBodyValidateResult::AcceptContact;

	default:
		// No contacts
		return SoftBodyValidateResult::RejectContact;
	}
}

void SoftBodyContactListenerTest::OnSoftBodyContactAdded(const Body &inSoftBody, const SoftBodyManifold &inManifold)
{
	// Draw contacts
	RMat44 com = inSoftBody.GetCenterOfMassTransform();
	for (const SoftBodyVertex &vertex : inManifold.GetVertices())
		if (inManifold.HasContact(vertex))
		{
			RVec3 position = com * inManifold.GetLocalContactPoint(vertex);
			Vec3 normal = inManifold.GetContactNormal(vertex);
			mDebugRenderer->DrawMarker(position, Color::sRed, 0.1f);
			mDebugRenderer->DrawArrow(position, position + normal, Color::sGreen, 0.1f);
		}
}
