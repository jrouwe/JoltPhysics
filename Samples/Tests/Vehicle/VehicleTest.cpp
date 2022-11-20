// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Vehicle/VehicleTest.h>
#include <Jolt/Physics/Constraints/DistanceConstraint.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/GroupFilterTable.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/PhysicsScene.h>
#include <Jolt/ObjectStream/ObjectStreamIn.h>
#include <Layers.h>
#include <Application/DebugUI.h>
#include <Utils/Log.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(VehicleTest) 
{ 
	JPH_ADD_BASE_CLASS(VehicleTest, Test) 
}

const char *VehicleTest::sScenes[] =
{
	"Flat",
	"Steep Slope",
	"Playground",
	"Terrain1",
};

const char *VehicleTest::sSceneName = "Playground";

void VehicleTest::Initialize()
{
	if (strcmp(sSceneName, "Flat") == 0)
	{
		// Flat test floor
		Body &floor = *mBodyInterface->CreateBody(BodyCreationSettings(new BoxShape(Vec3(1000.0f, 1.0f, 1000.0f), 0.0f), RVec3(0.0f, -1.0f, 0.0f), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING));
		floor.SetFriction(1.0f);
		mBodyInterface->AddBody(floor.GetID(), EActivation::DontActivate);
	}
	else if (strcmp(sSceneName, "Steep Slope") == 0)
	{
		// Steep slope test floor (20 degrees = 36% grade)
		Body &floor = *mBodyInterface->CreateBody(BodyCreationSettings(new BoxShape(Vec3(1000.0f, 1.0f, 1000.0f), 0.0f), RVec3(0.0f, -1.0f, 0.0f), Quat::sRotation(Vec3::sAxisX(), DegreesToRadians(-20.0f)), EMotionType::Static, Layers::NON_MOVING));
		floor.SetFriction(1.0f);
		mBodyInterface->AddBody(floor.GetID(), EActivation::DontActivate);
	}
	else if (strcmp(sSceneName, "Playground") == 0)
	{
		// Scene with hilly terrain and some objects to drive into
		Body &floor = CreateMeshTerrain();
		floor.SetFriction(1.0f);

		CreateBridge();

		CreateWall();

		CreateRubble();
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
}

void VehicleTest::CreateBridge()
{
	const int cChainLength = 20;

	// Build a collision group filter that disables collision between adjacent bodies
	Ref<GroupFilterTable> group_filter = new GroupFilterTable(cChainLength);
	for (CollisionGroup::SubGroupID i = 0; i < cChainLength - 1; ++i)
		group_filter->DisableCollision(i, i + 1);

	Vec3 part_half_size = Vec3(2.5f, 0.25f, 1.0f);
	RefConst<Shape> part_shape = new BoxShape(part_half_size);

	Vec3 large_part_half_size = Vec3(2.5f, 0.25f, 22.5f);
	RefConst<Shape> large_part_shape = new BoxShape(large_part_half_size);

	Quat first_part_rot = Quat::sRotation(Vec3::sAxisX(), DegreesToRadians(-10.0f));

	RVec3 prev_pos(-25, 7, 0);
	Body *prev_part = nullptr;

	for (int i = 0; i < cChainLength; ++i)
	{
		RVec3 pos = prev_pos + Vec3(0, 0, 2.0f * part_half_size.GetZ());

		Body &part = i == 0? *mBodyInterface->CreateBody(BodyCreationSettings(large_part_shape, pos - first_part_rot * Vec3(0, large_part_half_size.GetY() - part_half_size.GetY(), large_part_half_size.GetZ() - part_half_size.GetZ()), first_part_rot, EMotionType::Static, Layers::NON_MOVING))
					: *mBodyInterface->CreateBody(BodyCreationSettings(part_shape, pos, Quat::sIdentity(), i == 19? EMotionType::Static : EMotionType::Dynamic, i == 19? Layers::NON_MOVING : Layers::MOVING));
		part.SetCollisionGroup(CollisionGroup(group_filter, 1, CollisionGroup::SubGroupID(i)));
		part.SetFriction(1.0f);
		mBodyInterface->AddBody(part.GetID(), EActivation::Activate);

		if (prev_part != nullptr)
		{
			DistanceConstraintSettings dc;
			dc.mPoint1 = prev_pos + Vec3(-part_half_size.GetX(), 0, part_half_size.GetZ());
			dc.mPoint2 = pos + Vec3(-part_half_size.GetX(), 0, -part_half_size.GetZ());
			mPhysicsSystem->AddConstraint(dc.Create(*prev_part, part));

			dc.mPoint1 = prev_pos + Vec3(part_half_size.GetX(), 0, part_half_size.GetZ());
			dc.mPoint2 = pos + Vec3(part_half_size.GetX(), 0, -part_half_size.GetZ());
			mPhysicsSystem->AddConstraint(dc.Create(*prev_part, part));
		}

		prev_part = &part;
		prev_pos = pos;
	}
}

void VehicleTest::CreateWall()
{
	RefConst<Shape> box_shape = new BoxShape(Vec3(0.5f, 0.5f, 0.5f));
	for (int i = 0; i < 3; ++i)
		for (int j = i / 2; j < 5 - (i + 1) / 2; ++j)
		{
			RVec3 position(2.0f + j * 1.0f + (i & 1? 0.5f : 0.0f), 2.0f + i * 1.0f, 10.0f);
			mBodyInterface->CreateAndAddBody(BodyCreationSettings(box_shape, position, Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING), EActivation::Activate);
		}
}

void VehicleTest::CreateRubble()
{
	// Flat and light objects
	RefConst<Shape> box_shape = new BoxShape(Vec3(0.5f, 0.1f, 0.5f));
	for (int i = 0; i < 5; ++i)
		for (int j = 0; j < 5; ++j)
		{
			RVec3 position(-5.0f + j, 2.0f + i * 0.2f, 10.0f + 0.5f * i);
			mBodyInterface->CreateAndAddBody(BodyCreationSettings(box_shape, position, Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING), EActivation::Activate);
		}


	// Light convex shapes
	default_random_engine random;
	uniform_real_distribution<float> hull_size(0.2f, 0.4f);
	for (int i = 0; i < 10; ++i)
		for (int j = 0; j < 10; ++j)
		{
			// Create random points
			Array<Vec3> points;
			for (int k = 0; k < 20; ++k)
				points.push_back(hull_size(random) * Vec3::sRandom(random));

			mBodyInterface->CreateAndAddBody(BodyCreationSettings(new ConvexHullShapeSettings(points), RVec3(-5.0f + 0.5f * j, 2.0f, 15.0f + 0.5f * i), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING), EActivation::Activate);
		}
}

void VehicleTest::CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu)
{
	inUI->CreateTextButton(inSubMenu, "Select Scene", [this, inUI]() { 
		UIElement *scene_name = inUI->CreateMenu();
		for (uint i = 0; i < size(sScenes); ++i)
			inUI->CreateTextButton(scene_name, sScenes[i], [this, i]() { sSceneName = sScenes[i]; RestartTest(); });
		inUI->ShowMenu(scene_name);
	});
}

