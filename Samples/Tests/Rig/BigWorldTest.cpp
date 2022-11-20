// SPDX-FileCopyrightText: 2022 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Rig/BigWorldTest.h>
#include <Jolt/Physics/PhysicsScene.h>
#include <Jolt/Skeleton/Skeleton.h>
#include <Jolt/Skeleton/SkeletalAnimation.h>
#include <Jolt/Skeleton/SkeletonPose.h>
#include <Jolt/Core/StringTools.h>
#include <Jolt/ObjectStream/ObjectStreamIn.h>
#include <Utils/RagdollLoader.h>
#include <Application/DebugUI.h>
#include <Renderer/DebugRendererImp.h>
#include <Layers.h>
#include <Utils/Log.h>
#include <random>

JPH_IMPLEMENT_RTTI_VIRTUAL(BigWorldTest) 
{ 
	JPH_ADD_BASE_CLASS(BigWorldTest, Test) 
}

BigWorldTest::~BigWorldTest()
{
	for (Pile &pile : mPiles)
		for (Ragdoll *r : pile.mRagdolls)
			r->RemoveFromPhysicsSystem();
}

void BigWorldTest::Initialize()
{
	constexpr int cPileSize = 5;
	
	// Default terrain
	Body &floor = CreateMeshTerrain();
	RefConst<Shape> shape = floor.GetShape();

	// Load ragdoll
	Ref<RagdollSettings> settings = RagdollLoader::sLoad("Assets/Human.tof", EMotionType::Dynamic);

	// Load animation
	Ref<SkeletalAnimation> animation;
	if (!ObjectStreamIn::sReadObject("Assets/Human/Dead_Pose1.tof", animation))
		FatalError("Could not open animation");
	SkeletonPose pose;
	pose.SetSkeleton(settings->GetSkeleton());
	animation->Sample(0.0f, pose);

	// Determine rotation for each ragdoll in the pile
	default_random_engine random;
	uniform_real_distribution<float> angle(0.0f, JPH_PI);
	Array<Quat> rotation;
	for (int i = 0; i < cPileSize; ++i)
		rotation.push_back(Quat::sRotation(Vec3::sAxisY(), angle(random)) * pose.GetJoint(0).mRotation);

	// Create piles at various distances
	Real distances[] = { 0.0_r, 1.0e3_r, 5.0e3_r, 1.0e4_r, 5.0e4_r, 1.0e5_r, 5.0e5_r, 1.0e6_r };
	for (Real distance : distances)
	{
		// Create floor (floor at 0 was already created)
		if (distance != 0.0f)
			mBodyInterface->CreateAndAddBody(BodyCreationSettings(shape, RVec3(distance, 0, 0), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);

		// Create pile of ragdolls
		Pile pile;
		pile.mDistance = distance;
		for (int i = 0; i < cPileSize; ++i)
		{
			// Create ragdoll
			Ref<Ragdoll> ragdoll = settings->CreateRagdoll(0, 0, mPhysicsSystem);
	
			// Override root
			SkeletonPose::JointState &root = pose.GetJoint(0);
			root.mTranslation = Vec3::sZero();
			root.mRotation = rotation[i];
			pose.SetRootOffset(RVec3(distance, 5.0f + i, 0));
			pose.CalculateJointMatrices();

			// Drive to pose
			ragdoll->SetPose(pose);
			ragdoll->DriveToPoseUsingMotors(pose);
			ragdoll->AddToPhysicsSystem(EActivation::Activate);

			pile.mRagdolls.push_back(ragdoll);
		}

		mPiles.push_back(pile);
	}
}

void BigWorldTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	int pile_idx = 0;

	for (Pile &pile : mPiles)
		if (pile.mDistance != 0.0f) // Pile at 0 is drawn normally
		{
			// Check if we need to draw this pile
			if ((sDrawPileMask & (1 << pile_idx)) != 0)
			{
				Color color = Color::sGetDistinctColor(pile_idx);
				bool first = true;

				for (Ragdoll *r : pile.mRagdolls)
				{
					for (const BodyID &id : r->GetBodyIDs())
					{
						BodyLockRead lock(mPhysicsSystem->GetBodyLockInterface(), id);
						if (lock.Succeeded())
						{
							const Body &body = lock.GetBody();

							// Shift the transform back to the origin
							RMat44 transform = body.GetCenterOfMassTransform();
							transform.SetTranslation(transform.GetTranslation() - RVec3(pile.mDistance, 0, 0));

							// Draw distance label for the first body
							if (first)
							{
								mDebugRenderer->DrawText3D(transform.GetTranslation(), pile.GetLabel().c_str(), color, 0.2f);
								first = false;
							}

						#ifdef JPH_DEBUG_RENDERER
							// Draw the shape
							body.GetShape()->Draw(mDebugRenderer, transform, Vec3::sReplicate(1.0f), color, false, true);
						#endif // JPH_DEBUG_RENDERER
						}
					}
				}
			}

			pile_idx++;
		}
}

void BigWorldTest::CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu)
{
	// Enable / disable drawing of a particular distance
	int pile_idx = 0;
	for (Pile &pile : mPiles)
		if (pile.mDistance != 0.0f)
		{
			uint32 mask = 1 << pile_idx;
			inUI->CreateCheckBox(inSubMenu, "Draw pile at " + pile.GetLabel(), (sDrawPileMask & mask) != 0, [mask](UICheckBox::EState inState) { if (inState == UICheckBox::STATE_CHECKED) sDrawPileMask |= mask; else sDrawPileMask &= ~mask; });
			pile_idx++;
		}
}

