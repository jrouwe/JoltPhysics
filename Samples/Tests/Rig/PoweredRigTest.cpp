// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Rig/PoweredRigTest.h>
#include <Jolt/Physics/StateRecorder.h>
#include <Jolt/ObjectStream/ObjectStreamIn.h>
#include <Application/DebugUI.h>
#include <Utils/RagdollLoader.h>
#include <Utils/Log.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(PoweredRigTest)
{
	JPH_ADD_BASE_CLASS(PoweredRigTest, Test)
}

const char *PoweredRigTest::sAnimations[] =
{
	"Neutral",
	"Walk",
	"Sprint",
	"Dead_Pose1",
	"Dead_Pose2",
	"Dead_Pose3",
	"Dead_Pose4"
};

const char *PoweredRigTest::sAnimationName = "Sprint";

PoweredRigTest::~PoweredRigTest()
{
	mRagdoll->RemoveFromPhysicsSystem();
}

void PoweredRigTest::Initialize()
{
	// Floor
	CreateFloor();

	// Load ragdoll
	mRagdollSettings = RagdollLoader::sLoad("Assets/Human.tof", EMotionType::Dynamic);

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

void PoweredRigTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	// Update time
	mTime += inParams.mDeltaTime;

	// Sample new pose
	mAnimation->Sample(mTime, mPose);

	// Place the root joint on the first body so that we draw the pose in the right place
	RVec3 root_offset;
	SkeletonPose::JointState &joint = mPose.GetJoint(0);
	joint.mTranslation = Vec3::sZero(); // All the translation goes into the root offset
	mRagdoll->GetRootTransform(root_offset, joint.mRotation);
	mPose.SetRootOffset(root_offset);
	mPose.CalculateJointMatrices();
#ifdef JPH_DEBUG_RENDERER
	mPose.Draw(*inParams.mPoseDrawSettings, mDebugRenderer);
#endif // JPH_DEBUG_RENDERER

	mRagdoll->DriveToPoseUsingMotors(mPose);
}

void PoweredRigTest::CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu)
{
	inUI->CreateTextButton(inSubMenu, "Select Animation", [this, inUI]() {
		UIElement *animation_name = inUI->CreateMenu();
		for (uint i = 0; i < size(sAnimations); ++i)
			inUI->CreateTextButton(animation_name, sAnimations[i], [this, i]() { sAnimationName = sAnimations[i]; RestartTest(); });
		inUI->ShowMenu(animation_name);
	});
}

void PoweredRigTest::SaveState(StateRecorder &inStream) const
{
	inStream.Write(mTime);
}

void PoweredRigTest::RestoreState(StateRecorder &inStream)
{
	inStream.Read(mTime);
}
