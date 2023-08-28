// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Rig/SkeletonMapperTest.h>
#include <Jolt/Physics/StateRecorder.h>
#include <Jolt/ObjectStream/ObjectStreamIn.h>
#include <Renderer/DebugRendererImp.h>
#include <Layers.h>
#include <Utils/Log.h>
#include <Application/DebugUI.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(SkeletonMapperTest)
{
	JPH_ADD_BASE_CLASS(SkeletonMapperTest, Test)
}

SkeletonMapperTest::~SkeletonMapperTest()
{
	mRagdoll->RemoveFromPhysicsSystem();
}

void SkeletonMapperTest::Initialize()
{
	// Floor
	CreateFloor();

	// Load ragdoll
	mRagdollSettings = RagdollLoader::sLoad("Assets/Human.tof", EMotionType::Dynamic);

	// Create ragdoll
	mRagdoll = mRagdollSettings->CreateRagdoll(0, 0, mPhysicsSystem);
	mRagdoll->AddToPhysicsSystem(EActivation::Activate);

	// Load neutral animation for ragdoll
	Ref<SkeletalAnimation> neutral_ragdoll;
	if (!ObjectStreamIn::sReadObject("Assets/Human/neutral.tof", neutral_ragdoll))
		FatalError("Could not open neutral animation");

	// Load animation skeleton
	Ref<Skeleton> animation_skeleton;
	if (!ObjectStreamIn::sReadObject("Assets/Human/skeleton_hd.tof", animation_skeleton))
		FatalError("Could not open skeleton_hd");
	animation_skeleton->CalculateParentJointIndices();

	// Load neutral animation
	Ref<SkeletalAnimation> neutral_animation;
	if (!ObjectStreamIn::sReadObject("Assets/Human/neutral_hd.tof", neutral_animation))
		FatalError("Could not open neutral_hd animation");

	// Load test animation
	if (!ObjectStreamIn::sReadObject("Assets/Human/jog_hd.tof", mAnimation))
		FatalError("Could not open jog_hd animation");

	// Initialize pose
	mAnimatedPose.SetSkeleton(animation_skeleton);
	mRagdollPose.SetSkeleton(mRagdollSettings->GetSkeleton());

	// Calculate neutral poses and initialize skeleton mapper
	neutral_ragdoll->Sample(0.0f, mRagdollPose);
	mRagdollPose.CalculateJointMatrices();
	neutral_animation->Sample(0.0f, mAnimatedPose);
	mAnimatedPose.CalculateJointMatrices();
	mRagdollToAnimated.Initialize(mRagdollPose.GetSkeleton(), mRagdollPose.GetJointMatrices().data(), mAnimatedPose.GetSkeleton(), mAnimatedPose.GetJointMatrices().data());

	// Optionally lock translations (this can be used to prevent ragdolls from stretching)
	// Try wildly dragging the ragdoll by the head (using spacebar) to see how the ragdoll stretches under stress
	if (sLockTranslations)
		mRagdollToAnimated.LockAllTranslations(mAnimatedPose.GetSkeleton(), mAnimatedPose.GetJointMatrices().data());

	// Calculate initial pose and set it
	CalculateRagdollPose();
	mRagdoll->SetPose(mRagdollPose);
}

void SkeletonMapperTest::CalculateRagdollPose()
{
	// Sample new animated pose
	mAnimation->Sample(mTime, mAnimatedPose);
	mAnimatedPose.CalculateJointMatrices();

	// Map to ragdoll pose
	mRagdollToAnimated.MapReverse(mAnimatedPose.GetJointMatrices().data(), mRagdollPose.GetJointMatrices().data());
	mRagdollPose.CalculateJointStates();
}

void SkeletonMapperTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	// Update time
	mTime += inParams.mDeltaTime;

	// Drive the ragdoll pose and drive motors to reach it
	CalculateRagdollPose();
	mRagdoll->DriveToPoseUsingMotors(mRagdollPose);

#ifdef JPH_DEBUG_RENDERER
	// Draw animated skeleton
	mAnimatedPose.Draw(*inParams.mPoseDrawSettings, mDebugRenderer);
	mDebugRenderer->DrawText3D(mAnimatedPose.GetRootOffset() + mAnimatedPose.GetJointMatrix(0).GetTranslation(), "Animated", Color::sWhite, 0.2f);

	// Draw mapped skeleton
	RMat44 offset = RMat44::sTranslation(RVec3(1.0f, 0, 0));
	mRagdollPose.Draw(*inParams.mPoseDrawSettings, mDebugRenderer, offset);
	mDebugRenderer->DrawText3D(offset * (mAnimatedPose.GetRootOffset() + mAnimatedPose.GetJointMatrix(0).GetTranslation()), "Reverse Mapped", Color::sWhite, 0.2f);
#endif // JPH_DEBUG_RENDERER

	// Get ragdoll pose in model space
	RVec3 root_offset;
	Array<Mat44> pose1_model(mRagdollPose.GetJointCount());
	mRagdoll->GetPose(root_offset, pose1_model.data());

	// Get animated pose in local space
	Array<Mat44> pose2_local(mAnimatedPose.GetJointCount());
	mAnimatedPose.CalculateLocalSpaceJointMatrices(pose2_local.data());

	// Map ragdoll to animated pose, filling in the extra joints using the local space animated pose
	SkeletonPose pose2_world;
	pose2_world.SetSkeleton(mAnimatedPose.GetSkeleton());
	pose2_world.SetRootOffset(root_offset);
	mRagdollToAnimated.Map(pose1_model.data(), pose2_local.data(), pose2_world.GetJointMatrices().data());

#ifdef JPH_DEBUG_RENDERER
	// Draw mapped pose
	pose2_world.Draw(*inParams.mPoseDrawSettings, mDebugRenderer, offset);
	mDebugRenderer->DrawText3D(offset * pose2_world.GetJointMatrix(1).GetTranslation(), "Mapped", Color::sWhite, 0.2f);
#endif // JPH_DEBUG_RENDERER
}

void SkeletonMapperTest::CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu)
{
	inUI->CreateCheckBox(inSubMenu, "Lock Translations", sLockTranslations, [this](UICheckBox::EState inState) { sLockTranslations = inState == UICheckBox::STATE_CHECKED; RestartTest(); });
}

void SkeletonMapperTest::SaveState(StateRecorder &inStream) const
{
	inStream.Write(mTime);
}

void SkeletonMapperTest::RestoreState(StateRecorder &inStream)
{
	inStream.Read(mTime);
}
