// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

// Jolt includes
#include <Jolt/Physics/Ragdoll/Ragdoll.h>
#include <Jolt/Physics/PhysicsScene.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/RayCast.h>

// Local includes
#include "PerformanceTestScene.h"
#include "Layers.h"

// A scene that loads a part of a Horizon Zero Dawn level and drops many ragdolls on the terrain (motors enabled)
class RagdollScene : public PerformanceTestScene
{
public:
	virtual const char *	GetName() const override
	{
		return "Ragdoll";
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
		const float cHorizontalSeparation = 4.0f;
		const float cVerticalSeparation = 0.6f;
		#ifdef _DEBUG
			const int cPileSize = 5;
			const int cNumRows = 2;
			const int cNumCols = 2;
		#else
			const int cPileSize = 10;
			const int cNumRows = 4;
			const int cNumCols = 4;
		#endif

		// Set motion quality on ragdoll
		for (BodyCreationSettings &body : mRagdollSettings->mParts)
			body.mMotionQuality = inMotionQuality;

		// Add background geometry
		mBackground->CreateBodies(&inPhysicsSystem);

		// Create ragdoll piles
		mt19937 random;
		uniform_real_distribution<float> angle(0.0f, JPH_PI);
		CollisionGroup::GroupID group_id = 1;
		for (int row = 0; row < cNumRows; ++row)
			for (int col = 0; col < cNumCols; ++col)
			{
				// Determine start location of ray
				Vec3 start = Vec3(cHorizontalSeparation * (col - (cNumCols - 1) / 2.0f), 100, cHorizontalSeparation * (row - (cNumRows - 1) / 2.0f));

				// Cast ray down to terrain
				RayCastResult hit;
				Vec3 ray_direction(0, -200, 0);
				RayCast ray { start, ray_direction };
				if (inPhysicsSystem.GetNarrowPhaseQuery().CastRay(ray, hit, SpecifiedBroadPhaseLayerFilter(BroadPhaseLayers::NON_MOVING), SpecifiedObjectLayerFilter(Layers::NON_MOVING)))
					start = start + hit.mFraction * ray_direction;

				for (int i = 0; i < cPileSize; ++i)
				{
					// Create ragdoll
					Ref<Ragdoll> ragdoll = mRagdollSettings->CreateRagdoll(group_id++, 0, &inPhysicsSystem);
	
					// Override root
					SkeletonPose pose_copy = mPose;
					SkeletonPose::JointState &root = pose_copy.GetJoint(0);
					root.mTranslation = start + Vec3(0, cVerticalSeparation * (i + 1), 0);
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
	Ref<RagdollSettings>	mRagdollSettings;
	Ref<SkeletalAnimation>	mAnimation;
	SkeletonPose			mPose;
	Ref<PhysicsScene>		mBackground;
	vector<Ref<Ragdoll>>	mRagdolls;
};
