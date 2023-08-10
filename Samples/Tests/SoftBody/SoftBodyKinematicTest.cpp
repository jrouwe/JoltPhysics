// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/SoftBody/SoftBodyKinematicTest.h>
#include <Jolt/Physics/SoftBody/SoftBodyCreationSettings.h>
#include <Jolt/Physics/SoftBody/SoftBodyMotionProperties.h>
#include <Utils/SoftBodyCreator.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(SoftBodyKinematicTest)
{
	JPH_ADD_BASE_CLASS(SoftBodyKinematicTest, Test)
}

void SoftBodyKinematicTest::Initialize()
{
	// Floor
	CreateFloor();

	// A sphere
	Ref<SoftBodySharedSettings> sphere_settings = SoftBodyCreator::CreateSphere();
	sphere_settings->mVertices[0].mInvMass = 0.0f;
	sphere_settings->mVertices[0].mVelocity = Float3(0, 0, 5);
	SoftBodyCreationSettings sphere(sphere_settings, RVec3(0, 5, 0), Quat::sIdentity(), Layers::MOVING);
	sphere.mPressure = 2000.0f;
	mSphereID = mBodyInterface->CreateAndAddSoftBody(sphere, EActivation::Activate);
}

void SoftBodyKinematicTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	// Update the velocity of the first vertex
	BodyLockWrite body_lock(mPhysicsSystem->GetBodyLockInterface(), mSphereID);
	if (body_lock.Succeeded())
	{
		Body &body = body_lock.GetBody();
		SoftBodyMotionProperties *mp = static_cast<SoftBodyMotionProperties *>(body.GetMotionProperties());
		RVec3 com = body.GetCenterOfMassPosition();
		if (com.GetZ() >= 10.0f)
			mp->GetVertex(0).mVelocity = Vec3(0, 0, -5);
		else if (com.GetZ() <= -10.0f)
			mp->GetVertex(0).mVelocity = Vec3(0, 0, 5);
	}
}
