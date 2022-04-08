// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Character/CharacterVirtualTest.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Layers.h>
#include <Renderer/DebugRendererImp.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(CharacterVirtualTest) 
{ 
	JPH_ADD_BASE_CLASS(CharacterVirtualTest, CharacterTestBase)
}

void CharacterVirtualTest::Initialize()
{
	CharacterTestBase::Initialize();

	// Create 'player' character
	Ref<CharacterVirtualSettings> settings = new CharacterVirtualSettings();
	settings->mMaxSlopeAngle = DegreesToRadians(45.0f);
	settings->mShape = mStandingShape;
	mCharacter = new CharacterVirtual(settings, Vec3::sZero(), Quat::sIdentity(), mPhysicsSystem);
	mCharacter->SetListener(this);
}

void CharacterVirtualTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	CharacterTestBase::PrePhysicsUpdate(inParams);

	// Remember old position
	Vec3 old_position = mCharacter->GetPosition();

	// Update the character position (instant, do not have to wait for physics update)
	mCharacter->Update(inParams.mDeltaTime, mPhysicsSystem->GetGravity(), mPhysicsSystem->GetDefaultBroadPhaseLayerFilter(Layers::MOVING), mPhysicsSystem->GetDefaultLayerFilter(Layers::MOVING), { });

	// Calculate effective velocity
	Vec3 new_position = mCharacter->GetPosition();
	float velocity = (new_position - old_position).Length() / inParams.mDeltaTime;

	// Draw character
	if (mCharacter->GetShape() == mStandingShape)
		mDebugRenderer->DrawCapsule(mCharacter->GetCenterOfMassTransform(), 0.5f * cCharacterHeightStanding, cCharacterRadiusStanding, Color::sGreen, DebugRenderer::ECastShadow::Off, DebugRenderer::EDrawMode::Wireframe);
	else
		mDebugRenderer->DrawCapsule(mCharacter->GetCenterOfMassTransform(), 0.5f * cCharacterHeightCrouching, cCharacterRadiusCrouching, Color::sGreen, DebugRenderer::ECastShadow::Off, DebugRenderer::EDrawMode::Wireframe);

	// Draw state of character
	DrawCharacterState(mCharacter, mCharacter->GetWorldTransform(), velocity);
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
		mCharacter->SetShape(mCharacter->GetShape() == mStandingShape? mCrouchingShape : mStandingShape, 1.5f * mPhysicsSystem->GetPhysicsSettings().mPenetrationSlop, mPhysicsSystem->GetDefaultBroadPhaseLayerFilter(Layers::MOVING), mPhysicsSystem->GetDefaultLayerFilter(Layers::MOVING), { });
}

void CharacterVirtualTest::OnContactAdded(const CharacterVirtual *inCharacter, const BodyID &inBodyID2, const SubShapeID &inSubShapeID2, Vec3Arg inContactPosition, Vec3Arg inContactNormal, CharacterContactSettings &ioSettings)
{
	// Dynamic boxes cannot push the character (they're meant to be pushed by the player)
	if (find(mDynamicBoxes.begin(), mDynamicBoxes.end(), inBodyID2) != mDynamicBoxes.end())
		ioSettings.mCanPushCharacter = false;
}
