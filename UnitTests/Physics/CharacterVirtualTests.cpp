// SPDX-FileCopyrightText: 2022 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include "PhysicsTestContext.h"
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Jolt/Physics/Character/CharacterVirtual.h>
#include "Layers.h"

TEST_SUITE("CharacterVirtualTests")
{
	class Character
	{
	public:
		// Construct
								Character(PhysicsTestContext &ioContext) : mContext(ioContext) { }

		// Create the character
		void					Create()
		{
			Ref<Shape> capsule = new CapsuleShape(0.5f * mHeightStanding, mRadiusStanding);
			Ref<Shape> offset_capsule = RotatedTranslatedShapeSettings(Vec3(0, 0.5f * mHeightStanding + mRadiusStanding, 0), Quat::sIdentity(), capsule).Create().Get();

			CharacterVirtualSettings settings;
			settings.mMaxSlopeAngle = mMaxSlopeAngle;
			settings.mShape = offset_capsule;
			settings.mSupportingVolume = Plane(Vec3::sAxisY(), -mHeightStanding); // Accept contacts that touch the lower sphere of the capsule

			mCharacter = new CharacterVirtual(&settings, mInitialPosition, Quat::sIdentity(), mContext.GetSystem());
		}

		// Step the character and the world
		void					Step()
		{
			// Step the world
			mContext.SimulateSingleStep();

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
				new_velocity += Vec3(0, mJumpSpeed, 0);
				mJumpSpeed = 0.0f;
			}
			else
				new_velocity = current_vertical_velocity;

			// Gravity
			PhysicsSystem *system = mContext.GetSystem();
			float delta_time = mContext.GetDeltaTime();
			new_velocity += system->GetGravity() * delta_time;

			// Player input
			new_velocity += mHorizontalSpeed;

			// Update character velocity
			mCharacter->SetLinearVelocity(new_velocity);

			Vec3 old_pos = mCharacter->GetPosition();

			// Update the character position
			TempAllocatorMalloc allocator;
			mCharacter->ExtendedUpdate(delta_time,
				system->GetGravity(),
				mUpdateSettings,
				system->GetDefaultBroadPhaseLayerFilter(Layers::MOVING),
				system->GetDefaultLayerFilter(Layers::MOVING),
				{ },
				allocator);

			mEffectiveVelocity = (mCharacter->GetPosition() - old_pos) / delta_time;
		}

		// Simulate a longer period of time
		void					Simulate(float inTime)
		{
			float step = mContext.GetDeltaTime();
			for (float t = 0.0f; t < inTime; t += step)
				Step();
		}

		// Configuration
		Vec3					mInitialPosition = Vec3::sZero();
		float					mHeightStanding = 1.35f;
		float					mRadiusStanding = 0.3f;
		float					mMaxSlopeAngle = DegreesToRadians(50.0f);
		CharacterVirtual::ExtendedUpdateSettings mUpdateSettings;

		// Character movement settings (update to control the movement of the character)
		Vec3					mHorizontalSpeed = Vec3::sZero();
		float					mJumpSpeed = 0.0f; // Character will jump when not 0, will auto reset

		// The character
		Ref<CharacterVirtual>	mCharacter;

		// Calculated effective velocity after a step
		Vec3					mEffectiveVelocity = Vec3::sZero();

	private:
		PhysicsTestContext &	mContext;
	};

	TEST_CASE("TestFallingAndJumping")
	{
		// Create floor
		PhysicsTestContext c;
		c.CreateFloor();

		// Create character
		Character character(c);
		character.mInitialPosition = Vec3(0, 2, 0);
		character.Create();

		// After 1 step we should still be in air
		character.Step();
		CHECK(character.mCharacter->GetGroundState() == CharacterBase::EGroundState::InAir);

		// After some time we should be on the floor
		character.Simulate(1.0f);
		CHECK(character.mCharacter->GetGroundState() == CharacterBase::EGroundState::OnGround);
		CHECK_APPROX_EQUAL(character.mCharacter->GetPosition(), Vec3::sZero());
		CHECK_APPROX_EQUAL(character.mEffectiveVelocity, Vec3::sZero());

		// Jump
		character.mJumpSpeed = 1.0f;
		character.Step();
		Vec3 velocity(0, 1.0f + c.GetDeltaTime() * c.GetSystem()->GetGravity().GetY(), 0);
		CHECK_APPROX_EQUAL(character.mCharacter->GetPosition(), velocity * c.GetDeltaTime());
		CHECK_APPROX_EQUAL(character.mEffectiveVelocity, velocity);
		CHECK(character.mCharacter->GetGroundState() == CharacterBase::EGroundState::InAir);

		// After some time we should be on the floor again
		character.Simulate(1.0f);
		CHECK(character.mCharacter->GetGroundState() == CharacterBase::EGroundState::OnGround);
		CHECK_APPROX_EQUAL(character.mCharacter->GetPosition(), Vec3::sZero());
		CHECK_APPROX_EQUAL(character.mEffectiveVelocity, Vec3::sZero());
	}
}
