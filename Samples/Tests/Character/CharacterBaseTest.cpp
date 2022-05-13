// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Character/CharacterBaseTest.h>
#include <Jolt/Physics/PhysicsScene.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Core/StringTools.h>
#include <Application/DebugUI.h>
#include <Layers.h>
#include <Utils/Log.h>
#include <Renderer/DebugRendererImp.h>

JPH_IMPLEMENT_RTTI_ABSTRACT(CharacterBaseTest) 
{ 
	JPH_ADD_BASE_CLASS(CharacterBaseTest, Test) 
}

const char *CharacterBaseTest::sScenes[] =
{
	"PerlinMesh",
	"PerlinHeightField",
	"ObstacleCourse",
	"Terrain1",
	"Terrain2",
};

const char *CharacterBaseTest::sSceneName = "ObstacleCourse";

// Scene constants
static const Vec3 cRotatingPosition(-5, 0.15f, 15);
static const Quat cRotatingOrientation = Quat::sIdentity();
static const Vec3 cVerticallyMovingPosition(0, 2.0f, 15);
static const Quat cVerticallyMovingOrientation = Quat::sIdentity();
static const Vec3 cHorizontallyMovingPosition(5, 1, 15);
static const Quat cHorizontallyMovingOrientation = Quat::sRotation(Vec3::sAxisZ(), 0.5f * JPH_PI);
static const Vec3 cRampPosition(15, 2.2f, 15);
static const Quat cRampOrientation = Quat::sRotation(Vec3::sAxisX(), -0.25f * JPH_PI);
static const Vec3 cRampBlocksStart = cRampPosition + Vec3(-3.0f, 3.0f, 1.5f);
static const Vec3 cRampBlocksDelta = Vec3(2.0f, 0, 0);
static const float cRampBlocksTime = 5.0f;
static const Vec3 cBumpsPosition = Vec3(-5.0f, 0, 2.5f);
static const float cBumpHeight = 0.05f;
static const float cBumpWidth = 0.01f;
static const float cBumpDelta = 0.5f;
static const Vec3 cStairsPosition = Vec3(-10.0f, 0, 2.5f);
static const float cStairsStepHeight = 0.3f;

void CharacterBaseTest::Initialize()
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

		{
			// Create ramps with different inclinations
			Ref<Shape> ramp = RotatedTranslatedShapeSettings(Vec3(0, 0, -2.5f), Quat::sIdentity(), new BoxShape(Vec3(1.0f, 0.05f, 2.5f))).Create().Get();
			for (int angle = 0; angle < 18; ++angle)
				mBodyInterface->CreateAndAddBody(BodyCreationSettings(ramp, Vec3(-15.0f + angle * 2.0f, 0, -10.0f), Quat::sRotation(Vec3::sAxisX(), DegreesToRadians(10.0f * angle)), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);
		}

		{
			// Create wall consisting of vertical pillars
			// Note: Convex radius 0 because otherwise it will be a bumpy wall
			Ref<Shape> wall = new BoxShape(Vec3(0.1f, 2.5f, 0.1f), 0.0f); 
			for (int z = 0; z < 40; ++z)
				mBodyInterface->CreateAndAddBody(BodyCreationSettings(wall, Vec3(-10.0f, 2.5f, -10.0f + 0.2f * z), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);
		}

		{
			// Kinematic blocks to test interacting with moving objects
			Ref<Shape> kinematic = new BoxShape(Vec3(1, 0.15f, 3.0f));
			mRotatingBody = mBodyInterface->CreateAndAddBody(BodyCreationSettings(kinematic, cRotatingPosition, cRotatingOrientation, EMotionType::Kinematic, Layers::MOVING), EActivation::Activate);
			mVerticallyMovingBody = mBodyInterface->CreateAndAddBody(BodyCreationSettings(kinematic, cVerticallyMovingPosition, cVerticallyMovingOrientation, EMotionType::Kinematic, Layers::MOVING), EActivation::Activate);
			mHorizontallyMovingBody = mBodyInterface->CreateAndAddBody(BodyCreationSettings(kinematic, cHorizontallyMovingPosition, cHorizontallyMovingOrientation, EMotionType::Kinematic, Layers::MOVING), EActivation::Activate);
		}

		{
			// Dynamic blocks to test player pushing blocks
			Ref<Shape> block = new BoxShape(Vec3::sReplicate(0.5f));
			for (int y = 0; y < 3; ++y)
			{
				BodyCreationSettings bcs(block, Vec3(5.0f, 0.5f + float(y), 0.0f), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
				bcs.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
				bcs.mMassPropertiesOverride.mMass = 10.0f;
				mBodyInterface->CreateAndAddBody(bcs, EActivation::DontActivate);
			}
		}

		{
			// Create ramp
			BodyCreationSettings ramp(new BoxShape(Vec3(4.0f, 0.1f, 3.0f)), cRampPosition, cRampOrientation, EMotionType::Static, Layers::NON_MOVING);
			mBodyInterface->CreateAndAddBody(ramp, EActivation::DontActivate);

			// Create blocks on ramp
			Ref<Shape> block = new BoxShape(Vec3::sReplicate(0.5f));
			BodyCreationSettings bcs(block, cRampBlocksStart, cRampOrientation, EMotionType::Dynamic, Layers::MOVING);
			bcs.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
			bcs.mMassPropertiesOverride.mMass = 10.0f;
			for (int i = 0; i < 4; ++i)
			{
				mRampBlocks.emplace_back(mBodyInterface->CreateAndAddBody(bcs, EActivation::Activate));
				bcs.mPosition += cRampBlocksDelta;
			}
		}

		// Create three funnels with walls that are too steep to climb
		Ref<Shape> funnel = new BoxShape(Vec3(0.1f, 1.0f, 1.0f));
		for (int i = 0; i < 2; ++i)
		{
			Quat rotation = Quat::sRotation(Vec3::sAxisY(), JPH_PI * i);
			mBodyInterface->CreateAndAddBody(BodyCreationSettings(funnel, Vec3(5.0f, 0.1f, 5.0f) + rotation * Vec3(0.2f, 0, 0), rotation * Quat::sRotation(Vec3::sAxisZ(), -DegreesToRadians(40.0f)), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);
		}
		for (int i = 0; i < 3; ++i)
		{
			Quat rotation = Quat::sRotation(Vec3::sAxisY(), 2.0f / 3.0f * JPH_PI * i);
			mBodyInterface->CreateAndAddBody(BodyCreationSettings(funnel, Vec3(7.5f, 0.1f, 5.0f) + rotation * Vec3(0.2f, 0, 0), rotation * Quat::sRotation(Vec3::sAxisZ(), -DegreesToRadians(40.0f)), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);
		}
		for (int i = 0; i < 4; ++i)
		{
			Quat rotation = Quat::sRotation(Vec3::sAxisY(), 0.5f * JPH_PI * i);
			mBodyInterface->CreateAndAddBody(BodyCreationSettings(funnel, Vec3(10.0f, 0.1f, 5.0f) + rotation * Vec3(0.2f, 0, 0), rotation * Quat::sRotation(Vec3::sAxisZ(), -DegreesToRadians(40.0f)), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);
		}

		// Create small bumps
		{
			BodyCreationSettings step(new BoxShape(Vec3(2.0f, 0.5f * cBumpHeight, 0.5f * cBumpWidth), 0.0f), Vec3::sZero(), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);
			for (int i = 0; i < 10; ++i)
			{
				step.mPosition = cBumpsPosition + Vec3(0, 0.5f * cBumpHeight, cBumpDelta * i);
				mBodyInterface->CreateAndAddBody(step, EActivation::DontActivate);
			}
		}

		// Create stairs
		{
			BodyCreationSettings step(new BoxShape(Vec3(2.0f, 0.5f * cStairsStepHeight, 0.5f * cStairsStepHeight)), Vec3::sZero(), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);
			for (int i = 0; i < 10; ++i)
			{
				step.mPosition = cStairsPosition + Vec3(0, cStairsStepHeight * (0.5f + i), cStairsStepHeight * i);
				mBodyInterface->CreateAndAddBody(step, EActivation::DontActivate);
			}
		}
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

void CharacterBaseTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	// Update scene time
	mTime += inParams.mDeltaTime;

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

	// Animate bodies
	if (!mRotatingBody.IsInvalid())
		mBodyInterface->MoveKinematic(mRotatingBody, cRotatingPosition, Quat::sRotation(Vec3::sAxisY(), JPH_PI * sin(mTime)), inParams.mDeltaTime);
	if (!mHorizontallyMovingBody.IsInvalid())
		mBodyInterface->MoveKinematic(mHorizontallyMovingBody, cHorizontallyMovingPosition + Vec3(3.0f * sin(mTime), 0, 0), cHorizontallyMovingOrientation, inParams.mDeltaTime);
	if (!mVerticallyMovingBody.IsInvalid())
		mBodyInterface->MoveKinematic(mVerticallyMovingBody, cVerticallyMovingPosition + Vec3(0, 1.75f * sin(mTime), 0), cVerticallyMovingOrientation, inParams.mDeltaTime);

	// Reset ramp blocks
	mRampBlocksTimeLeft -= inParams.mDeltaTime;
	if (mRampBlocksTimeLeft < 0.0f)
	{
		for (size_t i = 0; i < mRampBlocks.size(); ++i)
		{
			mBodyInterface->SetPositionAndRotation(mRampBlocks[i], cRampBlocksStart + float(i) * cRampBlocksDelta, cRampOrientation, EActivation::Activate);
			mBodyInterface->SetLinearAndAngularVelocity(mRampBlocks[i], Vec3::sZero(), Vec3::sZero());
		}
		mRampBlocksTimeLeft = cRampBlocksTime;
	}
}

void CharacterBaseTest::CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu)
{
	inUI->CreateTextButton(inSubMenu, "Select Scene", [this, inUI]() { 
		UIElement *scene_name = inUI->CreateMenu();
		for (uint i = 0; i < size(sScenes); ++i)
			inUI->CreateTextButton(scene_name, sScenes[i], [this, i]() { sSceneName = sScenes[i]; RestartTest(); });
		inUI->ShowMenu(scene_name);
	});
}

void CharacterBaseTest::GetInitialCamera(CameraState& ioState) const
{
	// This will become the local space offset, look down the x axis and slightly down
	ioState.mPos = Vec3::sZero();
	ioState.mForward = Vec3(10.0f, -2.0f, 0).Normalized();
}

Mat44 CharacterBaseTest::GetCameraPivot(float inCameraHeading, float inCameraPitch) const 
{
	// Pivot is center of character + distance behind based on the heading and pitch of the camera
	Vec3 fwd = Vec3(cos(inCameraPitch) * cos(inCameraHeading), sin(inCameraPitch), cos(inCameraPitch) * sin(inCameraHeading));
	return Mat44::sTranslation(GetCharacterPosition() + Vec3(0, cCharacterHeightStanding + cCharacterRadiusStanding, 0) - 5.0f * fwd);
}

void CharacterBaseTest::SaveState(StateRecorder &inStream) const
{
	inStream.Write(mTime);
	inStream.Write(mRampBlocksTimeLeft);
}

void CharacterBaseTest::RestoreState(StateRecorder &inStream)
{
	inStream.Read(mTime);
	inStream.Read(mRampBlocksTimeLeft);
}

void CharacterBaseTest::DrawCharacterState(const CharacterBase *inCharacter, Mat44Arg inCharacterTransform, Vec3Arg inCharacterVelocity)
{
	// Draw current location
	// Drawing prior to update since the physics system state is also that prior to the simulation step (so that all detected collisions etc. make sense)
	mDebugRenderer->DrawCoordinateSystem(inCharacterTransform);

	// Determine color
	CharacterBase::EGroundState ground_state = inCharacter->GetGroundState();
	Color color;
	switch (ground_state)
	{
	case CharacterBase::EGroundState::OnGround:
		color = Color::sGreen;
		break;
	case CharacterBase::EGroundState::Sliding:
		color = Color::sOrange;
		break;
	case CharacterBase::EGroundState::InAir:
	default:
		color = Color::sRed;
		break;
	}

	// Draw the state of the ground contact
	if (ground_state != CharacterBase::EGroundState::InAir)
	{
		Vec3 ground_position = inCharacter->GetGroundPosition();
		Vec3 ground_normal = inCharacter->GetGroundNormal();
		Vec3 ground_velocity = inCharacter->GetGroundVelocity();

		// Draw ground position
		mDebugRenderer->DrawWireSphere(ground_position, 0.1f, Color::sRed);
		mDebugRenderer->DrawArrow(ground_position, ground_position + 2.0f * ground_normal, Color::sGreen, 0.1f);

		// Draw ground velocity
		if (!ground_velocity.IsNearZero())
			mDebugRenderer->DrawArrow(ground_position, ground_position + ground_velocity, Color::sBlue, 0.1f);
	}

	// Draw provided character velocity
	if (!inCharacterVelocity.IsNearZero())
		mDebugRenderer->DrawArrow(inCharacterTransform.GetTranslation(), inCharacterTransform.GetTranslation() + inCharacterVelocity, Color::sYellow, 0.1f);

	// Draw text info
	const PhysicsMaterial *ground_material = inCharacter->GetGroundMaterial();
	mDebugRenderer->DrawText3D(inCharacterTransform.GetTranslation(), StringFormat("Mat: %s\nVel: %.1f m/s", ground_material->GetDebugName(), (double)inCharacterVelocity.Length()), color, 0.25f);
}
