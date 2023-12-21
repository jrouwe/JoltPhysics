// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/General/ModifyMassTest.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Renderer/DebugRendererImp.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(ModifyMassTest)
{
	JPH_ADD_BASE_CLASS(ModifyMassTest, Test)
}

void ModifyMassTest::ResetBodies(int inCycle)
{
	mBodyInterface->SetPositionAndRotation(mBodies[0], RVec3(-5, 5, 0), Quat::sIdentity(), EActivation::Activate);
	mBodyInterface->SetLinearAndAngularVelocity(mBodies[0], Vec3(10, 0, 0), Vec3::sZero());
	mBodyInterface->SetUserData(mBodies[0], inCycle << 1);

	mBodyInterface->SetPositionAndRotation(mBodies[1], RVec3(5, 5, 0), Quat::sIdentity(), EActivation::Activate);
	mBodyInterface->SetLinearAndAngularVelocity(mBodies[1], Vec3(-10, 0, 0), Vec3::sZero());
	mBodyInterface->SetUserData(mBodies[1], (inCycle << 1) + 1);
}

void ModifyMassTest::Initialize()
{
	// Floor
	CreateFloor();

	// Create two spheres on a collision course
	BodyCreationSettings bcs(new SphereShape(1.0f), RVec3::sZero(), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
	bcs.mRestitution = 1.0f;
	mBodies[0] = mBodyInterface->CreateAndAddBody(bcs, EActivation::Activate);
	mBodies[1] = mBodyInterface->CreateAndAddBody(bcs, EActivation::Activate);
	ResetBodies(0);
}

void ModifyMassTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	constexpr float cTimeBetweenTests = 2.0f;

	int old_cycle = (int)(mTime / cTimeBetweenTests);
	mTime += inParams.mDeltaTime;
	int new_cycle = (int)(mTime / cTimeBetweenTests);
	if (old_cycle != new_cycle)
		ResetBodies(new_cycle);
}

void ModifyMassTest::PostPhysicsUpdate(float inDeltaTime)
{
	// Draw the mass scale
	for (BodyID id : mBodies)
	{
		BodyLockRead body_lock(mPhysicsSystem->GetBodyLockInterface(), id);
		if (body_lock.Succeeded())
		{
			const Body &body = body_lock.GetBody();
			DebugRenderer::sInstance->DrawText3D(body.GetPosition(), StringFormat("Inv mass scale: %.1f\nVelocity X: %.1f", (double)sGetInvMassScale(body), (double)body.GetLinearVelocity().GetX()), Color::sWhite);
		}
	}
}

float ModifyMassTest::sGetInvMassScale(const Body &inBody)
{
	uint64 ud = inBody.GetUserData();
	int index = ((ud & 1) != 0? (ud >> 1) : (ud >> 3)) & 0b11;
	float mass_overrides[] = { 1.0f, 0.0f, 0.5f, 2.0f };
	return mass_overrides[index];
}

void ModifyMassTest::OnContactAdded(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings)
{
	// We're only concerned with dynamic bodies (floor gets normal collision response)
	if (!inBody1.IsDynamic() || !inBody2.IsDynamic())
		return;

	// Override the mass of body 1
	float scale1 = sGetInvMassScale(inBody1);
	ioSettings.mInvMassScale1 = scale1;
	ioSettings.mInvInertiaScale1 = scale1;

	// Override the mass of body 2
	float scale2 = sGetInvMassScale(inBody2);
	ioSettings.mInvMassScale2 = scale2;
	ioSettings.mInvInertiaScale2 = scale2;
}

void ModifyMassTest::OnContactPersisted(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings)
{
	OnContactAdded(inBody1, inBody2, inManifold, ioSettings);
}

void ModifyMassTest::SaveState(StateRecorder &inStream) const
{
	Test::SaveState(inStream);

	inStream.Write(mTime);
}

void ModifyMassTest::RestoreState(StateRecorder &inStream)
{
	Test::RestoreState(inStream);

	inStream.Read(mTime);
}

