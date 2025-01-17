// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/SoftBody/SoftBodyCustomUpdateTest.h>
#include <Jolt/Physics/SoftBody/SoftBodyCreationSettings.h>
#include <Jolt/Physics/SoftBody/SoftBodyMotionProperties.h>
#include <Utils/SoftBodyCreator.h>
#include <Layers.h>
#include <Renderer/DebugRendererImp.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(SoftBodyCustomUpdateTest)
{
	JPH_ADD_BASE_CLASS(SoftBodyCustomUpdateTest, Test)
}

void SoftBodyCustomUpdateTest::Initialize()
{
	// Floor
	CreateFloor();

	// Create a body but do not add it to the physics system (we're updating it ourselves)
	SoftBodyCreationSettings sphere(SoftBodyCreator::CreateSphere(), RVec3(0, 5, 0), Quat::sIdentity(), Layers::MOVING);
	sphere.mPressure = 2000.0f;
	mBody = mBodyInterface->CreateSoftBody(sphere);
}

void SoftBodyCustomUpdateTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	// Note that passing a variable delta time results in differences in behavior, usually you want to have a fixed time step.
	// For this demo we'll just clamp the delta time to 1/60th of a second and allow behavioral changes due to frame rate fluctuations.
	float dt = min(inParams.mDeltaTime, 1.0f / 60.0f);

	// Call the update now
	SoftBodyMotionProperties *mp = static_cast<SoftBodyMotionProperties *>(mBody->GetMotionProperties());
	mp->CustomUpdate(dt, *mBody, *mPhysicsSystem);

#ifdef JPH_DEBUG_RENDERER
	// Draw it as well since it's not added to the world
	mBody->GetShape()->Draw(mDebugRenderer, mBody->GetCenterOfMassTransform(), Vec3::sOne(), Color::sWhite, false, false);
#else
	// Draw the vertices
	RMat44 com = mBody->GetCenterOfMassTransform();
	for (const SoftBodyVertex &v : mp->GetVertices())
		mDebugRenderer->DrawMarker(com * v.mPosition, Color::sRed, 0.1f);
#endif // JPH_DEBUG_RENDERER
}
