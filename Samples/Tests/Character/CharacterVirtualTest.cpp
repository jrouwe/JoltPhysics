// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
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

void CharacterVirtualTest::Initialize()
{
	CharacterBaseTest::Initialize();

	// Create 'player' character
	Ref<CharacterVirtualSettings> settings = new CharacterVirtualSettings();
	settings->mMaxSlopeAngle = sMaxSlopeAngle;
	settings->mMaxStrength = sMaxStrength;
	settings->mShape = mStandingShape;
	settings->mBackFaceMode = sBackFaceMode;
	settings->mCharacterPadding = sCharacterPadding;
	settings->mPenetrationRecoverySpeed = sPenetrationRecoverySpeed;
	settings->mPredictiveContactDistance = sPredictiveContactDistance;
	settings->mSupportingVolume = Plane(Vec3::sAxisY(), -cCharacterRadiusStanding); // Accept contacts that touch the lower sphere of the capsule
	mCharacter = new CharacterVirtual(settings, RVec3::sZero(), Quat::sIdentity(), mPhysicsSystem);
	mCharacter->SetListener(this);
}

void CharacterVirtualTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	CharacterBaseTest::PrePhysicsUpdate(inParams);

	// Draw character pre update (the sim is also drawn pre update)
	RMat44 com = mCharacter->GetCenterOfMassTransform();
#ifdef JPH_DEBUG_RENDERER
	mCharacter->GetShape()->Draw(mDebugRenderer, com, Vec3::sReplicate(1.0f), Color::sGreen, false, true);
#endif // JPH_DEBUG_RENDERER

	// Draw shape including padding (only implemented for capsules right now)
	if (static_cast<const RotatedTranslatedShape *>(mCharacter->GetShape())->GetInnerShape()->GetSubType() == EShapeSubType::Capsule)
	{
		if (mCharacter->GetShape() == mStandingShape)
			mDebugRenderer->DrawCapsule(com, 0.5f * cCharacterHeightStanding, cCharacterRadiusStanding + mCharacter->GetCharacterPadding(), Color::sGrey, DebugRenderer::ECastShadow::Off, DebugRenderer::EDrawMode::Wireframe);
		else
			mDebugRenderer->DrawCapsule(com, 0.5f * cCharacterHeightCrouching, cCharacterRadiusCrouching + mCharacter->GetCharacterPadding(), Color::sGrey, DebugRenderer::ECastShadow::Off, DebugRenderer::EDrawMode::Wireframe);
	}

	// Remember old position
	RVec3 old_position = mCharacter->GetPosition();

	// Settings for our update function
	CharacterVirtual::ExtendedUpdateSettings update_settings;
	if (!sEnableStickToFloor)
		update_settings.mStickToFloorStepDown = Vec3::sZero();
	else
		update_settings.mStickToFloorStepDown = -mCharacter->GetUp() * update_settings.mStickToFloorStepDown.Length();
	if (!sEnableWalkStairs)
		update_settings.mWalkStairsStepUp = Vec3::sZero();
	else
		update_settings.mWalkStairsStepUp = mCharacter->GetUp() * update_settings.mWalkStairsStepUp.Length();

	// Update the character position
	mCharacter->ExtendedUpdate(inParams.mDeltaTime,
		-mCharacter->GetUp() * mPhysicsSystem->GetGravity().Length(),
		update_settings,
		mPhysicsSystem->GetDefaultBroadPhaseLayerFilter(Layers::MOVING),
		mPhysicsSystem->GetDefaultLayerFilter(Layers::MOVING),
		{ },
		{ },
		*mTempAllocator);

	// Calculate effective velocity
	RVec3 new_position = mCharacter->GetPosition();
	Vec3 velocity = Vec3(new_position - old_position) / inParams.mDeltaTime;

	// Draw state of character
	DrawCharacterState(mCharacter, mCharacter->GetWorldTransform(), velocity);

	// Draw labels on ramp blocks
	for (size_t i = 0; i < mRampBlocks.size(); ++i)
		mDebugRenderer->DrawText3D(mBodyInterface->GetPosition(mRampBlocks[i]), StringFormat("PushesPlayer: %s\nPushable: %s", (i & 1) != 0? "True" : "False", (i & 2) != 0? "True" : "False"), Color::sWhite, 0.25f);
}

void CharacterVirtualTest::HandleInput(Vec3Arg inMovementDirection, bool inJump, bool inSwitchStance, float inDeltaTime)
{
	bool player_controls_horizontal_velocity = sControlMovementDuringJump || mCharacter->IsSupported();
	if (player_controls_horizontal_velocity)
	{
		// Smooth the player input
		mDesiredVelocity = 0.25f * inMovementDirection * sCharacterSpeed + 0.75f * mDesiredVelocity;

		// True if the player intended to move
		mAllowSliding = !inMovementDirection.IsNearZero();
	}
	else
	{
		// While in air we allow sliding
		mAllowSliding = true;
	}

	// Update the character rotation and its up vector to match the up vector set by the user settings
	Quat character_up_rotation = Quat::sEulerAngles(Vec3(sUpRotationX, 0, sUpRotationZ));
	mCharacter->SetUp(character_up_rotation.RotateAxisY());
	mCharacter->SetRotation(character_up_rotation);

	// A cheaper way to update the character's ground velocity, 
	// the platforms that the character is standing on may have changed velocity
	mCharacter->UpdateGroundVelocity();

	// Determine new basic velocity
	Vec3 current_vertical_velocity = mCharacter->GetLinearVelocity().Dot(mCharacter->GetUp()) * mCharacter->GetUp();
	Vec3 ground_velocity = mCharacter->GetGroundVelocity();
	Vec3 new_velocity;
	if (mCharacter->GetGroundState() == CharacterVirtual::EGroundState::OnGround // If on ground
		&& (current_vertical_velocity.GetY() - ground_velocity.GetY()) < 0.1f) // And not moving away from ground
	{
		// Assume velocity of ground when on ground
		new_velocity = ground_velocity;

		// Jump
		if (inJump)
			new_velocity += sJumpSpeed * mCharacter->GetUp();
	}
	else
		new_velocity = current_vertical_velocity;

	// Gravity
	new_velocity += (character_up_rotation * mPhysicsSystem->GetGravity()) * inDeltaTime;

	if (player_controls_horizontal_velocity)
	{
		// Player input
		new_velocity += character_up_rotation * mDesiredVelocity;
	}
	else
	{
		// Preserve horizontal velocity
		Vec3 current_horizontal_velocity = mCharacter->GetLinearVelocity() - current_vertical_velocity;
		new_velocity += current_horizontal_velocity;
	}

	// Update character velocity
	mCharacter->SetLinearVelocity(new_velocity);

	// Stance switch
	if (inSwitchStance)
		mCharacter->SetShape(mCharacter->GetShape() == mStandingShape? mCrouchingShape : mStandingShape, 1.5f * mPhysicsSystem->GetPhysicsSettings().mPenetrationSlop, mPhysicsSystem->GetDefaultBroadPhaseLayerFilter(Layers::MOVING), mPhysicsSystem->GetDefaultLayerFilter(Layers::MOVING), { }, { }, *mTempAllocator);
}

void CharacterVirtualTest::AddConfigurationSettings(DebugUI *inUI, UIElement *inSubMenu)
{
	inUI->CreateComboBox(inSubMenu, "Back Face Mode", { "Ignore", "Collide" }, (int)sBackFaceMode, [=](int inItem) { sBackFaceMode = (EBackFaceMode)inItem; });
	inUI->CreateSlider(inSubMenu, "Up Rotation X (degrees)", RadiansToDegrees(sUpRotationX), -90.0f, 90.0f, 1.0f, [](float inValue) { sUpRotationX = DegreesToRadians(inValue); });
	inUI->CreateSlider(inSubMenu, "Up Rotation Z (degrees)", RadiansToDegrees(sUpRotationZ), -90.0f, 90.0f, 1.0f, [](float inValue) { sUpRotationZ = DegreesToRadians(inValue); });
	inUI->CreateSlider(inSubMenu, "Max Slope Angle (degrees)", RadiansToDegrees(sMaxSlopeAngle), 0.0f, 90.0f, 1.0f, [](float inValue) { sMaxSlopeAngle = DegreesToRadians(inValue); });
	inUI->CreateSlider(inSubMenu, "Max Strength (N)", sMaxStrength, 0.0f, 500.0f, 1.0f, [](float inValue) { sMaxStrength = inValue; });
	inUI->CreateSlider(inSubMenu, "Character Padding", sCharacterPadding, 0.01f, 0.5f, 0.01f, [](float inValue) { sCharacterPadding = inValue; });
	inUI->CreateSlider(inSubMenu, "Penetration Recovery Speed", sPenetrationRecoverySpeed, 0.0f, 1.0f, 0.05f, [](float inValue) { sPenetrationRecoverySpeed = inValue; });
	inUI->CreateSlider(inSubMenu, "Predictive Contact Distance", sPredictiveContactDistance, 0.01f, 1.0f, 0.01f, [](float inValue) { sPredictiveContactDistance = inValue; });
	inUI->CreateCheckBox(inSubMenu, "Enable Walk Stairs", sEnableWalkStairs, [](UICheckBox::EState inState) { sEnableWalkStairs = inState == UICheckBox::STATE_CHECKED; });
	inUI->CreateCheckBox(inSubMenu, "Enable Stick To Floor", sEnableStickToFloor, [](UICheckBox::EState inState) { sEnableStickToFloor = inState == UICheckBox::STATE_CHECKED; });
}

void CharacterVirtualTest::SaveState(StateRecorder &inStream) const
{
	CharacterBaseTest::SaveState(inStream);

	mCharacter->SaveState(inStream);

	bool is_standing = mCharacter->GetShape() == mStandingShape;
	inStream.Write(is_standing);

	inStream.Write(mAllowSliding);
	inStream.Write(mDesiredVelocity);
}

void CharacterVirtualTest::RestoreState(StateRecorder &inStream)
{
	CharacterBaseTest::RestoreState(inStream);

	mCharacter->RestoreState(inStream);

	bool is_standing = mCharacter->GetShape() == mStandingShape; // Initialize variable for validation mode
	inStream.Read(is_standing);
	mCharacter->SetShape(is_standing? mStandingShape : mCrouchingShape, FLT_MAX, { }, { }, { }, { }, *mTempAllocator);

	inStream.Read(mAllowSliding);
	inStream.Read(mDesiredVelocity);
}

void CharacterVirtualTest::OnAdjustBodyVelocity(const CharacterVirtual *inCharacter, const Body &inBody2, Vec3 &ioLinearVelocity, Vec3 &ioAngularVelocity)
{
	// Apply artificial velocity to the character when standing on the conveyor belt
	if (inBody2.GetID() == mConveyorBeltBody)
		ioLinearVelocity += Vec3(0, 0, 2);
}

void CharacterVirtualTest::OnContactAdded(const CharacterVirtual *inCharacter, const BodyID &inBodyID2, const SubShapeID &inSubShapeID2, RVec3Arg inContactPosition, Vec3Arg inContactNormal, CharacterContactSettings &ioSettings)
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

void CharacterVirtualTest::OnContactSolve(const CharacterVirtual *inCharacter, const BodyID &inBodyID2, const SubShapeID &inSubShapeID2, RVec3Arg inContactPosition, Vec3Arg inContactNormal, Vec3Arg inContactVelocity, const PhysicsMaterial *inContactMaterial, Vec3Arg inCharacterVelocity, Vec3 &ioNewCharacterVelocity)
{
	// Don't allow the player to slide down static not-too-steep surfaces when not actively moving and when not on a moving platform
	if (!mAllowSliding && inContactVelocity.IsNearZero() && !inCharacter->IsSlopeTooSteep(inContactNormal))
		ioNewCharacterVelocity = Vec3::sZero();
}
