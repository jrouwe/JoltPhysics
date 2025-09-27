// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>
#include <Jolt/Skeleton/Skeleton.h>
#include <Jolt/Skeleton/SkeletalAnimation.h>
#include <Jolt/Skeleton/SkeletonPose.h>
#include <Utils/RagdollLoader.h>
#include <Jolt/Physics/Ragdoll/Ragdoll.h>

class SoftKeyframedRigTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, SoftKeyframedRigTest)

	// Description of the test
	virtual const char *	GetDescription() const override
	{
		return "Tests a soft keyframed ragdoll moving towards a wall of boxes.\n"
			"This applies velocities to dynamic bodies to force the ragdoll to follow an animation.\n"
			"Since the bodies are dynamic, they will collide with static objects.";
	}

	// Destructor
	virtual					~SoftKeyframedRigTest() override;

	// Number used to scale the terrain and camera movement to the scene
	virtual float			GetWorldScale() const override								{ return 0.2f; }

	virtual void			Initialize() override;
	virtual void			PrePhysicsUpdate(const PreUpdateParams &inParams) override;

	// Saving / restoring state for replay
	virtual void			SaveState(StateRecorder &inStream) const override;
	virtual void			RestoreState(StateRecorder &inStream) override;

private:
	float					mTime = 0.0f;
	Ref<RagdollSettings>	mRagdollSettings;
	Ref<Ragdoll>			mRagdoll;
	Ref<SkeletalAnimation>	mAnimation;
	SkeletonPose			mPose;
};
