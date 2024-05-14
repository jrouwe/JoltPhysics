// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
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
#include <Jolt/Core/StringTools.h>
#include <Jolt/ObjectStream/ObjectStreamIn.h>
#include <Utils/RagdollLoader.h>
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

#ifdef JPH_DEBUG
	const char *RigPileTest::sSceneName = "PerlinMesh";
	int RigPileTest::sPileSize = 5;
	int RigPileTest::sNumPilesPerAxis = 2;
#else
	const char *RigPileTest::sSceneName = "Terrain1";
	int RigPileTest::sPileSize = 10;
	int RigPileTest::sNumPilesPerAxis = 4;
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
		if (!ObjectStreamIn::sReadObject((String("Assets/") + sSceneName + ".bof").c_str(), scene))
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

	// Limit the size of the piles so we don't go over 160 ragdolls
	int pile_size = min(sPileSize, 160 / Square(sNumPilesPerAxis));

	// Create piles
	default_random_engine random;
	uniform_real_distribution<float> angle(0.0f, JPH_PI);
	CollisionGroup::GroupID group_id = 1;
	for (int row = 0; row < sNumPilesPerAxis; ++row)
		for (int col = 0; col < sNumPilesPerAxis; ++col)
		{
			// Determine start location of ray
			RVec3 start = RVec3(cHorizontalSeparation * (col - (sNumPilesPerAxis - 1) / 2.0f), 100, cHorizontalSeparation * (row - (sNumPilesPerAxis - 1) / 2.0f));

			// Cast ray down to terrain
			RayCastResult hit;
			Vec3 ray_direction(0, -200, 0);
			RRayCast ray { start, ray_direction };
			if (mPhysicsSystem->GetNarrowPhaseQuery().CastRay(ray, hit, SpecifiedBroadPhaseLayerFilter(BroadPhaseLayers::NON_MOVING), SpecifiedObjectLayerFilter(Layers::NON_MOVING)))
				start = ray.GetPointOnRay(hit.mFraction);

			for (int i = 0; i < pile_size; ++i)
			{
				// Create ragdoll
				Ref<Ragdoll> ragdoll = settings->CreateRagdoll(group_id++, 0, mPhysicsSystem);

				// Sample pose
				SkeletonPose pose;
				pose.SetSkeleton(settings->GetSkeleton());
				animation[random() % cAnimationCount]->Sample(0.0f, pose);

				// Override root
				pose.SetRootOffset(start);
				SkeletonPose::JointState &root = pose.GetJoint(0);
				root.mTranslation = Vec3(0, cVerticalSeparation * (i + 1), 0);
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

	inUI->CreateSlider(inSubMenu, "Num Ragdolls Per Pile", float(sPileSize), 1, 160, 1, [](float inValue) { sPileSize = (int)inValue; });
	inUI->CreateSlider(inSubMenu, "Num Piles Per Axis", float(sNumPilesPerAxis), 1, 4, 1, [](float inValue) { sNumPilesPerAxis = (int)inValue; });
}
