// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/SoftBody/SoftBodyForceTest.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/SoftBody/SoftBodyCreationSettings.h>
#include <Utils/SoftBodyCreator.h>
#include <Renderer/DebugRendererImp.h>
#include <External/Perlin.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(SoftBodyForceTest)
{
	JPH_ADD_BASE_CLASS(SoftBodyForceTest, Test)
}

void SoftBodyForceTest::Initialize()
{
	CreateFloor();

	static constexpr uint cGridSize = 30;

	// Create hanging cloth
	auto inv_mass = [](uint inX, uint inZ) {
		return (inX == 0 && inZ == 0)
			|| (inX == cGridSize - 1 && inZ == 0)? 0.0f : 1.0f;
	};
	SoftBodyCreationSettings cloth(SoftBodyCreator::CreateCloth(cGridSize, cGridSize, 0.75f, inv_mass), RVec3(0, 15.0f, 0), Quat::sRotation(Vec3::sAxisX(), 0.5f * JPH_PI), Layers::MOVING);
	mBodyID = mBodyInterface->CreateAndAddSoftBody(cloth, EActivation::Activate);
}

void SoftBodyForceTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	mTime += inParams.mDeltaTime;

	// Apply a fluctuating force
	constexpr float cMaxForce = 10000.0f;
	constexpr float cMaxAngle = DegreesToRadians(90.0f);
	Vec3 force(0, 0, 0.5f * cMaxForce * (1.0f + PerlinNoise3(0, 0, mTime / 2.0f, 256, 256, 256)));
	force = Mat44::sRotationY(cMaxAngle * PerlinNoise3(mTime / 10.0f, 0, 0, 256, 256, 256)) * force;
	mBodyInterface->AddForce(mBodyID, force);

	// Draw the force
	RVec3 offset(0, 10, 0);
	DebugRenderer::sInstance->DrawArrow(offset, offset + 10.0f * force.Normalized(), Color::sGreen, 0.1f);
}

void SoftBodyForceTest::SaveState(StateRecorder &inStream) const
{
	inStream.Write(mTime);
}

void SoftBodyForceTest::RestoreState(StateRecorder &inStream)
{
	inStream.Read(mTime);
}
