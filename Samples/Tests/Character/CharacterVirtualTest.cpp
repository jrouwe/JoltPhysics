// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Character/CharacterVirtualTest.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(CharacterVirtualTest) 
{ 
	JPH_ADD_BASE_CLASS(CharacterVirtualTest, CharacterTestBase)
}

void CharacterVirtualTest::Initialize()
{
	CharacterTestBase::Initialize();

	// Create 'player' character
	Ref<CharacterVirtualSettings> settings = new CharacterVirtualSettings();
	settings->mShape = mStandingShape;
	mCharacter = new CharacterVirtual(settings, Vec3::sZero(), Quat::sIdentity(), mPhysicsSystem);
}

void CharacterVirtualTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	// Get the state of the character
	CharacterVirtual::EGroundState ground_state = mCharacter->GetGroundState();

	// Determine controller input
	Vec3 control_input = Vec3::sZero();
	if (inParams.mKeyboard->IsKeyPressed(DIK_LEFT))		control_input.SetX(-1);
	if (inParams.mKeyboard->IsKeyPressed(DIK_RIGHT))	control_input.SetX(1);
	if (inParams.mKeyboard->IsKeyPressed(DIK_UP))		control_input.SetZ(-1);
	if (inParams.mKeyboard->IsKeyPressed(DIK_DOWN))		control_input.SetZ(1);
	if (control_input != Vec3::sZero())
		control_input = control_input.Normalized();

	// Cancel movement in opposite direction of normal when sliding
	if (ground_state == CharacterVirtual::EGroundState::Sliding)
	{
		Vec3 normal = mCharacter->GetGroundNormal();
		normal.SetY(0);
		if (normal.Dot(control_input) <= 0.0f)
			control_input = Vec3::sZero();
	}

	// Update velocity
	Vec3 current_velocity = mCharacter->GetLinearVelocity();
	Vec3 desired_velocity = cCharacterSpeed * control_input;
	desired_velocity.SetY(current_velocity.GetY());
	Vec3 new_velocity = 0.75f * current_velocity + 0.25f * desired_velocity;

	// Apply gravity only if we're not on solid ground (otherwise we'll slowly slide as there is no friction)
	if (ground_state != CharacterVirtual::EGroundState::OnGround)
		new_velocity += mPhysicsSystem->GetGravity() * inParams.mDeltaTime;

	// Check actions
	for (int key = inParams.mKeyboard->GetFirstKey(); key != 0; key = inParams.mKeyboard->GetNextKey())
	{
		if (key == DIK_RETURN)
		{
			// Stance switch
			mCharacter->SetShape(mCharacter->GetShape() == mStandingShape? mCrouchingShape : mStandingShape, 1.5f * mPhysicsSystem->GetPhysicsSettings().mPenetrationSlop, mPhysicsSystem->GetDefaultBroadPhaseLayerFilter(Layers::MOVING), mPhysicsSystem->GetDefaultLayerFilter(Layers::MOVING), { });
			break;
		}
		else if (key == DIK_J)
		{
			// Jump
			if (ground_state == CharacterVirtual::EGroundState::OnGround)
				new_velocity += Vec3(0, cJumpSpeed, 0);
		}
	}

	// Update the velocity
	mCharacter->SetLinearVelocity(new_velocity);

	// Update the character position (instant, do not have to wait for physics update)
	mCharacter->Update(inParams.mDeltaTime, mPhysicsSystem->GetGravity(), mPhysicsSystem->GetDefaultBroadPhaseLayerFilter(Layers::MOVING), mPhysicsSystem->GetDefaultLayerFilter(Layers::MOVING), { });

	// Draw the character capsule
	mCharacter->GetShape()->Draw(DebugRenderer::sInstance, mCharacter->GetCenterOfMassTransform(), Vec3::sReplicate(1.0f), Color::sRed, false, true);

	// Draw current location
	// Drawing prior to update since the physics system state is also that prior to the simulation step (so that all detected collisions etc. make sense)
	mDebugRenderer->DrawCoordinateSystem(mCharacter->GetWorldTransform());

	// Get the state of the character again (may have changed)
	ground_state = mCharacter->GetGroundState();
	if (ground_state != CharacterVirtual::EGroundState::InAir)
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

Mat44 CharacterVirtualTest::GetCameraPivot(float inCameraHeading, float inCameraPitch) const 
{
	return mCharacter->GetWorldTransform();
}
