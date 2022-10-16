// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Character/CharacterTest.h>
#include <Layers.h>
#include <Renderer/DebugRendererImp.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(CharacterTest) 
{ 
	JPH_ADD_BASE_CLASS(CharacterTest, CharacterBaseTest)
}

static const float cCollisionTolerance = 0.05f;

CharacterTest::~CharacterTest()
{
	mCharacter->RemoveFromPhysicsSystem();
}

void CharacterTest::Initialize()
{
	CharacterBaseTest::Initialize();

	// Create 'player' character
	Ref<CharacterSettings> settings = new CharacterSettings();
	settings->mMaxSlopeAngle = DegreesToRadians(45.0f);
	settings->mLayer = Layers::MOVING;
	settings->mShape = mStandingShape;
	settings->mFriction = 0.5f;
	settings->mSupportingVolume = Plane(Vec3::sAxisY(), -cCharacterRadiusStanding); // Accept contacts that touch the lower sphere of the capsule
	mCharacter = new Character(settings, Vec3::sZero(), Quat::sIdentity(), 0, mPhysicsSystem);
	mCharacter->AddToPhysicsSystem(EActivation::Activate);
}

void CharacterTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	CharacterBaseTest::PrePhysicsUpdate(inParams);

	// Draw state of character
	DrawCharacterState(mCharacter, mCharacter->GetWorldTransform(), mCharacter->GetLinearVelocity());
}
void CharacterTest::PostPhysicsUpdate(float inDeltaTime)
{
	// Fetch the new ground properties
	mCharacter->PostSimulation(cCollisionTolerance);
}

void CharacterTest::SaveState(StateRecorder &inStream) const
{
	CharacterBaseTest::SaveState(inStream);

	mCharacter->SaveState(inStream);

	bool is_standing = mCharacter->GetShape() == mStandingShape;
	inStream.Write(is_standing);
}

void CharacterTest::RestoreState(StateRecorder &inStream)
{
	CharacterBaseTest::RestoreState(inStream);

	mCharacter->RestoreState(inStream);

	bool is_standing = mCharacter->GetShape() == mStandingShape; // Initialize variable for validation mode
	inStream.Read(is_standing);
	mCharacter->SetShape(is_standing? mStandingShape : mCrouchingShape, FLT_MAX);
}

void CharacterTest::HandleInput(Vec3Arg inMovementDirection, bool inJump, bool inSwitchStance, float inDeltaTime)
{
	// Cancel movement in opposite direction of normal when touching something we can't walk up
	Character::EGroundState ground_state = mCharacter->GetGroundState();
	if (ground_state == Character::EGroundState::OnSteepGround
		|| ground_state == Character::EGroundState::NotSupported)
	{
		Vec3 normal = mCharacter->GetGroundNormal();
		normal.SetY(0.0f);
		float dot = normal.Dot(inMovementDirection);
		if (dot < 0.0f)
			inMovementDirection -= (dot * normal) / normal.LengthSq();
	}

	// Update velocity
	Vec3 current_velocity = mCharacter->GetLinearVelocity();
	Vec3 desired_velocity = cCharacterSpeed * inMovementDirection;
	desired_velocity.SetY(current_velocity.GetY());
	Vec3 new_velocity = 0.75f * current_velocity + 0.25f * desired_velocity;

	// Stance switch
	if (inSwitchStance)
		mCharacter->SetShape(mCharacter->GetShape() == mStandingShape? mCrouchingShape : mStandingShape, 1.5f * mPhysicsSystem->GetPhysicsSettings().mPenetrationSlop);

	// Jump
	if (inJump && ground_state == Character::EGroundState::OnGround)
		new_velocity += Vec3(0, cJumpSpeed, 0);

	// Update the velocity
	mCharacter->SetLinearVelocity(new_velocity);
}
