// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Samples.h>

#include <Tests/Character/CharacterGatheringTest.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Jolt/Physics/Collision/Shape/HeightFieldShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <External/Perlin.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(CharacterGatheringTest)
{
	JPH_ADD_BASE_CLASS(CharacterGatheringTest, Test)
}

CharacterGatheringTest::~CharacterGatheringTest()
{
	for (Character *c : mCharacters)
		c->RemoveFromPhysicsSystem();
	mCharacters.clear();
}

void CharacterGatheringTest::Initialize()
{
	const int n = 128;
	const float cell_size = 1.0f;
	const float max_height = 5.0f;

	// Create height field
	Array<float> terrain;
	terrain.resize(n * n);
	for (int y = 0; y < n; ++y)
		for (int x = 0; x < n; ++x)
			terrain[y * n + x] = max_height * PerlinNoise3(float(x) * 8.0f / n, 0, float(y) * 8.0f / n, 256, 256, 256);
	Vec3 terrain_offset = Vec3(-0.5f * cell_size * n, -2.0f, -0.5f * cell_size * n);
	Vec3 terrain_scale = Vec3(cell_size, 1.0f, cell_size);
	mBodyInterface->CreateAndAddBody(BodyCreationSettings(new HeightFieldShapeSettings(terrain.data(), terrain_offset, terrain_scale, n), RVec3::sZero(), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);

	// Create characters
	Ref<Shape> character_shape = RotatedTranslatedShapeSettings(Vec3(0, 0.5f * cCharacterHeightStanding + cCharacterRadiusStanding, 0), Quat::sIdentity(), new CapsuleShape(0.5f * cCharacterHeightStanding, cCharacterRadiusStanding)).Create().Get();
	for (int y = 0; y < cNumCharactersY; ++y)
		for (int x = 0; x < cNumCharactersX; ++x)
		{
			Ref<CharacterSettings> csettings = new CharacterSettings();
			csettings->mLayer = Layers::MOVING;
			csettings->mShape = character_shape;
			csettings->mSupportingVolume = Plane(Vec3::sAxisY(), -cCharacterRadiusStanding); // Accept contacts that touch the lower sphere of the capsule

			Character *c = new Character(csettings, RVec3(5.0_r * (x - cNumCharactersX / 2), 2.0_r, 5.0_r * (y - cNumCharactersY / 2)), Quat::sIdentity(), 0, mPhysicsSystem);
			c->AddToPhysicsSystem(EActivation::Activate);
			mCharacters.push_back(c);
		}
}

void CharacterGatheringTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	for (Character *c : mCharacters)
		if (c->IsSupported())
		{
			// Update velocity
			Vec3 current_velocity = c->GetLinearVelocity();
			Vec3 movement_direction = -Vec3(c->GetPosition()).Normalized();
			Vec3 desired_velocity = sCharacterSpeed * movement_direction;
			desired_velocity.SetY(current_velocity.GetY());
			Vec3 new_velocity = 0.75f * current_velocity + 0.25f * desired_velocity;
			c->SetLinearVelocity(new_velocity);
		}
}

void CharacterGatheringTest::PostPhysicsUpdate(float inDeltaTime)
{
	// Fetch the new ground properties
	for (Character *c : mCharacters)
		c->PostSimulation(cCollisionTolerance);
}
