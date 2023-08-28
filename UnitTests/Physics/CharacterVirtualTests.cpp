// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2022 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include "PhysicsTestContext.h"
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Character/CharacterVirtual.h>
#include "Layers.h"

TEST_SUITE("CharacterVirtualTests")
{
	class Character : public CharacterContactListener
	{
	public:
		// Construct
								Character(PhysicsTestContext &ioContext) : mContext(ioContext) { }

		// Create the character
		void					Create()
		{
			// Create capsule
			Ref<Shape> capsule = new CapsuleShape(0.5f * mHeightStanding, mRadiusStanding);
			mCharacterSettings.mShape = RotatedTranslatedShapeSettings(Vec3(0, 0.5f * mHeightStanding + mRadiusStanding, 0), Quat::sIdentity(), capsule).Create().Get();

			// Configure supporting volume
			mCharacterSettings.mSupportingVolume = Plane(Vec3::sAxisY(), -mHeightStanding); // Accept contacts that touch the lower sphere of the capsule

			// Create character
			mCharacter = new CharacterVirtual(&mCharacterSettings, mInitialPosition, Quat::sIdentity(), mContext.GetSystem());
			mCharacter->SetListener(this);
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

			RVec3 start_pos = mCharacter->GetPosition();

			// Update the character position
			TempAllocatorMalloc allocator;
			mCharacter->ExtendedUpdate(delta_time,
				system->GetGravity(),
				mUpdateSettings,
				system->GetDefaultBroadPhaseLayerFilter(Layers::MOVING),
				system->GetDefaultLayerFilter(Layers::MOVING),
				{ },
				{ },
				allocator);

			// Calculate effective velocity in this step
			mEffectiveVelocity = Vec3(mCharacter->GetPosition() - start_pos) / delta_time;
		}

		// Simulate a longer period of time
		void					Simulate(float inTime)
		{
			int num_steps = (int)round(inTime / mContext.GetDeltaTime());
			for (int step = 0; step < num_steps; ++step)
				Step();
		}

		// Configuration
		RVec3					mInitialPosition = RVec3::sZero();
		float					mHeightStanding = 1.35f;
		float					mRadiusStanding = 0.3f;
		CharacterVirtualSettings mCharacterSettings;
		CharacterVirtual::ExtendedUpdateSettings mUpdateSettings;

		// Character movement settings (update to control the movement of the character)
		Vec3					mHorizontalSpeed = Vec3::sZero();
		float					mJumpSpeed = 0.0f; // Character will jump when not 0, will auto reset

		// The character
		Ref<CharacterVirtual>	mCharacter;

		// Calculated effective velocity after a step
		Vec3					mEffectiveVelocity = Vec3::sZero();

	private:
		// CharacterContactListener callback
		virtual void			OnContactSolve(const CharacterVirtual *inCharacter, const BodyID &inBodyID2, const SubShapeID &inSubShapeID2, RVec3Arg inContactPosition, Vec3Arg inContactNormal, Vec3Arg inContactVelocity, const PhysicsMaterial *inContactMaterial, Vec3Arg inCharacterVelocity, Vec3 &ioNewCharacterVelocity) override
		{
			// Don't allow sliding if the character doesn't want to move
			if (mHorizontalSpeed.IsNearZero() && inContactVelocity.IsNearZero() && !inCharacter->IsSlopeTooSteep(inContactNormal))
				ioNewCharacterVelocity = Vec3::sZero();
		}

		PhysicsTestContext &	mContext;
	};

	TEST_CASE("TestFallingAndJumping")
	{
		// Create floor
		PhysicsTestContext c;
		c.CreateFloor();

		// Create character
		Character character(c);
		character.mInitialPosition = RVec3(0, 2, 0);
		character.Create();

		// After 1 step we should still be in air
		character.Step();
		CHECK(character.mCharacter->GetGroundState() == CharacterBase::EGroundState::InAir);

		// After some time we should be on the floor
		character.Simulate(1.0f);
		CHECK(character.mCharacter->GetGroundState() == CharacterBase::EGroundState::OnGround);
		CHECK_APPROX_EQUAL(character.mCharacter->GetPosition(), RVec3::sZero());
		CHECK_APPROX_EQUAL(character.mEffectiveVelocity, Vec3::sZero());

		// Jump
		character.mJumpSpeed = 1.0f;
		character.Step();
		Vec3 velocity(0, 1.0f + c.GetDeltaTime() * c.GetSystem()->GetGravity().GetY(), 0);
		CHECK_APPROX_EQUAL(character.mCharacter->GetPosition(), RVec3(velocity * c.GetDeltaTime()));
		CHECK_APPROX_EQUAL(character.mEffectiveVelocity, velocity);
		CHECK(character.mCharacter->GetGroundState() == CharacterBase::EGroundState::InAir);

		// After some time we should be on the floor again
		character.Simulate(1.0f);
		CHECK(character.mCharacter->GetGroundState() == CharacterBase::EGroundState::OnGround);
		CHECK_APPROX_EQUAL(character.mCharacter->GetPosition(), RVec3::sZero());
		CHECK_APPROX_EQUAL(character.mEffectiveVelocity, Vec3::sZero());
	}

	TEST_CASE("TestMovingOnSlope")
	{
		constexpr float cFloorHalfHeight = 1.0f;
		constexpr float cMovementTime = 1.5f;

		// Iterate various slope angles
		for (float slope_angle = DegreesToRadians(5.0f); slope_angle < DegreesToRadians(85.0f); slope_angle += DegreesToRadians(10.0f))
		{
			// Create sloped floor
			PhysicsTestContext c;
			Quat slope_rotation = Quat::sRotation(Vec3::sAxisZ(), slope_angle);
			c.CreateBox(RVec3::sZero(), slope_rotation, EMotionType::Static, EMotionQuality::Discrete, Layers::NON_MOVING, Vec3(100.0f, cFloorHalfHeight, 100.0f));

			// Create character so that it is touching the slope
			Character character(c);
			float radius_and_padding = character.mRadiusStanding + character.mCharacterSettings.mCharacterPadding;
			character.mInitialPosition = RVec3(0, (radius_and_padding + cFloorHalfHeight) / Cos(slope_angle) - radius_and_padding, 0);
			character.Create();

			// Determine if the slope is too steep for the character
			bool too_steep = slope_angle > character.mCharacterSettings.mMaxSlopeAngle;
			CharacterBase::EGroundState expected_ground_state = (too_steep? CharacterBase::EGroundState::OnSteepGround : CharacterBase::EGroundState::OnGround);

			Vec3 gravity = c.GetSystem()->GetGravity();
			float time_step = c.GetDeltaTime();
			Vec3 slope_normal = slope_rotation.RotateAxisY();

			// Calculate expected position after 1 time step
			RVec3 position_after_1_step = character.mInitialPosition;
			if (too_steep)
			{
				// Apply 1 frame of gravity and cancel movement in the slope normal direction
				Vec3 velocity = gravity * time_step;
				velocity -= velocity.Dot(slope_normal) * slope_normal;
				position_after_1_step += velocity * time_step;
			}

			// After 1 step we should be on the slope
			character.Step();
			CHECK(character.mCharacter->GetGroundState() == expected_ground_state);
			CHECK_APPROX_EQUAL(character.mCharacter->GetPosition(), position_after_1_step, 2.0e-6f);

			// Cancel any velocity to make the calculation below easier (otherwise we have to take gravity for 1 time step into account)
			character.mCharacter->SetLinearVelocity(Vec3::sZero());

			RVec3 start_pos = character.mCharacter->GetPosition();

			// Start moving in X direction
			character.mHorizontalSpeed = Vec3(2.0f, 0, 0);
			character.Simulate(cMovementTime);
			CHECK(character.mCharacter->GetGroundState() == expected_ground_state);

			// Calculate resulting translation
			Vec3 translation = Vec3(character.mCharacter->GetPosition() - start_pos);

			// Calculate expected translation
			Vec3 expected_translation;
			if (too_steep)
			{
				// If too steep, we're just falling. Integrate using an Euler integrator.
				Vec3 velocity = Vec3::sZero();
				expected_translation = Vec3::sZero();
				int num_steps = (int)round(cMovementTime / time_step);
				for (int i = 0; i < num_steps; ++i)
				{
					velocity += gravity * time_step;
					expected_translation += velocity * time_step;
				}
			}
			else
			{
				// Every frame we apply 1 delta time * gravity which gets reset on the next update, add this to the horizontal speed
				expected_translation = (character.mHorizontalSpeed + gravity * time_step) * cMovementTime;
			}

			// Cancel movement in slope direction
			expected_translation -= expected_translation.Dot(slope_normal) * slope_normal;

			// Check that we travelled the right amount
			CHECK_APPROX_EQUAL(translation, expected_translation, 1.0e-4f);
		}
	}

	TEST_CASE("TestStickToFloor")
	{
		constexpr float cFloorHalfHeight = 1.0f;
		constexpr float cSlopeAngle = DegreesToRadians(45.0f);
		constexpr float cMovementTime = 1.5f;

		for (int mode = 0; mode < 2; ++mode)
		{
			// If this run is with 'stick to floor' enabled
			bool stick_to_floor = mode == 0;

			// Create sloped floor
			PhysicsTestContext c;
			Quat slope_rotation = Quat::sRotation(Vec3::sAxisZ(), cSlopeAngle);
			c.CreateBox(RVec3::sZero(), slope_rotation, EMotionType::Static, EMotionQuality::Discrete, Layers::NON_MOVING, Vec3(100.0f, cFloorHalfHeight, 100.0f));

			// Create character so that it is touching the slope
			Character character(c);
			float radius_and_padding = character.mRadiusStanding + character.mCharacterSettings.mCharacterPadding;
			character.mInitialPosition = RVec3(0, (radius_and_padding + cFloorHalfHeight) / Cos(cSlopeAngle) - radius_and_padding, 0);
			character.mUpdateSettings.mStickToFloorStepDown = stick_to_floor? Vec3(0, -0.5f, 0) : Vec3::sZero();
			character.Create();

			// After 1 step we should be on the slope
			character.Step();
			CHECK(character.mCharacter->GetGroundState() == CharacterBase::EGroundState::OnGround);

			// Cancel any velocity to make the calculation below easier (otherwise we have to take gravity for 1 time step into account)
			character.mCharacter->SetLinearVelocity(Vec3::sZero());

			RVec3 start_pos = character.mCharacter->GetPosition();

			// Start moving down the slope at a speed high enough so that gravity will not keep us on the floor
			character.mHorizontalSpeed = Vec3(-10.0f, 0, 0);
			character.Simulate(cMovementTime);
			CHECK(character.mCharacter->GetGroundState() == (stick_to_floor? CharacterBase::EGroundState::OnGround : CharacterBase::EGroundState::InAir));

			// Calculate resulting translation
			Vec3 translation = Vec3(character.mCharacter->GetPosition() - start_pos);

			// Calculate expected translation
			Vec3 expected_translation;
			if (stick_to_floor)
			{
				// We should stick to the floor, so the vertical translation follows the slope perfectly
				expected_translation = character.mHorizontalSpeed * cMovementTime;
				expected_translation.SetY(expected_translation.GetX() * Tan(cSlopeAngle));
			}
			else
			{
				Vec3 gravity = c.GetSystem()->GetGravity();
				float time_step = c.GetDeltaTime();

				// If too steep, we're just falling. Integrate using an Euler integrator.
				Vec3 velocity = character.mHorizontalSpeed;
				expected_translation = Vec3::sZero();
				int num_steps = (int)round(cMovementTime / time_step);
				for (int i = 0; i < num_steps; ++i)
				{
					velocity += gravity * time_step;
					expected_translation += velocity * time_step;
				}
			}

			// Check that we travelled the right amount
			CHECK_APPROX_EQUAL(translation, expected_translation, 1.0e-4f);
		}
	}

	TEST_CASE("TestWalkStairs")
	{
		const float cStepHeight = 0.3f;
		const int cNumSteps = 10;

		// Create stairs from triangles
		TriangleList triangles;
		for (int i = 0; i < cNumSteps; ++i)
		{
			// Start of step
			Vec3 base(0, cStepHeight * i, cStepHeight * i);

			// Left side
			Vec3 b1 = base + Vec3(2.0f, 0, 0);
			Vec3 s1 = b1 + Vec3(0, cStepHeight, 0);
			Vec3 p1 = s1 + Vec3(0, 0, cStepHeight);

			// Right side
			Vec3 width(-4.0f, 0, 0);
			Vec3 b2 = b1 + width;
			Vec3 s2 = s1 + width;
			Vec3 p2 = p1 + width;

			triangles.push_back(Triangle(s1, b1, s2));
			triangles.push_back(Triangle(b1, b2, s2));
			triangles.push_back(Triangle(s1, p2, p1));
			triangles.push_back(Triangle(s1, s2, p2));
		}

		MeshShapeSettings mesh(triangles);
		mesh.SetEmbedded();
		BodyCreationSettings mesh_stairs(&mesh, RVec3::sZero(), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);

		// Stair stepping is very delta time sensitive, so test various update frequencies
		float frequencies[] = { 60.0f, 120.0f, 240.0f, 360.0f };
		for (float frequency : frequencies)
		{
			float time_step = 1.0f / frequency;

			PhysicsTestContext c(time_step);
			c.CreateFloor();
			c.GetBodyInterface().CreateAndAddBody(mesh_stairs, EActivation::DontActivate);

			// Create character so that it is touching the slope
			Character character(c);
			character.mInitialPosition = RVec3(0, 0, -2.0f); // Start in front of the stairs
			character.mUpdateSettings.mWalkStairsStepUp = Vec3::sZero(); // No stair walking
			character.Create();

			// Start moving towards the stairs
			character.mHorizontalSpeed = Vec3(0, 0, 4.0f);
			character.Simulate(1.0f);

			// We should have gotten stuck at the start of the stairs (can't move up)
			CHECK(character.mCharacter->GetGroundState() == CharacterBase::EGroundState::OnGround);
			float radius_and_padding = character.mRadiusStanding + character.mCharacterSettings.mCharacterPadding;
			CHECK_APPROX_EQUAL(character.mCharacter->GetPosition(), RVec3(0, 0, -radius_and_padding), 1.1e-2f);

			// Enable stair walking
			character.mUpdateSettings.mWalkStairsStepUp = Vec3(0, 0.4f, 0);

			// Calculate time it should take to move up the stairs at constant speed
			float movement_time = (cNumSteps * cStepHeight + radius_and_padding) / character.mHorizontalSpeed.GetZ();
			int max_steps = int(1.5f * round(movement_time / time_step)); // In practise there is a bit of slowdown while stair stepping, so add a bit of slack

			// Step until we reach the top of the stairs
			RVec3 last_position = character.mCharacter->GetPosition();
			bool reached_goal = false;
			for (int i = 0; i < max_steps; ++i)
			{
				character.Step();

				// We should always be on the floor during stair stepping
				CHECK(character.mCharacter->GetGroundState() == CharacterBase::EGroundState::OnGround);

				// Check position progression
				RVec3 position = character.mCharacter->GetPosition();
				CHECK_APPROX_EQUAL(position.GetX(), 0); // No movement in X
				CHECK(position.GetZ() > last_position.GetZ()); // Always moving forward
				CHECK(position.GetZ() < cNumSteps * cStepHeight); // No movement beyond stairs
				if (position.GetY() > cNumSteps * cStepHeight - 1.0e-3f)
				{
					reached_goal = true;
					break;
				}

				last_position = position;
			}
			CHECK(reached_goal);
		}
	}

	TEST_CASE("TestRotatingPlatform")
	{
		constexpr float cFloorHalfHeight = 1.0f;
		constexpr float cFloorHalfWidth = 10.0f;
		constexpr float cCharacterPosition = 0.9f * cFloorHalfWidth;
		constexpr float cAngularVelocity = 2.0f * JPH_PI;

		PhysicsTestContext c;

		// Create box
		Body &box = c.CreateBox(RVec3::sZero(), Quat::sIdentity(), EMotionType::Kinematic, EMotionQuality::Discrete, Layers::MOVING, Vec3(cFloorHalfWidth, cFloorHalfHeight, cFloorHalfWidth));
		box.SetAllowSleeping(false);

		// Create character so that it is touching the box at the
		Character character(c);
		character.mInitialPosition = RVec3(cCharacterPosition, cFloorHalfHeight, 0);
		character.Create();

		// Step to ensure the character is on the box
		character.Step();
		CHECK(character.mCharacter->GetGroundState() == CharacterBase::EGroundState::OnGround);

		// Set the box to rotate a full circle per second
		box.SetAngularVelocity(Vec3(0, cAngularVelocity, 0));

		// Rotate and check that character stays on the box
		for (int t = 0; t < 60; ++t)
		{
			character.Step();
			CHECK(character.mCharacter->GetGroundState() == CharacterBase::EGroundState::OnGround);

			// Note that the character moves according to the ground velocity and the ground velocity is updated at the end of the step
			// so the character is always 1 time step behind the platform. This is why we use t and not t + 1 to calculate the expected position.
			RVec3 expected_position = RMat44::sRotation(Quat::sRotation(Vec3::sAxisY(), float(t) * c.GetDeltaTime() * cAngularVelocity)) * character.mInitialPosition;
			CHECK_APPROX_EQUAL(character.mCharacter->GetPosition(), expected_position, 1.0e-4f);
		}
	}

	TEST_CASE("TestMovingPlatformUp")
	{
		constexpr float cFloorHalfHeight = 1.0f;
		constexpr float cFloorHalfWidth = 10.0f;
		constexpr float cLinearVelocity = 0.5f;

		PhysicsTestContext c;

		// Create box
		Body &box = c.CreateBox(RVec3::sZero(), Quat::sIdentity(), EMotionType::Kinematic, EMotionQuality::Discrete, Layers::MOVING, Vec3(cFloorHalfWidth, cFloorHalfHeight, cFloorHalfWidth));
		box.SetAllowSleeping(false);

		// Create character so that it is touching the box at the
		Character character(c);
		character.mInitialPosition = RVec3(0, cFloorHalfHeight, 0);
		character.Create();

		// Step to ensure the character is on the box
		character.Step();
		CHECK(character.mCharacter->GetGroundState() == CharacterBase::EGroundState::OnGround);

		// Set the box to move up
		box.SetLinearVelocity(Vec3(0, cLinearVelocity, 0));

		// Check that character stays on the box
		for (int t = 0; t < 60; ++t)
		{
			character.Step();
			CHECK(character.mCharacter->GetGroundState() == CharacterBase::EGroundState::OnGround);
			RVec3 expected_position = box.GetPosition() + character.mInitialPosition;
			CHECK_APPROX_EQUAL(character.mCharacter->GetPosition(), expected_position, 1.0e-2f);
		}

		// Stop box
		box.SetLinearVelocity(Vec3::sZero());
		character.Simulate(0.5f);

		// Set the box to move down
		box.SetLinearVelocity(Vec3(0, -cLinearVelocity, 0));

		// Check that character stays on the box
		for (int t = 0; t < 60; ++t)
		{
			character.Step();
			CHECK(character.mCharacter->GetGroundState() == CharacterBase::EGroundState::OnGround);
			RVec3 expected_position = box.GetPosition() + character.mInitialPosition;
			CHECK_APPROX_EQUAL(character.mCharacter->GetPosition(), expected_position, 1.0e-2f);
		}
	}

	TEST_CASE("TestContactPointLimit")
	{
		PhysicsTestContext ctx;
		Body &floor = ctx.CreateFloor();

		// Create character at the origin
		Character character(ctx);
		character.mInitialPosition = RVec3(0, 1, 0);
		character.mUpdateSettings.mStickToFloorStepDown = Vec3::sZero();
		character.mUpdateSettings.mWalkStairsStepUp = Vec3::sZero();
		character.Create();

		// Radius including pading
		const float character_radius = character.mRadiusStanding + character.mCharacterSettings.mCharacterPadding;

		// Create a half cylinder with caps for testing contact point limit
		VertexList vertices;
		IndexedTriangleList triangles;

		// The half cylinder
		const int cPosSegments = 2;
		const int cAngleSegments = 768;
		const float cCylinderLength = 2.0f;
		for (int pos = 0; pos < cPosSegments; ++pos)
			for (int angle = 0; angle < cAngleSegments; ++angle)
			{
				uint32 start = (uint32)vertices.size();

				float radius = character_radius + 0.01f;
				float angle_rad = (-0.5f + float(angle) / cAngleSegments) * JPH_PI;
				float s = Sin(angle_rad);
				float c = Cos(angle_rad);
				float x = cCylinderLength * (-0.5f + float(pos) / (cPosSegments - 1));
				float y = angle == 0 || angle == cAngleSegments - 1? 0.5f : (1.0f - c) * radius;
				float z = s * radius;
				vertices.push_back(Float3(x, y, z));

				if (pos > 0 && angle > 0)
				{
					triangles.push_back(IndexedTriangle(start, start - 1, start - cAngleSegments));
					triangles.push_back(IndexedTriangle(start - 1, start - cAngleSegments - 1, start - cAngleSegments));
				}
			}

		// Add end caps
		uint32 end = cAngleSegments * (cPosSegments - 1);
		for (int angle = 0; angle < cAngleSegments - 1; ++angle)
		{
			triangles.push_back(IndexedTriangle(0, angle + 1, angle));
			triangles.push_back(IndexedTriangle(end, end + angle, end + angle + 1));
		}

		// Create test body
		MeshShapeSettings mesh(vertices, triangles);
		mesh.SetEmbedded();
		BodyCreationSettings mesh_cylinder(&mesh, character.mInitialPosition, Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);
		BodyID cylinder_id = ctx.GetBodyInterface().CreateAndAddBody(mesh_cylinder, EActivation::DontActivate);

		// End positions that can be reached by character
		RVec3 pos_end(0.5_r * cCylinderLength - character_radius, 1, 0);
		RVec3 neg_end(-0.5_r * cCylinderLength + character_radius, 1, 0);

		// Move towards positive cap and test if we hit the end
		character.mHorizontalSpeed = Vec3(cCylinderLength, 0, 0);
		for (int t = 0; t < 60; ++t)
		{
			character.Step();
			CHECK(character.mCharacter->GetMaxHitsExceeded());
			CHECK(character.mCharacter->GetActiveContacts().size() <= character.mCharacter->GetMaxNumHits());
			CHECK(character.mCharacter->GetGroundBodyID() == cylinder_id);
			CHECK(character.mCharacter->GetGroundNormal().Dot(Vec3::sAxisY()) > 0.999f);
		}
		CHECK_APPROX_EQUAL(character.mCharacter->GetPosition(), pos_end, 1.0e-4f);

		// Move towards negative cap and test if we hit the end
		character.mHorizontalSpeed = Vec3(-cCylinderLength, 0, 0);
		for (int t = 0; t < 60; ++t)
		{
			character.Step();
			CHECK(character.mCharacter->GetMaxHitsExceeded());
			CHECK(character.mCharacter->GetActiveContacts().size() <= character.mCharacter->GetMaxNumHits());
			CHECK(character.mCharacter->GetGroundBodyID() == cylinder_id);
			CHECK(character.mCharacter->GetGroundNormal().Dot(Vec3::sAxisY()) > 0.999f);
		}
		CHECK_APPROX_EQUAL(character.mCharacter->GetPosition(), neg_end, 1.0e-4f);

		// Turn off contact point reduction
		character.mCharacter->SetHitReductionCosMaxAngle(-1.0f);

		// Move towards positive cap and test that we did not reach the end
		character.mHorizontalSpeed = Vec3(cCylinderLength, 0, 0);
		for (int t = 0; t < 60; ++t)
		{
			character.Step();
			CHECK(character.mCharacter->GetMaxHitsExceeded());
			CHECK(character.mCharacter->GetActiveContacts().size() == character.mCharacter->GetMaxNumHits());
		}
		RVec3 cur_pos = character.mCharacter->GetPosition();
		CHECK((pos_end - cur_pos).Length() > 0.01_r);

		// Move towards negative cap and test that we got stuck
		character.mHorizontalSpeed = Vec3(-cCylinderLength, 0, 0);
		for (int t = 0; t < 60; ++t)
		{
			character.Step();
			CHECK(character.mCharacter->GetMaxHitsExceeded());
			CHECK(character.mCharacter->GetActiveContacts().size() == character.mCharacter->GetMaxNumHits());
		}
		CHECK(cur_pos.IsClose(character.mCharacter->GetPosition(), 1.0e-6f));

		// Now teleport the character next to the half cylinder
		character.mCharacter->SetPosition(RVec3(0, 0, 1));

		// Move in positive X and check that we did not exceed max hits and that we were able to move unimpeded
		character.mHorizontalSpeed = Vec3(cCylinderLength, 0, 0);
		for (int t = 0; t < 60; ++t)
		{
			character.Step();
			CHECK(!character.mCharacter->GetMaxHitsExceeded());
			CHECK(character.mCharacter->GetActiveContacts().size() == 1); // We should only hit the floor
			CHECK(character.mCharacter->GetGroundBodyID() == floor.GetID());
			CHECK(character.mCharacter->GetGroundNormal().Dot(Vec3::sAxisY()) > 0.999f);
		}
		CHECK_APPROX_EQUAL(character.mCharacter->GetPosition(), RVec3(cCylinderLength, 0, 1), 1.0e-4f);
	}

	TEST_CASE("TestStairWalkAlongWall")
	{
		// Stair stepping is very delta time sensitive, so test various update frequencies
		float frequencies[] = { 60.0f, 120.0f, 240.0f, 360.0f };
		for (float frequency : frequencies)
		{
			float time_step = 1.0f / frequency;

			PhysicsTestContext c(time_step);
			c.CreateFloor();

			// Create character
			Character character(c);
			character.Create();

			// Create a wall
			const float cWallHalfThickness = 0.05f;
			c.GetBodyInterface().CreateAndAddBody(BodyCreationSettings(new BoxShape(Vec3(50.0f, 1.0f, cWallHalfThickness)), RVec3(0, 1.0_r, Real(-character.mRadiusStanding - character.mCharacter->GetCharacterPadding() - cWallHalfThickness)), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);

			// Start moving along the wall, if the stair stepping algorithm is working correctly it should not trigger and not apply extra speed to the character
			character.mHorizontalSpeed = Vec3(5.0f, 0, -1.0f);
			character.Simulate(1.0f);

			// We should have moved along the wall at the desired speed
			CHECK(character.mCharacter->GetGroundState() == CharacterBase::EGroundState::OnGround);
			CHECK_APPROX_EQUAL(character.mCharacter->GetPosition(), RVec3(5.0f, 0, 0), 1.0e-2f);
		}
	}

	TEST_CASE("TestInitiallyIntersecting")
	{
		PhysicsTestContext c;
		c.CreateFloor();

		// Create box that is intersecting with the character
		c.CreateBox(RVec3(-0.5f, 0.5f, 0), Quat::sIdentity(), EMotionType::Static, EMotionQuality::Discrete, Layers::NON_MOVING, Vec3::sReplicate(0.5f));

		// Try various penetration recovery values
		for (float penetration_recovery : { 0.0f, 0.5f, 0.75f, 1.0f })
		{
			// Create character
			Character character(c);
			character.mCharacterSettings.mPenetrationRecoverySpeed = penetration_recovery;
			character.Create();
			CHECK_APPROX_EQUAL(character.mCharacter->GetPosition(), RVec3::sZero());

			// Total radius of character
			float radius_and_padding = character.mRadiusStanding + character.mCharacterSettings.mCharacterPadding;

			float x = 0.0f;
			for (int step = 0; step < 3; ++step)
			{
				// Calculate expected position
				x += penetration_recovery * (radius_and_padding - x);

				// Step character and check that it matches expected recovery
				character.Step();
				CHECK_APPROX_EQUAL(character.mCharacter->GetPosition(), RVec3(x, 0, 0));
			}
		}
	}
}
