// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Character/CharacterVirtualTest.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Layers.h>
#include <Renderer/DebugRendererImp.h>
#include <Application/DebugUI.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(CharacterVirtualTest) 
{ 
	JPH_ADD_BASE_CLASS(CharacterVirtualTest, CharacterBaseTest)
}

static const Vec3 cStepUpHeight = Vec3(0.0f, 0.4f, 0.0f);
static const float cMinStepForward = 0.15f;

void CharacterVirtualTest::Initialize()
{
	CharacterBaseTest::Initialize();

	// Create 'player' character
	Ref<CharacterVirtualSettings> settings = new CharacterVirtualSettings();
	settings->mMaxSlopeAngle = sMaxSlopeAngle;
	settings->mMaxStrength = sMaxStrength;
	settings->mShape = mStandingShape;
	settings->mCharacterPadding = sCharacterPadding;
	settings->mPenetrationRecoverySpeed = sPenetrationRecoverySpeed;
	settings->mPredictiveContactDistance = sPredictiveContactDistance;
	mCharacter = new CharacterVirtual(settings, Vec3::sZero(), Quat::sIdentity(), mPhysicsSystem);
	mCharacter->SetListener(this);
}

void CharacterVirtualTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	CharacterBaseTest::PrePhysicsUpdate(inParams);

	// Draw character pre update (the sim is also drawn pre update)
	Mat44 com = mCharacter->GetCenterOfMassTransform();
	if (mCharacter->GetShape() == mStandingShape)
	{
		mDebugRenderer->DrawCapsule(com, 0.5f * cCharacterHeightStanding, cCharacterRadiusStanding, Color::sGreen, DebugRenderer::ECastShadow::Off, DebugRenderer::EDrawMode::Wireframe);
		mDebugRenderer->DrawCapsule(com, 0.5f * cCharacterHeightStanding, cCharacterRadiusStanding + mCharacter->GetCharacterPadding(), Color::sGrey, DebugRenderer::ECastShadow::Off, DebugRenderer::EDrawMode::Wireframe);
	}
	else
	{
		mDebugRenderer->DrawCapsule(com, 0.5f * cCharacterHeightCrouching, cCharacterRadiusCrouching, Color::sGreen, DebugRenderer::ECastShadow::Off, DebugRenderer::EDrawMode::Wireframe);
		mDebugRenderer->DrawCapsule(com, 0.5f * cCharacterHeightCrouching, cCharacterRadiusCrouching + mCharacter->GetCharacterPadding(), Color::sGrey, DebugRenderer::ECastShadow::Off, DebugRenderer::EDrawMode::Wireframe);
	}

	// Remember old position
	Vec3 old_position = mCharacter->GetPosition();

	// Update the character position (instant, do not have to wait for physics update)
	mCharacter->Update(inParams.mDeltaTime, mPhysicsSystem->GetGravity(), mPhysicsSystem->GetDefaultBroadPhaseLayerFilter(Layers::MOVING), mPhysicsSystem->GetDefaultLayerFilter(Layers::MOVING), { }, *mTempAllocator);

	// Allow user to turn off walk stairs algorithm
	if (sEnableWalkStairs)
	{
		// Calculate how much we wanted to move horizontally
		Vec3 desired_horizontal_step = mCharacter->GetLinearVelocity() * inParams.mDeltaTime;
		desired_horizontal_step.SetY(0);
		float desired_horizontal_step_len = desired_horizontal_step.Length();

		// Calculate how much we moved horizontally
		Vec3 achieved_horizontal_step = mCharacter->GetPosition() - old_position;
		achieved_horizontal_step.SetY(0);
		float achieved_horizontal_step_len = achieved_horizontal_step.Length();

		// If we didn't move as far as we wanted and we're against a slope that's too steep
		if (achieved_horizontal_step_len + 1.0e-4f < desired_horizontal_step_len
			&& mCharacter->CanWalkStairs())
		{
			// Calculate how much we should step forward
			Vec3 step_forward_normalized = desired_horizontal_step / desired_horizontal_step_len;
			Vec3 step_forward = step_forward_normalized * (desired_horizontal_step_len - achieved_horizontal_step_len);

			// Calculate how far to scan ahead for a floor
			Vec3 step_forward_test = step_forward_normalized * cMinStepForward;

			mCharacter->WalkStairs(inParams.mDeltaTime, mPhysicsSystem->GetGravity(), cStepUpHeight, step_forward, step_forward_test, Vec3::sZero(), mPhysicsSystem->GetDefaultBroadPhaseLayerFilter(Layers::MOVING), mPhysicsSystem->GetDefaultLayerFilter(Layers::MOVING), { }, *mTempAllocator);
		}
	}

	// Calculate effective velocity
	Vec3 new_position = mCharacter->GetPosition();
	Vec3 velocity = (new_position - old_position) / inParams.mDeltaTime;

	// Draw state of character
	DrawCharacterState(mCharacter, mCharacter->GetWorldTransform(), velocity);

	// Draw labels on ramp blocks
	for (size_t i = 0; i < mRampBlocks.size(); ++i)
		mDebugRenderer->DrawText3D(mBodyInterface->GetPosition(mRampBlocks[i]), StringFormat("PushesPlayer: %s\nPushable: %s", (i & 1) != 0? "True" : "False", (i & 2) != 0? "True" : "False"), Color::sWhite, 0.25f);
}

void CharacterVirtualTest::HandleInput(Vec3Arg inMovementDirection, bool inJump, bool inSwitchStance, float inDeltaTime)
{
	// Cancel movement in opposite direction of normal when sliding
	CharacterVirtual::EGroundState ground_state = mCharacter->GetGroundState();
	if (ground_state == CharacterVirtual::EGroundState::Sliding)
	{
		Vec3 normal = mCharacter->GetGroundNormal();
		normal.SetY(0.0f);
		float dot = normal.Dot(inMovementDirection);
		if (dot < 0.0f)
			inMovementDirection -= (dot * normal) / normal.LengthSq();
	}

	// Smooth the player input
	mSmoothMovementDirection = 0.25f * inMovementDirection + 0.75f * mSmoothMovementDirection;

	Vec3 current_vertical_velocity = Vec3(0, mCharacter->GetLinearVelocity().GetY(), 0);

	Vec3 ground_velocity = mCharacter->GetGroundVelocity();

	Vec3 new_velocity;
	if (ground_state == CharacterVirtual::EGroundState::OnGround // If on ground
		&& (current_vertical_velocity.GetY() - ground_velocity.GetY()) < 0.1f) // And not moving away from ground
	{
		// Assume velocity of ground when on ground
		new_velocity = ground_velocity;
		
		// Jump
		if (inJump)
			new_velocity += Vec3(0, cJumpSpeed, 0);
	}
	else
		new_velocity = current_vertical_velocity;

	// Gravity
	new_velocity += mPhysicsSystem->GetGravity() * inDeltaTime;

	// Player input
	new_velocity += mSmoothMovementDirection * cCharacterSpeed;

	// Update the velocity
	mCharacter->SetLinearVelocity(new_velocity);

	// Stance switch
	if (inSwitchStance)
		mCharacter->SetShape(mCharacter->GetShape() == mStandingShape? mCrouchingShape : mStandingShape, 1.5f * mPhysicsSystem->GetPhysicsSettings().mPenetrationSlop, mPhysicsSystem->GetDefaultBroadPhaseLayerFilter(Layers::MOVING), mPhysicsSystem->GetDefaultLayerFilter(Layers::MOVING), { }, *mTempAllocator);
}

void CharacterVirtualTest::CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu)
{
	CharacterBaseTest::CreateSettingsMenu(inUI, inSubMenu);

	inUI->CreateTextButton(inSubMenu, "Configuration Settings", [=]() {
		UIElement *configuration_settings = inUI->CreateMenu();
		
		inUI->CreateSlider(configuration_settings, "Max Slope Angle (degrees)", RadiansToDegrees(sMaxSlopeAngle), 0.0f, 90.0f, 1.0f, [](float inValue) { sMaxSlopeAngle = DegreesToRadians(inValue); });
		inUI->CreateSlider(configuration_settings, "Max Strength (N)", sMaxStrength, 0.0f, 500.0f, 1.0f, [](float inValue) { sMaxStrength = inValue; });
		inUI->CreateSlider(configuration_settings, "Character Padding", sCharacterPadding, 0.01f, 0.5f, 0.01f, [](float inValue) { sCharacterPadding = inValue; });
		inUI->CreateSlider(configuration_settings, "Penetration Recovery Speed", sPenetrationRecoverySpeed, 0.0f, 1.0f, 0.05f, [](float inValue) { sPenetrationRecoverySpeed = inValue; });
		inUI->CreateSlider(configuration_settings, "Predictive Contact Distance", sPredictiveContactDistance, 0.01f, 1.0f, 0.01f, [](float inValue) { sPredictiveContactDistance = inValue; });
		inUI->CreateCheckBox(configuration_settings, "Enable Walk Stairs", sEnableWalkStairs, [](UICheckBox::EState inState) { sEnableWalkStairs = inState == UICheckBox::STATE_CHECKED; });
		inUI->CreateTextButton(configuration_settings, "Accept Changes", [=]() { RestartTest(); });
		inUI->ShowMenu(configuration_settings);
	});
}

void CharacterVirtualTest::SaveState(StateRecorder &inStream) const
{
	CharacterBaseTest::SaveState(inStream);

	mCharacter->SaveState(inStream);

	bool is_standing = mCharacter->GetShape() == mStandingShape;
	inStream.Write(is_standing);

	inStream.Write(mSmoothMovementDirection);
}

void CharacterVirtualTest::RestoreState(StateRecorder &inStream)
{
	CharacterBaseTest::RestoreState(inStream);

	mCharacter->RestoreState(inStream);

	bool is_standing = mCharacter->GetShape() == mStandingShape; // Initialize variable for validation mode
	inStream.Read(is_standing);
	mCharacter->SetShape(is_standing? mStandingShape : mCrouchingShape, FLT_MAX, { }, { }, { }, *mTempAllocator);

	inStream.Read(mSmoothMovementDirection);
}

void CharacterVirtualTest::OnContactAdded(const CharacterVirtual *inCharacter, const BodyID &inBodyID2, const SubShapeID &inSubShapeID2, Vec3Arg inContactPosition, Vec3Arg inContactNormal, CharacterContactSettings &ioSettings)
{
	// Dynamic boxes on the ramp go through all permutations
	vector<BodyID>::const_iterator i = find(mRampBlocks.begin(), mRampBlocks.end(), inBodyID2);
	if (i != mRampBlocks.end())
	{
		size_t index = i - mRampBlocks.begin();
		ioSettings.mCanPushCharacter = (index & 1) != 0;
		ioSettings.mCanReceiveImpulses = (index & 2) != 0;
	}
}
