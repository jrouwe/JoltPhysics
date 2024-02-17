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

	// Create the cloth
	Ref<SoftBodySharedSettings> cloth_settings = SoftBodyCreator::CreateCloth(15);

	// Create body creation settings
	BodyCreationSettings bcs(new SphereShape(1.0f), RVec3::sZero(), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
	bcs.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
	bcs.mMassPropertiesOverride.mMass = 100.0f;

	// Create variations in mass
	for (int i = 0; i < 6; ++i)
	{
		RVec3 position = RVec3(-45.0f + 15.0f * i, 5.0f, 0.0f);

		// Create cloth that's fixated at the corners
		SoftBodyCreationSettings cloth(cloth_settings, position, Quat::sRotation(Vec3::sAxisY(), 0.25f * JPH_PI), Layers::MOVING);
		cloth.mUpdatePosition = false; // Don't update the position of the cloth as it is fixed to the world
		cloth.mMakeRotationIdentity = false; // Test explicitly checks if soft bodies with a rotation collide with shapes properly
		mBodyInterface->CreateAndAddSoftBody(cloth, EActivation::Activate);

		// Create sphere
		bcs.mPosition = position + Vec3(0, 2.0f, 0);
		mBodies.push_back(mBodyInterface->CreateAndAddBody(bcs, EActivation::Activate));
	}
}

SoftBodyValidateResult SoftBodyContactListenerTest::OnSoftBodyContactValidate(const Body &inSoftBody, const Body &inOtherBody, SoftBodyContactSettings &ioSettings)
{
	BodyID id = inOtherBody.GetID();
	if (id == mBodies[1])
	{
		// 2nd one is normal
		return SoftBodyValidateResult::AcceptContact;
	}
	else if (id == mBodies[2])
	{
		// 3rd one makes the sphere 10x as heavy
		ioSettings.mInvMassScale2 = 0.1f;
		ioSettings.mInvInertiaScale2 = 0.1f;
		return SoftBodyValidateResult::AcceptContact;
	}
	else if (id == mBodies[3])
	{
		// 4th one makes the cloth 10x as heavy
		ioSettings.mInvMassScale1 = 0.1f;
		return SoftBodyValidateResult::AcceptContact;
	}
	else if (id == mBodies[4])
	{
		// 5th one makes the cloth react to the sphere but not vice versa
		ioSettings.mInvMassScale2 = 0.0f;
		ioSettings.mInvInertiaScale2 = 0.0f;
		return SoftBodyValidateResult::AcceptContact;
	}
	else if (id == mBodies[5])
	{
		// 6th one makes the sphere react to the cloth but not vice versa
		ioSettings.mInvMassScale1 = 0.0f;
		return SoftBodyValidateResult::AcceptContact;
	}

	// 1st one falls through
	return SoftBodyValidateResult::RejectContact;
}

void SoftBodyContactListenerTest::OnSoftBodyContactAdded(const Body &inSoftBody, const Body &inOtherBody, const SoftBodyManifold &inManifold)
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
