// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Character/CharacterTestBase.h>
#include <Jolt/Physics/PhysicsScene.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Core/StringTools.h>
#include <Application/DebugUI.h>
#include <Layers.h>
#include <Utils/Log.h>

JPH_IMPLEMENT_RTTI_ABSTRACT(CharacterTestBase) 
{ 
	JPH_ADD_BASE_CLASS(CharacterTestBase, Test) 
}

const char *CharacterTestBase::sScenes[] =
{
	"PerlinMesh",
	"PerlinHeightField",
	"ObstacleCourse",
	"Terrain1",
	"Terrain2",
};

const char *CharacterTestBase::sSceneName = "ObstacleCourse";

void CharacterTestBase::Initialize()
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
	else if (strcmp(sSceneName, "ObstacleCourse") == 0)
	{
		// Default terrain
		CreateFloor();

		// Create ramps with different inclinations
		Ref<Shape> ramp = RotatedTranslatedShapeSettings(Vec3(0, 0, -2.5f), Quat::sIdentity(), new BoxShape(Vec3(0.5f, 0.05f, 2.5f))).Create().Get();
		for (int angle = 0; angle < 18; ++angle)
			mBodyInterface->CreateAndAddBody(BodyCreationSettings(ramp, Vec3(-10.0f + angle * 1.0f, 0, -10.0f), Quat::sRotation(Vec3::sAxisX(), DegreesToRadians(10.0f * angle)), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);

		// Creat wall consisting of vertical pillars
		// Note: Convex radius 0 because otherwise it will be a bumpy wall
		Ref<Shape> wall = new BoxShape(Vec3(0.1f, 2.5f, 0.1f), 0.0f); 
		for (int z = 0; z < 40; ++z)
			mBodyInterface->CreateAndAddBody(BodyCreationSettings(wall, Vec3(-10.0f, 2.5f, -10.0f + 0.2f * z), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);
	}
	else
	{
		// Load scene
		Ref<PhysicsScene> scene;
		if (!ObjectStreamIn::sReadObject((string("Assets/") + sSceneName + ".bof").c_str(), scene))
			FatalError("Failed to load scene");
		scene->FixInvalidScales();
		for (BodyCreationSettings &settings : scene->GetBodies())
		{
			settings.mObjectLayer = Layers::NON_MOVING;
			settings.mFriction = 0.5f;
		}
		scene->CreateBodies(mPhysicsSystem);
	}

	// Create capsule shapes for all stances
	mStandingShape = RotatedTranslatedShapeSettings(Vec3(0, 0.5f * cCharacterHeightStanding + cCharacterRadiusStanding, 0), Quat::sIdentity(), new CapsuleShape(0.5f * cCharacterHeightStanding, cCharacterRadiusStanding)).Create().Get();
	mCrouchingShape = RotatedTranslatedShapeSettings(Vec3(0, 0.5f * cCharacterHeightCrouching + cCharacterRadiusCrouching, 0), Quat::sIdentity(), new CapsuleShape(0.5f * cCharacterHeightCrouching, cCharacterRadiusCrouching)).Create().Get();
}

void CharacterTestBase::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	// Determine controller input
	Vec3 control_input = Vec3::sZero();
	if (inParams.mKeyboard->IsKeyPressed(DIK_LEFT))		control_input.SetZ(-1);
	if (inParams.mKeyboard->IsKeyPressed(DIK_RIGHT))	control_input.SetZ(1);
	if (inParams.mKeyboard->IsKeyPressed(DIK_UP))		control_input.SetX(1);
	if (inParams.mKeyboard->IsKeyPressed(DIK_DOWN))		control_input.SetX(-1);
	if (control_input != Vec3::sZero())
		control_input = control_input.Normalized();

	// Rotate controls to align with the camera
	Vec3 cam_fwd = inParams.mCameraState.mForward;
	cam_fwd.SetY(0.0f);
	cam_fwd = cam_fwd.NormalizedOr(Vec3::sAxisX());
	Quat rotation = Quat::sFromTo(Vec3::sAxisX(), cam_fwd);
	control_input = rotation * control_input;

	// Check actions
	bool jump = false;
	bool switch_stance = false;
	for (int key = inParams.mKeyboard->GetFirstKey(); key != 0; key = inParams.mKeyboard->GetNextKey())
	{
		if (key == DIK_RSHIFT)
			switch_stance = true;
		else if (key == DIK_RCONTROL)
			jump = true;
	}

	HandleInput(control_input, jump, switch_stance, inParams.mDeltaTime);
}

void CharacterTestBase::CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu)
{
	inUI->CreateTextButton(inSubMenu, "Select Scene", [this, inUI]() { 
		UIElement *scene_name = inUI->CreateMenu();
		for (uint i = 0; i < size(sScenes); ++i)
			inUI->CreateTextButton(scene_name, sScenes[i], [this, i]() { sSceneName = sScenes[i]; RestartTest(); });
		inUI->ShowMenu(scene_name);
	});
}

void CharacterTestBase::GetInitialCamera(CameraState& ioState) const
{
	// This will become the local space offset, look down the x axis and slightly down
	ioState.mPos = Vec3::sZero();
	ioState.mForward = Vec3(10.0f, -2.0f, 0).Normalized();
}

Mat44 CharacterTestBase::GetCameraPivot(float inCameraHeading, float inCameraPitch) const 
{
	// Pivot is center of character + distance behind based on the heading and pitch of the camera
	Vec3 fwd = Vec3(cos(inCameraPitch) * cos(inCameraHeading), sin(inCameraPitch), cos(inCameraPitch) * sin(inCameraHeading));
	return Mat44::sTranslation(GetCharacterPosition() + Vec3(0, cCharacterHeightStanding + cCharacterRadiusStanding, 0) - 5.0f * fwd);
}
