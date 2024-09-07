// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Character/CharacterPlanetTest.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(CharacterPlanetTest)
{
	JPH_ADD_BASE_CLASS(CharacterPlanetTest, Test)
}

void CharacterPlanetTest::Initialize()
{
	// Create planet
	mBodyInterface->CreateAndAddBody(BodyCreationSettings(new SphereShape(cPlanetRadius), RVec3::sZero(), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);

	// Create spheres
	BodyCreationSettings sphere(new SphereShape(0.5f), RVec3::sZero(), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
	sphere.mGravityFactor = 0.0f; // We do our own gravity
	sphere.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
	sphere.mMassPropertiesOverride.mMass = 10.0f;
	sphere.mAngularDamping = 0.5f;
	default_random_engine random;
	for (int i = 0; i < 200; ++i)
	{
		uniform_real_distribution<float> theta(0, JPH_PI);
		uniform_real_distribution<float> phi(0, 2 * JPH_PI);
		sphere.mPosition = RVec3(1.1f * cPlanetRadius * Vec3::sUnitSpherical(theta(random), phi(random)));
		mBodyInterface->CreateAndAddBody(sphere, EActivation::Activate);
	}

	// Register to receive OnStep callbacks to apply gravity
	mPhysicsSystem->AddStepListener(this);

	// Create 'player' character
	Ref<CharacterVirtualSettings> settings = new CharacterVirtualSettings();
	settings->mShape = RotatedTranslatedShapeSettings(Vec3(0, 0.5f * cCharacterHeightStanding + cCharacterRadiusStanding, 0), Quat::sIdentity(), new CapsuleShape(0.5f * cCharacterHeightStanding, cCharacterRadiusStanding)).Create().Get();
	settings->mSupportingVolume = Plane(Vec3::sAxisY(), -cCharacterRadiusStanding); // Accept contacts that touch the lower sphere of the capsule
	mCharacter = new CharacterVirtual(settings, RVec3(0, cPlanetRadius, 0), Quat::sIdentity(), 0, mPhysicsSystem);
	mCharacter->SetListener(this);
}

void CharacterPlanetTest::ProcessInput(const ProcessInputParams &inParams)
{
	// Determine controller input
	Vec3 control_input = Vec3::sZero();
	if (inParams.mKeyboard->IsKeyPressed(DIK_LEFT))		control_input.SetZ(-1);
	if (inParams.mKeyboard->IsKeyPressed(DIK_RIGHT))	control_input.SetZ(1);
	if (inParams.mKeyboard->IsKeyPressed(DIK_UP))		control_input.SetX(1);
	if (inParams.mKeyboard->IsKeyPressed(DIK_DOWN))		control_input.SetX(-1);
	if (control_input != Vec3::sZero())
		control_input = control_input.Normalized();

	// Smooth the player input
	mDesiredVelocity = 0.25f * control_input * cCharacterSpeed + 0.75f * mDesiredVelocity;

	// Convert player input to world space
	Vec3 up = mCharacter->GetUp();
	Vec3 right = inParams.mCameraState.mForward.Cross(up).NormalizedOr(Vec3::sAxisZ());
	Vec3 forward = up.Cross(right).NormalizedOr(Vec3::sAxisX());
	mDesiredVelocityWS = right * mDesiredVelocity.GetZ() + forward * mDesiredVelocity.GetX();

	// Check actions
	mJump = inParams.mKeyboard->IsKeyPressedAndTriggered(DIK_RCONTROL, mWasJump);
}

void CharacterPlanetTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	// Calculate up vector based on position on planet surface
	Vec3 old_up = mCharacter->GetUp();
	Vec3 up = Vec3(mCharacter->GetPosition()).Normalized();
	mCharacter->SetUp(up);

	// Rotate capsule so it points up relative to the planet surface
	mCharacter->SetRotation((Quat::sFromTo(old_up, up) * mCharacter->GetRotation()).Normalized());

	// Draw character pre update (the sim is also drawn pre update)
#ifdef JPH_DEBUG_RENDERER
	mCharacter->GetShape()->Draw(mDebugRenderer, mCharacter->GetCenterOfMassTransform(), Vec3::sReplicate(1.0f), Color::sGreen, false, true);
#endif // JPH_DEBUG_RENDERER

	// Determine new character velocity
	Vec3 current_vertical_velocity = mCharacter->GetLinearVelocity().Dot(up) * up;
	Vec3 ground_velocity = mCharacter->GetGroundVelocity();
	Vec3 new_velocity;
	if (mCharacter->GetGroundState() == CharacterVirtual::EGroundState::OnGround // If on ground
		&& (current_vertical_velocity - ground_velocity).Dot(up) < 0.1f) // And not moving away from ground
	{
		// Assume velocity of ground when on ground
		new_velocity = ground_velocity;

		// Jump
		if (mJump)
			new_velocity += cJumpSpeed * up;
	}
	else
		new_velocity = current_vertical_velocity;

	// Apply gravity
	Vec3 gravity = -up * mPhysicsSystem->GetGravity().Length();
	new_velocity += gravity * inParams.mDeltaTime;

	// Apply player input
	new_velocity += mDesiredVelocityWS;

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
}

void CharacterPlanetTest::GetInitialCamera(CameraState& ioState) const
{
	ioState.mPos = RVec3(0, 0.5f, 0);
	ioState.mForward = Vec3(1, -0.3f, 0).Normalized();
}

RMat44 CharacterPlanetTest::GetCameraPivot(float inCameraHeading, float inCameraPitch) const
{
	// Pivot is center of character + distance behind based on the heading and pitch of the camera.
	Vec3 fwd = Vec3(Cos(inCameraPitch) * Cos(inCameraHeading), Sin(inCameraPitch), Cos(inCameraPitch) * Sin(inCameraHeading));
	RVec3 cam_pos = mCharacter->GetPosition() - 5.0f * (mCharacter->GetRotation() * fwd);
	return RMat44::sRotationTranslation(mCharacter->GetRotation(), cam_pos);
}

void CharacterPlanetTest::SaveState(StateRecorder &inStream) const
{
	mCharacter->SaveState(inStream);

	// Save character up, it's not stored by default but we use it in this case update the rotation of the character
	inStream.Write(mCharacter->GetUp());
}

void CharacterPlanetTest::RestoreState(StateRecorder &inStream)
{
	mCharacter->RestoreState(inStream);

	Vec3 up = mCharacter->GetUp();
	inStream.Read(up);
	mCharacter->SetUp(up);
}

void CharacterPlanetTest::SaveInputState(StateRecorder &inStream) const
{
	inStream.Write(mDesiredVelocity);
	inStream.Write(mDesiredVelocityWS);
	inStream.Write(mJump);
}

void CharacterPlanetTest::RestoreInputState(StateRecorder &inStream)
{
	inStream.Read(mDesiredVelocity);
	inStream.Read(mDesiredVelocityWS);
	inStream.Read(mJump);
}

void CharacterPlanetTest::OnStep(float inDeltaTime, PhysicsSystem &inPhysicsSystem)
{
	// Use the length of the global gravity vector
	float gravity = inPhysicsSystem.GetGravity().Length();

	// We don't need to lock the bodies since they're already locked in the OnStep callback.
	// Note that this means we're responsible for avoiding race conditions with other step listeners while accessing bodies.
	// We know that this is safe because in this demo there's only one step listener.
	const BodyLockInterface &body_interface = inPhysicsSystem.GetBodyLockInterfaceNoLock();

	// Loop over all active bodies
	BodyIDVector body_ids;
	inPhysicsSystem.GetActiveBodies(EBodyType::RigidBody, body_ids);
	for (const BodyID &id : body_ids)
	{
		BodyLockWrite lock(body_interface, id);
		if (lock.Succeeded())
		{
			// Apply gravity towards the center of the planet
			Body &body = lock.GetBody();
			RVec3 position = body.GetPosition();
			float mass = 1.0f / body.GetMotionProperties()->GetInverseMass();
			body.AddForce(-gravity * mass * Vec3(position).Normalized());
		}
	}
}

void CharacterPlanetTest::OnContactAdded(const CharacterVirtual *inCharacter, const BodyID &inBodyID2, const SubShapeID &inSubShapeID2, RVec3Arg inContactPosition, Vec3Arg inContactNormal, CharacterContactSettings &ioSettings)
{
	// We don't want the spheres to push the player character
	ioSettings.mCanPushCharacter = false;
}
