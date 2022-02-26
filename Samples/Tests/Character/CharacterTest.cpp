// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Character/CharacterTest.h>
#include <Physics/PhysicsScene.h>
#include <Physics/Collision/Shape/CapsuleShape.h>
#include <Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Core/StringTools.h>
#include <Application/DebugUI.h>
#include <Layers.h>
#include <Utils/Log.h>
#include <Renderer/DebugRendererImp.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(CharacterTest) 
{ 
	JPH_ADD_BASE_CLASS(CharacterTest, Test) 
}

const char *CharacterTest::sScenes[] =
{
	"PerlinMesh",
	"PerlinHeightField",
	"Terrain1",
	"Terrain2",
};

const char *CharacterTest::sSceneName = "Terrain2";

static const float cCharacterHeightStanding = 1.35f;
static const float cCharacterRadiusStanding = 0.3f;
static const float cCharacterHeightCrouching = 0.8f;
static const float cCharacterRadiusCrouching = 0.3f;
static const float cJumpSpeed = 4.0f;
static const float cCollisionTolerance = 0.05f;

CharacterTest::~CharacterTest()
{
	mCharacter->RemoveFromPhysicsSystem();
}

void CharacterTest::Initialize()
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

	// Create 'player' character
	Ref<CharacterSettings> settings = new CharacterSettings();
	settings->mLayer = Layers::MOVING;
	settings->mShape = mStandingShape;
	settings->mFriction = 0.5f;
	mCharacter = new Character(settings, Vec3::sZero(), Quat::sIdentity(), 0, mPhysicsSystem);
	mCharacter->AddToPhysicsSystem(EActivation::Activate);
}

void CharacterTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	// Get the state of the character
	Character::EGroundState ground_state = mCharacter->GetGroundState();

	// Determine controller input
	Vec3 control_input = Vec3::sZero();
	if (inParams.mKeyboard->IsKeyPressed(DIK_LEFT))		control_input.SetX(-1);
	if (inParams.mKeyboard->IsKeyPressed(DIK_RIGHT))	control_input.SetX(1);
	if (inParams.mKeyboard->IsKeyPressed(DIK_UP))		control_input.SetZ(-1);
	if (inParams.mKeyboard->IsKeyPressed(DIK_DOWN))		control_input.SetZ(1);
	if (control_input != Vec3::sZero())
		control_input = control_input.Normalized();

	// Cancel movement in opposite direction of normal when sliding
	if (ground_state == Character::EGroundState::Sliding)
	{
		Vec3 normal = mCharacter->GetGroundNormal();
		normal.SetY(0);
		if (normal.Dot(control_input) <= 0.0f)
			control_input = Vec3::sZero();
	}

	// Update velocity
	const float cMaxSpeed = 6;
	Vec3 current_velocity = mCharacter->GetLinearVelocity();
	Vec3 desired_velocity = cMaxSpeed * control_input;
	desired_velocity.SetY(current_velocity.GetY());
	Vec3 new_velocity = 0.75f * current_velocity + 0.25f * desired_velocity;

	// Check actions
	for (int key = inParams.mKeyboard->GetFirstKey(); key != 0; key = inParams.mKeyboard->GetNextKey())
	{
		if (key == DIK_RETURN)
		{
			// Stance switch
			mCharacter->SetShape(mCharacter->GetShape() == mStandingShape? mCrouchingShape : mStandingShape, 1.5f * mPhysicsSystem->GetPhysicsSettings().mPenetrationSlop);
			break;
		}
		else if (key == DIK_J)
		{
			// Jump
			if (ground_state == Character::EGroundState::OnGround)
				new_velocity += Vec3(0, cJumpSpeed, 0);
		}
	}

	// Update the velocity
	mCharacter->SetLinearVelocity(new_velocity);

	// Get properties
	Vec3 position;
	Quat rotation;
	mCharacter->GetPositionAndRotation(position, rotation);

	// Draw current location
	// Drawing prior to update since the physics system state is also that prior to the simulation step (so that all detected collisions etc. make sense)
	mDebugRenderer->DrawCoordinateSystem(Mat44::sRotationTranslation(rotation, position));

	if (ground_state != Character::EGroundState::InAir)
	{
		Vec3 ground_position = mCharacter->GetGroundPosition();
		Vec3 ground_normal = mCharacter->GetGroundNormal();
		const PhysicsMaterial *ground_material = mCharacter->GetGroundMaterial();

		// Draw ground position
		mDebugRenderer->DrawWireSphere(ground_position, 0.1f, Color::sRed);
		mDebugRenderer->DrawArrow(ground_position, ground_position + 2.0f * ground_normal, Color::sGreen, 0.1f);

		// Draw ground material
		mDebugRenderer->DrawText3D(ground_position, ground_material->GetDebugName());
	}
}

void CharacterTest::PostPhysicsUpdate(float inDeltaTime)
{
	// Fetch the new ground properties
	mCharacter->PostSimulation(cCollisionTolerance);
}

void CharacterTest::GetInitialCamera(CameraState &ioState) const 
{
	// Position camera behind character
	Vec3 cam_tgt = Vec3(0, cCharacterHeightStanding, 0);
	ioState.mPos = Vec3(0, 2.5f, 5);
	ioState.mForward = (cam_tgt - ioState.mPos).Normalized();
}

Mat44 CharacterTest::GetCameraPivot(float inCameraHeading, float inCameraPitch) const 
{
	// Get properties
	Vec3 position;
	Quat rotation;
	mCharacter->GetPositionAndRotation(position, rotation);
	return Mat44::sRotationTranslation(rotation, position);
}

void CharacterTest::CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu)
{
	inUI->CreateTextButton(inSubMenu, "Select Scene", [this, inUI]() { 
		UIElement *scene_name = inUI->CreateMenu();
		for (uint i = 0; i < size(sScenes); ++i)
			inUI->CreateTextButton(scene_name, sScenes[i], [this, i]() { sSceneName = sScenes[i]; RestartTest(); });
		inUI->ShowMenu(scene_name);
	});
}