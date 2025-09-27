// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Rig/SoftKeyframedRigTest.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/StateRecorder.h>
#include <Jolt/ObjectStream/ObjectStreamIn.h>
#include <Application/DebugUI.h>
#include <Layers.h>
#include <Utils/Log.h>
#include <Utils/AssetStream.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(SoftKeyframedRigTest)
{
	JPH_ADD_BASE_CLASS(SoftKeyframedRigTest, Test)
}

SoftKeyframedRigTest::~SoftKeyframedRigTest()
{
	mRagdoll->RemoveFromPhysicsSystem();
}

void SoftKeyframedRigTest::Initialize()
{
	// Floor
	CreateFloor();

	// Wall
	RefConst<Shape> box_shape = new BoxShape(Vec3(0.2f, 0.2f, 0.2f), 0.01f);
	for (int i = 0; i < 3; ++i)
		for (int j = i / 2; j < 10 - (i + 1) / 2; ++j)
		{
			RVec3 position(-2.0f + j * 0.4f + (i & 1? 0.2f : 0.0f), 0.2f + i * 0.4f, -2.0f);
			mBodyInterface->CreateAndAddBody(BodyCreationSettings(box_shape, position, Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING), EActivation::DontActivate);
		}

	// Bar to hit head against
	mBodyInterface->CreateAndAddBody(BodyCreationSettings(new BoxShape(Vec3(2.0f, 0.1f, 0.1f), 0.01f), RVec3(0, 1.5f, -2.0f), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);

	// Load ragdoll
	mRagdollSettings = RagdollLoader::sLoad("Human.tof", EMotionType::Dynamic);

	// Limit max velocity of the bodies to avoid excessive jittering when the head hits the bar
	// Note that this also limits how fast an animation can be and as a result you can see
	// the ragdolls lag behind when the animation loops.
	// Note that the velocity doesn't need to be limited at body level, it can also be done
	// by calculating the needed velocities and clamping them instead of calling DriveToPoseUsingKinematics.
	for (BodyCreationSettings &bcs : mRagdollSettings->mParts)
		bcs.mMaxLinearVelocity = 10.0f;

	// Create ragdoll
	mRagdoll = mRagdollSettings->CreateRagdoll(0, 0, mPhysicsSystem);
	mRagdoll->AddToPhysicsSystem(EActivation::Activate);

	// Load animation
	AssetStream stream("Human/walk.tof", std::ios::in);
	if (!ObjectStreamIn::sReadObject(stream.Get(), mAnimation))
		FatalError("Could not open animation");

	// Initialize pose
	mPose.SetSkeleton(mRagdollSettings->GetSkeleton());

	// Position ragdoll
	mAnimation->Sample(0.0f, mPose);
	mPose.CalculateJointMatrices();
	mRagdoll->SetPose(mPose);
}

void SoftKeyframedRigTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	// Sample previous pose and draw it (ragdoll should have achieved this position)
	mAnimation->Sample(mTime, mPose);
	mPose.CalculateJointMatrices();
#ifdef JPH_DEBUG_RENDERER
	mPose.Draw(*inParams.mPoseDrawSettings, mDebugRenderer);
#endif // JPH_DEBUG_RENDERER

	// Update time
	mTime += inParams.mDeltaTime;

	// Sample new pose
	mAnimation->Sample(mTime, mPose);
	mPose.CalculateJointMatrices();

	// Drive the ragdoll by setting velocities
	mRagdoll->DriveToPoseUsingKinematics(mPose, inParams.mDeltaTime);

	// Cancel gravity that will be applied in the next step
	mRagdoll->AddLinearVelocity(mPhysicsSystem->GetGravity() * inParams.mDeltaTime);
}

void SoftKeyframedRigTest::SaveState(StateRecorder &inStream) const
{
	inStream.Write(mTime);
}

void SoftKeyframedRigTest::RestoreState(StateRecorder &inStream)
{
	inStream.Read(mTime);
}
