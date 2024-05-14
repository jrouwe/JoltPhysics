// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Rig/KinematicRigTest.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/StateRecorder.h>
#include <Jolt/ObjectStream/ObjectStreamIn.h>
#include <Application/DebugUI.h>
#include <Layers.h>
#include <Utils/Log.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(KinematicRigTest)
{
	JPH_ADD_BASE_CLASS(KinematicRigTest, Test)
}

const char *KinematicRigTest::sAnimations[] =
{
	"Neutral",
	"Walk",
	"Sprint",
	"Dead_Pose1",
	"Dead_Pose2",
	"Dead_Pose3",
	"Dead_Pose4"
};

const char *KinematicRigTest::sAnimationName = "Walk";

KinematicRigTest::~KinematicRigTest()
{
	mRagdoll->RemoveFromPhysicsSystem();
}

void KinematicRigTest::Initialize()
{
	// Floor
	CreateFloor();

	// Wall
	RefConst<Shape> box_shape = new BoxShape(Vec3(0.2f, 0.2f, 0.2f), 0.01f);
	for (int i = 0; i < 3; ++i)
		for (int j = i / 2; j < 10 - (i + 1) / 2; ++j)
		{
			RVec3 position(-2.0f + j * 0.4f + (i & 1? 0.2f : 0.0f), 0.2f + i * 0.4f, -2.0f);
			Body &wall = *mBodyInterface->CreateBody(BodyCreationSettings(box_shape, position, Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
			mBodyInterface->AddBody(wall.GetID(), EActivation::DontActivate);
		}

	// Load ragdoll
	mRagdollSettings = RagdollLoader::sLoad("Assets/Human.tof", EMotionType::Kinematic);

	// Create ragdoll
	mRagdoll = mRagdollSettings->CreateRagdoll(0, 0, mPhysicsSystem);
	mRagdoll->AddToPhysicsSystem(EActivation::Activate);

	// Load animation
	String filename = String("Assets/Human/") + sAnimationName + ".tof";
	if (!ObjectStreamIn::sReadObject(filename.c_str(), mAnimation))
		FatalError("Could not open animation");

	// Initialize pose
	mPose.SetSkeleton(mRagdollSettings->GetSkeleton());

	// Position ragdoll
	mAnimation->Sample(0.0f, mPose);
	mPose.CalculateJointMatrices();
	mRagdoll->SetPose(mPose);
}

void KinematicRigTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
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

	mRagdoll->DriveToPoseUsingKinematics(mPose, inParams.mDeltaTime);
}

void KinematicRigTest::CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu)
{
	inUI->CreateTextButton(inSubMenu, "Select Animation", [this, inUI]() {
		UIElement *animation_name = inUI->CreateMenu();
		for (uint i = 0; i < size(sAnimations); ++i)
			inUI->CreateTextButton(animation_name, sAnimations[i], [this, i]() { sAnimationName = sAnimations[i]; RestartTest(); });
		inUI->ShowMenu(animation_name);
	});
}

void KinematicRigTest::SaveState(StateRecorder &inStream) const
{
	inStream.Write(mTime);
}

void KinematicRigTest::RestoreState(StateRecorder &inStream)
{
	inStream.Read(mTime);
}
