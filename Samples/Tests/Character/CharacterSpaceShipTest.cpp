// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Character/CharacterSpaceShipTest.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>
#include <Jolt/Physics/Collision/Shape/CylinderShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(CharacterSpaceShipTest)
{
	JPH_ADD_BASE_CLASS(CharacterSpaceShipTest, Test)
}

void CharacterSpaceShipTest::Initialize()
{
	// Dimensions of our space ship
	constexpr float cSpaceShipHeight = 2.0f;
	constexpr float cSpaceShipRingHeight = 0.2f;
	constexpr float cSpaceShipRadius = 100.0f;
	const RVec3 cShipInitialPosition(-25, 15, 0);

	// Create floor for reference
	CreateFloor();

	// Create 'player' character
	Ref<CharacterVirtualSettings> settings = new CharacterVirtualSettings();
	settings->mShape = RotatedTranslatedShapeSettings(Vec3(0, 0.5f * cCharacterHeightStanding + cCharacterRadiusStanding, 0), Quat::sIdentity(), new CapsuleShape(0.5f * cCharacterHeightStanding, cCharacterRadiusStanding)).Create().Get();
	settings->mSupportingVolume = Plane(Vec3::sAxisY(), -cCharacterRadiusStanding); // Accept contacts that touch the lower sphere of the capsule
	mCharacter = new CharacterVirtual(settings, cShipInitialPosition + Vec3(0, cSpaceShipHeight, 0), Quat::sIdentity(), 0, mPhysicsSystem);
	mCharacter->SetListener(this);

	// Create the space ship
	StaticCompoundShapeSettings compound;
	compound.SetEmbedded();
	for (float h = cSpaceShipRingHeight; h < cSpaceShipHeight; h += cSpaceShipRingHeight)
		compound.AddShape(Vec3::sZero(), Quat::sIdentity(), new CylinderShape(h, sqrt(Square(cSpaceShipRadius) - Square(cSpaceShipRadius - cSpaceShipHeight - cSpaceShipRingHeight + h))));
	mSpaceShip = mBodyInterface->CreateAndAddBody(BodyCreationSettings(&compound, cShipInitialPosition, Quat::sIdentity(), EMotionType::Kinematic, Layers::MOVING), EActivation::Activate);
	mSpaceShipPrevTransform = mBodyInterface->GetCenterOfMassTransform(mSpaceShip);
}

void CharacterSpaceShipTest::ProcessInput(const ProcessInputParams &inParams)
{
	// Determine controller input
	Vec3 control_input = Vec3::sZero();
	if (inParams.mKeyboard->IsKeyPressed(DIK_LEFT))		control_input.SetZ(-1);
	if (inParams.mKeyboard->IsKeyPressed(DIK_RIGHT))	control_input.SetZ(1);
	if (inParams.mKeyboard->IsKeyPressed(DIK_UP))		control_input.SetX(1);
	if (inParams.mKeyboard->IsKeyPressed(DIK_DOWN))		control_input.SetX(-1);
	if (control_input != Vec3::sZero())
		control_input = control_input.Normalized();

	// Calculate the desired velocity in local space to the ship based on the camera forward
	RMat44 new_space_ship_transform = mBodyInterface->GetCenterOfMassTransform(mSpaceShip);
	Vec3 cam_fwd = new_space_ship_transform.GetRotation().Multiply3x3Transposed(inParams.mCameraState.mForward);
	cam_fwd.SetY(0.0f);
	cam_fwd = cam_fwd.NormalizedOr(Vec3::sAxisX());
	Quat rotation = Quat::sFromTo(Vec3::sAxisX(), cam_fwd);
	control_input = rotation * control_input;

	// Smooth the player input in local space to the ship
	mDesiredVelocity = 0.25f * control_input * cCharacterSpeed + 0.75f * mDesiredVelocity;

	// Check actions
	mJump = inParams.mKeyboard->IsKeyPressedAndTriggered(DIK_RCONTROL, mWasJump);
}

void CharacterSpaceShipTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	// Update scene time
	mTime += inParams.mDeltaTime;

	// Update the character so it stays relative to the space ship
	RMat44 new_space_ship_transform = mBodyInterface->GetCenterOfMassTransform(mSpaceShip);
	mCharacter->SetPosition(new_space_ship_transform * mSpaceShipPrevTransform.Inversed() * mCharacter->GetPosition());

	// Update the character rotation and its up vector to match the new up vector of the ship
	mCharacter->SetUp(new_space_ship_transform.GetAxisY());
	mCharacter->SetRotation(new_space_ship_transform.GetQuaternion());

	// Draw character pre update (the sim is also drawn pre update)
	// Note that we have first updated the position so that it matches the new position of the ship
#ifdef JPH_DEBUG_RENDERER
	mCharacter->GetShape()->Draw(mDebugRenderer, mCharacter->GetCenterOfMassTransform(), Vec3::sReplicate(1.0f), Color::sGreen, false, true);
#endif // JPH_DEBUG_RENDERER

	// Determine new character velocity
	Vec3 current_vertical_velocity = mCharacter->GetLinearVelocity().Dot(mSpaceShipPrevTransform.GetAxisY()) * mCharacter->GetUp();
	Vec3 ground_velocity = mCharacter->GetGroundVelocity();
	Vec3 new_velocity;
	if (mCharacter->GetGroundState() == CharacterVirtual::EGroundState::OnGround // If on ground
		&& (current_vertical_velocity.GetY() - ground_velocity.GetY()) < 0.1f) // And not moving away from ground
	{
		// Assume velocity of ground when on ground
		new_velocity = ground_velocity;

		// Jump
		if (mJump)
			new_velocity += cJumpSpeed * mCharacter->GetUp();
	}
	else
		new_velocity = current_vertical_velocity;

	// Gravity always acts relative to the ship
	Vec3 gravity = new_space_ship_transform.Multiply3x3(mPhysicsSystem->GetGravity());
	new_velocity += gravity * inParams.mDeltaTime;

	// Transform player input to world space
	new_velocity += new_space_ship_transform.Multiply3x3(mDesiredVelocity);

	// Update character velocity
	mCharacter->SetLinearVelocity(new_velocity);

	// Update the character position
	CharacterVirtual::ExtendedUpdateSettings update_settings;
	mCharacter->ExtendedUpdate(inParams.mDeltaTime,
		gravity,
		update_settings,
		mPhysicsSystem->GetDefaultBroadPhaseLayerFilter(Layers::MOVING),
		mPhysicsSystem->GetDefaultLayerFilter(Layers::MOVING),
		{ },
		{ },
		*mTempAllocator);

	// Update previous transform
	mSpaceShipPrevTransform = new_space_ship_transform;

	// Calculate new velocity
	UpdateShipVelocity();
}

void CharacterSpaceShipTest::UpdateShipVelocity()
{
	// Make it a rocky ride...
	mSpaceShipLinearVelocity = Vec3(Sin(mTime), 0, Cos(mTime)) * 50.0f;
	mSpaceShipAngularVelocity = Vec3(Sin(2.0f * mTime), 1, Cos(2.0f * mTime)) * 0.5f;

	mBodyInterface->SetLinearAndAngularVelocity(mSpaceShip, mSpaceShipLinearVelocity, mSpaceShipAngularVelocity);
}

void CharacterSpaceShipTest::GetInitialCamera(CameraState& ioState) const
{
	// This will become the local space offset, look down the x axis and slightly down
	ioState.mPos = RVec3::sZero();
	ioState.mForward = Vec3(10.0f, -2.0f, 0).Normalized();
}

RMat44 CharacterSpaceShipTest::GetCameraPivot(float inCameraHeading, float inCameraPitch) const
{
	// Pivot is center of character + distance behind based on the heading and pitch of the camera
	Vec3 fwd = Vec3(Cos(inCameraPitch) * Cos(inCameraHeading), Sin(inCameraPitch), Cos(inCameraPitch) * Sin(inCameraHeading));
	return RMat44::sTranslation(mCharacter->GetPosition() + Vec3(0, cCharacterHeightStanding + cCharacterRadiusStanding, 0) - 5.0f * fwd);
}

void CharacterSpaceShipTest::SaveState(StateRecorder &inStream) const
{
	mCharacter->SaveState(inStream);

	inStream.Write(mTime);
	inStream.Write(mSpaceShipPrevTransform);
}

void CharacterSpaceShipTest::RestoreState(StateRecorder &inStream)
{
	mCharacter->RestoreState(inStream);

	inStream.Read(mTime);
	inStream.Read(mSpaceShipPrevTransform);

	// Calculate new velocity
	UpdateShipVelocity();
}

void CharacterSpaceShipTest::SaveInputState(StateRecorder &inStream) const
{
	inStream.Write(mDesiredVelocity);
	inStream.Write(mJump);
}

void CharacterSpaceShipTest::RestoreInputState(StateRecorder &inStream)
{
	inStream.Read(mDesiredVelocity);
	inStream.Read(mJump);
}

void CharacterSpaceShipTest::OnAdjustBodyVelocity(const CharacterVirtual *inCharacter, const Body &inBody2, Vec3 &ioLinearVelocity, Vec3 &ioAngularVelocity)
{
	// Cancel out velocity of space ship, we move relative to this which means we don't feel any of the acceleration of the ship (= engage inertial dampeners!)
	ioLinearVelocity -= mSpaceShipLinearVelocity;
	ioAngularVelocity -= mSpaceShipAngularVelocity;
}
