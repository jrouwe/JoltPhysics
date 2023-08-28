// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2022 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>
#include <Jolt/Skeleton/Skeleton.h>
#include <Jolt/Skeleton/SkeletalAnimation.h>
#include <Jolt/Skeleton/SkeletonPose.h>
#include <Jolt/Skeleton/SkeletonMapper.h>
#include <Utils/RagdollLoader.h>
#include <Jolt/Physics/Ragdoll/Ragdoll.h>

// This test takes shows how you can map a high detail animation skeleton on a low detail physics skeleton and back
class SkeletonMapperTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, SkeletonMapperTest)

	// Destructor
	virtual					~SkeletonMapperTest() override;

	// Number used to scale the terrain and camera movement to the scene
	virtual float			GetWorldScale() const override								{ return 0.2f; }

	virtual void			Initialize() override;
	virtual void			PrePhysicsUpdate(const PreUpdateParams &inParams) override;

	// Optional settings menu
	virtual bool			HasSettingsMenu() const override							{ return true; }
	virtual void			CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu) override;

	// Saving / restoring state for replay
	virtual void			SaveState(StateRecorder &inStream) const override;
	virtual void			RestoreState(StateRecorder &inStream) override;

private:
	inline static bool		sLockTranslations = false;

	void					CalculateRagdollPose();

	float					mTime = 0.0f;
	Ref<RagdollSettings>	mRagdollSettings;
	Ref<Ragdoll>			mRagdoll;
	Ref<SkeletalAnimation>	mAnimation;
	SkeletonMapper			mRagdollToAnimated;
	SkeletonPose			mAnimatedPose;
	SkeletonPose			mRagdollPose;
};
