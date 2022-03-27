// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Rig/RigPileTest.h>
#include <Jolt/Physics/PhysicsScene.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Skeleton/Skeleton.h>
#include <Jolt/Skeleton/SkeletalAnimation.h>
#include <Jolt/Skeleton/SkeletonPose.h>
#include <Utils/RagdollLoader.h>
#include <Jolt/Core/StringTools.h>
#include <Application/DebugUI.h>
#include <Layers.h>
#include <Utils/Log.h>
#include <random>

JPH_IMPLEMENT_RTTI_VIRTUAL(RigPileTest) 
{ 
	JPH_ADD_BASE_CLASS(RigPileTest, Test) 
}

const char *RigPileTest::sScenes[] =
{
	"PerlinMesh",
	"PerlinHeightField",
	"Terrain1",
	"Terrain2",
};

#ifdef _DEBUG
	const char *RigPileTest::sSceneName = "PerlinMesh";
#else
	const char *RigPileTest::sSceneName = "Terrain1";
#endif

RigPileTest::~RigPileTest()
{
	for (Ragdoll *r : mRagdolls)
		r->RemoveFromPhysicsSystem();
}

void RigPileTest::Initialize()
{
	if (strcmp(sSceneName, "PerlinMesh") == 0)
	{
		// Default terrain
		CreateMeshTerrain();
	}
	else if (strcmp(sSceneName, "PerlinHeightField") == 0)
	{
		// Default terrain
		CreateHeightFieldTerrain();
	}	
	else
	{
		// Load scene
		Ref<PhysicsScene> scene;
		if (!ObjectStreamIn::sReadObject((string("Assets/") + sSceneName + ".bof").c_str(), scene))
			FatalError("Failed to load scene");
		for (BodyCreationSettings &body : scene->GetBodies())
			body.mObjectLayer = Layers::NON_MOVING;
		scene->FixInvalidScales();
		scene->CreateBodies(mPhysicsSystem);
	}

	// Load ragdoll
	Ref<RagdollSettings> settings = RagdollLoader::sLoad("Assets/Human.tof", EMotionType::Dynamic);

	// Load animation
	const int cAnimationCount = 4;
	Ref<SkeletalAnimation> animation[cAnimationCount];
	for (int i = 0; i < cAnimationCount; ++i)
	{
		if (!ObjectStreamIn::sReadObject(StringFormat("Assets/Human/Dead_Pose%d.tof", i + 1).c_str(), animation[i]))
			FatalError("Could not open animation");
	}

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

	// Create piles
	default_random_engine random;
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
			if (mPhysicsSystem->GetNarrowPhaseQuery().CastRay(ray, hit, SpecifiedBroadPhaseLayerFilter(BroadPhaseLayers::NON_MOVING), SpecifiedObjectLayerFilter(Layers::NON_MOVING)))
				start = start + hit.mFraction * ray_direction;

			for (int i = 0; i < cPileSize; ++i)
			{
				// Create ragdoll
				Ref<Ragdoll> ragdoll = settings->CreateRagdoll(group_id++, 0, mPhysicsSystem);
	
				// Sample pose
				SkeletonPose pose;
				pose.SetSkeleton(settings->GetSkeleton());
				animation[random() % cAnimationCount]->Sample(0.0f, pose);

				// Override root
				SkeletonPose::JointState &root = pose.GetJoint(0);
				root.mTranslation = start + Vec3(0, cVerticalSeparation * (i + 1), 0);
				root.mRotation = Quat::sRotation(Vec3::sAxisY(), angle(random)) * root.mRotation;
				pose.CalculateJointMatrices();

				// Drive to pose
				ragdoll->SetPose(pose);
				ragdoll->DriveToPoseUsingMotors(pose);
				ragdoll->AddToPhysicsSystem(EActivation::Activate);

				mRagdolls.push_back(ragdoll);
			}
		}
}

void RigPileTest::CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu)
{
	inUI->CreateTextButton(inSubMenu, "Select Scene", [this, inUI]() { 
		UIElement *scene_name = inUI->CreateMenu();
		for (uint i = 0; i < size(sScenes); ++i)
			inUI->CreateTextButton(scene_name, sScenes[i], [this, i]() { sSceneName = sScenes[i]; RestartTest(); });
		inUI->ShowMenu(scene_name);
	});
}

