// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Samples.h>

#include <Tests/Rig/PoweredRigTest.h>
#include <Jolt/Physics/StateRecorder.h>
#include <Jolt/ObjectStream/ObjectStreamIn.h>
#include <Application/DebugUI.h>
#include <Utils/RagdollLoader.h>
#include <Utils/Log.h>
#include <Utils/AssetStream.h>
#include <Renderer/DebugRendererImp.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(PoweredRigTest)
{
	JPH_ADD_BASE_CLASS(PoweredRigTest, Test)
}

const char *PoweredRigTest::sAnimations[] =
{
	"neutral",
	"walk",
	"sprint",
	"dead_pose1",
	"dead_pose2",
	"dead_pose3",
	"dead_pose4"
};

const char *PoweredRigTest::sAnimationName = "sprint";

PoweredRigTest::~PoweredRigTest()
{
	mRagdoll->RemoveFromPhysicsSystem();
}

void PoweredRigTest::Initialize()
{
	// Floor
	CreateFloor();

	// Load ragdoll
	mRagdollSettings = RagdollLoader::sLoad("Human.tof", EMotionType::Dynamic);

	// Create ragdoll
	mRagdoll = mRagdollSettings->CreateRagdoll(0, 0, mPhysicsSystem);
	mRagdoll->AddToPhysicsSystem(EActivation::Activate);

	// Load animation
	AssetStream stream(String("Human/") + sAnimationName + ".tof", std::ios::in);
	if (!ObjectStreamIn::sReadObject(stream.Get(), mAnimation))
		FatalError("Could not open animation");

	// Initialize pose
	mPose.SetSkeleton(mRagdollSettings->GetSkeleton());
	mPrevPose.SetSkeleton(mRagdollSettings->GetSkeleton());

	// Position ragdoll
	mAnimation->Sample(0.0f, mPose);
	mPose.CalculateJointMatrices();
	mRagdoll->SetPose(mPose);
}

void PoweredRigTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	float prev_time = mTime;

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

	// Sample previous pose (you can also store this, but since we can scrub back in time we need to resample)
	mAnimation->Sample(prev_time, mPrevPose);
	mPrevPose.GetJoint(0) = joint;
	mPrevPose.SetRootOffset(root_offset);
	mPrevPose.CalculateJointMatrices();

	// Measure max error to previous target pose (we drove to that last frame so that's what we should have reached now)
	double avg_error = 0.0, max_error = 0.0;
	for (int j = 0; j < mPose.GetSkeleton()->GetJointCount(); ++j)
	{
		int constraint_index = mRagdollSettings->GetConstraintIndexForBodyIndex(j);
		if (constraint_index < 0)
			continue;
		TwoBodyConstraint *constraint = mRagdoll->GetConstraint(constraint_index);

		RMat44 target = RMat44::sTranslation(root_offset) * mPrevPose.GetJointMatrix(j);
		RMat44 actual = constraint->GetBody2()->GetCenterOfMassTransform() * constraint->GetConstraintToBody2Matrix();
		double error = (double)(target.GetTranslation() - actual.GetTranslation()).Length();
		max_error = max(max_error, error);
		avg_error += error;
	}
	avg_error /= mPose.GetSkeleton()->GetJointCount();
	mDebugRenderer->DrawText3D(root_offset, StringFormat("AvgErr: %.3g, MaxErr: %.3g", avg_error, max_error), Color::sWhite, 0.1f);

	if (sMotorMode == 0)
		mRagdoll->DriveToPoseUsingMotors(mPose);
	else
		mRagdoll->DriveToPoseUsingMotors(mPrevPose, mPose, inParams.mDeltaTime);
}

void PoweredRigTest::CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu)
{
	inUI->CreateTextButton(inSubMenu, "Select Animation", [this, inUI]() {
		UIElement *animation_name = inUI->CreateMenu();
		for (uint i = 0; i < size(sAnimations); ++i)
			inUI->CreateTextButton(animation_name, sAnimations[i], [this, i]() { sAnimationName = sAnimations[i]; RestartTest(); });
		inUI->ShowMenu(animation_name);
	});

	inUI->CreateComboBox(inSubMenu, "Motor", { "Position", "PositionAndVelocity" }, (int)sMotorMode, [](int inItem) { sMotorMode = inItem; });
}

void PoweredRigTest::SaveState(StateRecorder &inStream) const
{
	inStream.Write(mTime);
}

void PoweredRigTest::RestoreState(StateRecorder &inStream)
{
	inStream.Read(mTime);
}
