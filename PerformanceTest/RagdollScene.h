// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

// Jolt includes
#include <Jolt/Physics/Ragdoll/Ragdoll.h>
#include <Jolt/Physics/PhysicsScene.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/ObjectStream/ObjectStreamIn.h>

// Local includes
#include "PerformanceTestScene.h"
#include "Layers.h"

// A scene that loads a part of a Horizon Zero Dawn level and drops many ragdolls on the terrain (motors enabled)
class RagdollScene : public PerformanceTestScene
{
public:
							RagdollScene(int inNumPilesPerAxis, int inPileSize, float inVerticalSeparation) : mNumPilesPerAxis(inNumPilesPerAxis), mPileSize(inPileSize), mVerticalSeparation(inVerticalSeparation) { }

	virtual const char *	GetName() const override
	{
		return mNumPilesPerAxis == 1? "RagdollSinglePile" : "Ragdoll";
	}

	virtual bool			Load() override
	{
		// Load ragdoll
		if (!ObjectStreamIn::sReadObject("Assets/Human.tof", mRagdollSettings))
		{
			cerr << "Unable to load ragdoll" << endl;
			return false;
		}
		for (BodyCreationSettings &body : mRagdollSettings->mParts)
			body.mObjectLayer = Layers::MOVING;

		// Init ragdoll
		mRagdollSettings->GetSkeleton()->CalculateParentJointIndices();
		mRagdollSettings->Stabilize();
		mRagdollSettings->CalculateBodyIndexToConstraintIndex();
		mRagdollSettings->CalculateConstraintIndexToBodyIdxPair();

		// Load animation
		if (!ObjectStreamIn::sReadObject("Assets/Human/dead_pose1.tof", mAnimation))
		{
			cerr << "Unable to load animation" << endl;
			return false;
		}

		// Sample pose
		mPose.SetSkeleton(mRagdollSettings->GetSkeleton());
		mAnimation->Sample(0.0f, mPose);

		// Read the background scene
		if (!ObjectStreamIn::sReadObject("Assets/terrain2.bof", mBackground))
		{
			cerr << "Unable to load terrain" << endl;
			return false;
		}
		for (BodyCreationSettings &body : mBackground->GetBodies())
			body.mObjectLayer = Layers::NON_MOVING;
		mBackground->FixInvalidScales();

		return true;
	}

	virtual void			StartTest(PhysicsSystem &inPhysicsSystem, EMotionQuality inMotionQuality) override
	{
		// Test configuration
		const Real cHorizontalSeparation = 4.0_r;

		// Set motion quality on ragdoll
		for (BodyCreationSettings &body : mRagdollSettings->mParts)
			body.mMotionQuality = inMotionQuality;

		// Add background geometry
		mBackground->CreateBodies(&inPhysicsSystem);

		// Create ragdoll piles
		mt19937 random;
		uniform_real_distribution<float> angle(0.0f, JPH_PI);
		CollisionGroup::GroupID group_id = 1;
		for (int row = 0; row < mNumPilesPerAxis; ++row)
			for (int col = 0; col < mNumPilesPerAxis; ++col)
			{
				// Determine start location of ray
				RVec3 start(cHorizontalSeparation * (col - (mNumPilesPerAxis - 1) / 2.0_r), 100, cHorizontalSeparation * (row - (mNumPilesPerAxis - 1) / 2.0_r));

				// Cast ray down to terrain
				RayCastResult hit;
				Vec3 ray_direction(0, -200, 0);
				RRayCast ray { start, ray_direction };
				if (inPhysicsSystem.GetNarrowPhaseQuery().CastRay(ray, hit, SpecifiedBroadPhaseLayerFilter(BroadPhaseLayers::NON_MOVING), SpecifiedObjectLayerFilter(Layers::NON_MOVING)))
					start = ray.GetPointOnRay(hit.mFraction);

				for (int i = 0; i < mPileSize; ++i)
				{
					// Create ragdoll
					Ref<Ragdoll> ragdoll = mRagdollSettings->CreateRagdoll(group_id++, 0, &inPhysicsSystem);

					// Override root
					SkeletonPose pose_copy = mPose;
					pose_copy.SetRootOffset(start);
					SkeletonPose::JointState &root = pose_copy.GetJoint(0);
					root.mTranslation = Vec3(0, mVerticalSeparation * (i + 1), 0);
					root.mRotation = Quat::sRotation(Vec3::sAxisY(), angle(random)) * root.mRotation;
					pose_copy.CalculateJointMatrices();

					// Drive to pose
					ragdoll->SetPose(pose_copy);
					ragdoll->DriveToPoseUsingMotors(pose_copy);
					ragdoll->AddToPhysicsSystem(EActivation::Activate);

					// Keep reference
					mRagdolls.push_back(ragdoll);
				}
			}
	}

	virtual void			StopTest(PhysicsSystem &inPhysicsSystem) override
	{
		// Remove ragdolls
		for (Ragdoll *ragdoll : mRagdolls)
			ragdoll->RemoveFromPhysicsSystem();
		mRagdolls.clear();
	}

private:
	int 					mNumPilesPerAxis;
	int 					mPileSize;
	float 					mVerticalSeparation;
	Ref<RagdollSettings>	mRagdollSettings;
	Ref<SkeletalAnimation>	mAnimation;
	SkeletonPose			mPose;
	Ref<PhysicsScene>		mBackground;
	Array<Ref<Ragdoll>>		mRagdolls;
};
