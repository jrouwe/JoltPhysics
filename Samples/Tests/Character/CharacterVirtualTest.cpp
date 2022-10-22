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

static const Vec3 cStickToFloorStepDown(0, -0.5f, 0);
static const Vec3 cWalkStairsStepUp = Vec3(0.0f, 0.4f, 0.0f);
static const float cWalkStairsMinStepForward = 0.02f;
static const float cWalkStairsStepForwardTest = 0.15f;
static const Vec3 cWalkStairsStepDownExtra = Vec3::sZero();

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
	settings->mSupportingVolume = Plane(Vec3::sAxisY(), -cCharacterRadiusStanding); // Accept contacts that touch the lower sphere of the capsule
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

	// Update the character position
	mCharacter->ExtendedUpdate(inParams.mDeltaTime,
		mPhysicsSystem->GetGravity(),
		sEnableStickToFloor? cStickToFloorStepDown : Vec3::sZero(),
		sEnableWalkStairs? cWalkStairsStepUp : Vec3::sZero(),
		cWalkStairsMinStepForward,
		cWalkStairsStepForwardTest,
		cWalkStairsStepDownExtra,
		mPhysicsSystem->GetDefaultBroadPhaseLayerFilter(Layers::MOVING),
		mPhysicsSystem->GetDefaultLayerFilter(Layers::MOVING),
		{ },
		*mTempAllocator);

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
	// Smooth the player input
	mDesiredVelocity = 0.25f * inMovementDirection * cCharacterSpeed + 0.75f * mDesiredVelocity;

	// True if the player intended to move
	mAllowSliding = !inMovementDirection.IsNearZero();

	// Determine new basic velocity
	Vec3 current_vertical_velocity = Vec3(0, mCharacter->GetLinearVelocity().GetY(), 0);
	Vec3 ground_velocity = mCharacter->GetGroundVelocity();
	Vec3 new_velocity;
	if (mCharacter->GetGroundState() == CharacterVirtual::EGroundState::OnGround // If on ground
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
	new_velocity += mDesiredVelocity;

	// Update character velocity
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
		inUI->CreateCheckBox(configuration_settings, "Enable Stick To Floor", sEnableStickToFloor, [](UICheckBox::EState inState) { sEnableStickToFloor = inState == UICheckBox::STATE_CHECKED; });
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

	inStream.Write(mDesiredVelocity);
}

void CharacterVirtualTest::RestoreState(StateRecorder &inStream)
{
	CharacterBaseTest::RestoreState(inStream);

	mCharacter->RestoreState(inStream);

	bool is_standing = mCharacter->GetShape() == mStandingShape; // Initialize variable for validation mode
	inStream.Read(is_standing);
	mCharacter->SetShape(is_standing? mStandingShape : mCrouchingShape, FLT_MAX, { }, { }, { }, *mTempAllocator);

	inStream.Read(mDesiredVelocity);
}

void CharacterVirtualTest::OnContactAdded(const CharacterVirtual *inCharacter, const BodyID &inBodyID2, const SubShapeID &inSubShapeID2, Vec3Arg inContactPosition, Vec3Arg inContactNormal, CharacterContactSettings &ioSettings)
{
	// Dynamic boxes on the ramp go through all permutations
	Array<BodyID>::const_iterator i = find(mRampBlocks.begin(), mRampBlocks.end(), inBodyID2);
	if (i != mRampBlocks.end())
	{
		size_t index = i - mRampBlocks.begin();
		ioSettings.mCanPushCharacter = (index & 1) != 0;
		ioSettings.mCanReceiveImpulses = (index & 2) != 0;
	}

	// If we encounter an object that can push us, enable sliding
	if (ioSettings.mCanPushCharacter && mPhysicsSystem->GetBodyInterface().GetMotionType(inBodyID2) != EMotionType::Static)
		mAllowSliding = true;
}

void CharacterVirtualTest::OnContactSolve(const CharacterVirtual *inCharacter, const BodyID &inBodyID2, const SubShapeID &inSubShapeID2, Vec3Arg inContactPosition, Vec3Arg inContactNormal, Vec3Arg inContactVelocity, const PhysicsMaterial *inContactMaterial, Vec3Arg inCharacterVelocity, Vec3 &ioNewCharacterVelocity)
{
	// Don't allow the player to slide down static not-too-steep surfaces when not actively moving and when not on a moving platform
	if (!mAllowSliding && inContactVelocity.IsNearZero() && !inCharacter->IsSlopeTooSteep(inContactNormal))
		ioNewCharacterVelocity = Vec3::sZero();
}
