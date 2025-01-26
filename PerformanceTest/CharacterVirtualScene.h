// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

// Jolt includes
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Jolt/Physics/Character/CharacterVirtual.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>

// Local includes
#include "PerformanceTestScene.h"
#include "Layers.h"

// A scene that drops a number of virtual characters on a scene and simulates them
class CharacterVirtualScene : public PerformanceTestScene, public CharacterContactListener
{
public:
	virtual const char *	GetName() const override
	{
		return "CharacterVirtual";
	}

	virtual bool			Load(const String &inAssetPath) override
	{
		const int n = 100;
		const float cell_size = 0.5f;
		const float max_height = 2.0f;
		float center = n * cell_size / 2;

		// Create vertices
		const int num_vertices = (n + 1) * (n + 1);
		VertexList vertices;
		vertices.resize(num_vertices);
		for (int x = 0; x <= n; ++x)
			for (int z = 0; z <= n; ++z)
			{
				float height = Sin(float(x) * 20.0f / n) * Cos(float(z) * 20.0f / n);
				vertices[z * (n + 1) + x] = Float3(cell_size * x, max_height * height, cell_size * z);
			}

		// Create regular grid of triangles
		const int num_triangles = n * n * 2;
		IndexedTriangleList indices;
		indices.resize(num_triangles);
		IndexedTriangle *next = indices.data();
		for (int x = 0; x < n; ++x)
			for (int z = 0; z < n; ++z)
			{
				int start = (n + 1) * z + x;

				next->mIdx[0] = start;
				next->mIdx[1] = start + n + 1;
				next->mIdx[2] = start + 1;
				next++;

				next->mIdx[0] = start + 1;
				next->mIdx[1] = start + n + 1;
				next->mIdx[2] = start + n + 2;
				next++;
			}

		// Create mesh
		BodyCreationSettings mesh(new MeshShapeSettings(vertices, indices), RVec3(Real(-center), 0, Real(-center)), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);
		mWorld.push_back(mesh);

		// Create pyramid stairs
		for (int i = 0; i < 10; ++i)
		{
			float width = 4.0f - 0.4f * i;
			BodyCreationSettings step(new BoxShape(Vec3(width, 0.5f * cStairsStepHeight, width)), RVec3(-4.0_r, -1.0_r + Real(i * cStairsStepHeight), 0), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);
			mWorld.push_back(step);
		}

		// Create wall consisting of vertical pillars
		Ref<Shape> wall = new BoxShape(Vec3(0.1f, 2.5f, 0.1f), 0.0f);
		for (int z = 0; z < 10; ++z)
		{
			BodyCreationSettings bcs(wall, RVec3(2.0_r, 1.0_r, 2.0_r + 0.2_r * z), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);
			mWorld.push_back(bcs);
		}

		// Create some dynamic boxes
		Ref<Shape> box = new BoxShape(Vec3::sReplicate(0.25f));
		for (int x = 0; x < 10; ++x)
			for (int z = 0; z < 10; ++z)
			{
				BodyCreationSettings bcs(box, RVec3(4.0_r * x - 20.0_r, 5.0_r, 4.0_r * z - 20.0_r), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
				bcs.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
				bcs.mMassPropertiesOverride.mMass = 1.0f;
				mWorld.push_back(bcs);
			}

		return true;
	}

	virtual void			StartTest(PhysicsSystem &inPhysicsSystem, EMotionQuality inMotionQuality) override
	{
		// Construct bodies
		BodyInterface &bi = inPhysicsSystem.GetBodyInterface();
		for (BodyCreationSettings &bcs : mWorld)
			if (bcs.mMotionType == EMotionType::Dynamic)
			{
				bcs.mMotionQuality = inMotionQuality;
				bi.CreateAndAddBody(bcs, EActivation::Activate);
			}
			else
				bi.CreateAndAddBody(bcs, EActivation::DontActivate);

		// Construct characters
		CharacterID::sSetNextCharacterID();
		RefConst<Shape> standing_shape = RotatedTranslatedShapeSettings(Vec3(0, 0.5f * cCharacterHeightStanding + cCharacterRadiusStanding, 0), Quat::sIdentity(), new CapsuleShape(0.5f * cCharacterHeightStanding, cCharacterRadiusStanding)).Create().Get();
		RefConst<Shape> inner_standing_shape = RotatedTranslatedShapeSettings(Vec3(0, 0.5f * cCharacterHeightStanding + cCharacterRadiusStanding, 0), Quat::sIdentity(), new CapsuleShape(0.5f * cInnerShapeFraction * cCharacterHeightStanding, cInnerShapeFraction * cCharacterRadiusStanding)).Create().Get();
		for (int y = 0; y < cNumCharactersY; ++y)
			for (int x = 0; x < cNumCharactersX; ++x)
			{
				Ref<CharacterVirtualSettings> settings = new CharacterVirtualSettings();
				settings->mShape = standing_shape;
				settings->mSupportingVolume = Plane(Vec3::sAxisY(), -cCharacterRadiusStanding); // Accept contacts that touch the lower sphere of the capsule
				settings->mInnerBodyShape = inner_standing_shape;
				settings->mInnerBodyLayer = Layers::MOVING;
				Ref<CharacterVirtual> character = new CharacterVirtual(settings, RVec3(4.0_r * x - 20.0_r, 2.0_r, 4.0_r * y - 20.0_r), Quat::sIdentity(), 0, &inPhysicsSystem);
				character->SetCharacterVsCharacterCollision(&mCharacterVsCharacterCollision);
				character->SetListener(this);
				mCharacters.push_back(character);
				mCharacterVsCharacterCollision.Add(character);
			}

		// Start at time 0
		mTime = 0.0f;
		mHash = HashBytes(nullptr, 0);
	}

	virtual void			UpdateTest(PhysicsSystem &inPhysicsSystem, TempAllocator &ioTempAllocator, float inDeltaTime) override
	{
		// Change direction every 2 seconds
		mTime += inDeltaTime;
		uint64 count = uint64(mTime / 2.0f) * cNumCharactersX * cNumCharactersY;

		for (CharacterVirtual *ch : mCharacters)
		{
			// Calculate new vertical velocity
			Vec3 new_velocity;
			if (ch->GetGroundState() == CharacterVirtual::EGroundState::OnGround	// If on ground
				&& ch->GetLinearVelocity().GetY() < 0.1f)						// And not moving away from ground
				new_velocity = Vec3::sZero();
			else
				new_velocity = ch->GetLinearVelocity() * Vec3(0, 1, 0);
			new_velocity += inPhysicsSystem.GetGravity() * inDeltaTime;

			// Deterministic random input
			uint64 hash = Hash<uint64> {} (count);
			int x = int(hash % 10);
			int y = int((hash / 10) % 10);
			int speed = int((hash / 100) % 10);

			// Determine target position
			RVec3 target = RVec3(4.0_r * x - 20.0_r, 5.0_r, 4.0_r * y - 20.0_r);

			// Determine new character velocity
			Vec3 direction = Vec3(target - ch->GetPosition()).NormalizedOr(Vec3::sZero());
			direction.SetY(0);
			new_velocity += (5.0f + 0.5f * speed) * direction;
			ch->SetLinearVelocity(new_velocity);

			// Update the character position
			CharacterVirtual::ExtendedUpdateSettings update_settings;
			ch->ExtendedUpdate(inDeltaTime,
				inPhysicsSystem.GetGravity(),
				update_settings,
				inPhysicsSystem.GetDefaultBroadPhaseLayerFilter(Layers::MOVING),
				inPhysicsSystem.GetDefaultLayerFilter(Layers::MOVING),
				{ },
				{ },
				ioTempAllocator);

			++count;
		}
	}

	virtual void			UpdateHash(uint64 &ioHash) const override
	{
		// Hash the contact callback hash
		HashCombine(ioHash, mHash);

		// Hash the state of all characters
		for (const CharacterVirtual *ch : mCharacters)
			HashCombine(ioHash, ch->GetPosition());
	}

	virtual void			StopTest(PhysicsSystem &inPhysicsSystem) override
	{
		for (const CharacterVirtual *ch : mCharacters)
			mCharacterVsCharacterCollision.Remove(ch);
		mCharacters.clear();
	}

	// See: CharacterContactListener
	virtual void			OnContactAdded(const CharacterVirtual *inCharacter, const BodyID &inBodyID2, const SubShapeID &inSubShapeID2, RVec3Arg inContactPosition, Vec3Arg inContactNormal, CharacterContactSettings &ioSettings) override
	{
		HashCombine(mHash, 1);
		HashCombine(mHash, inCharacter->GetID());
		HashCombine(mHash, inBodyID2);
		HashCombine(mHash, inSubShapeID2.GetValue());
		HashCombine(mHash, inContactPosition);
		HashCombine(mHash, inContactNormal);
	}
	virtual void			OnContactPersisted(const CharacterVirtual *inCharacter, const BodyID &inBodyID2, const SubShapeID &inSubShapeID2, RVec3Arg inContactPosition, Vec3Arg inContactNormal, CharacterContactSettings &ioSettings) override
	{
		HashCombine(mHash, 2);
		HashCombine(mHash, inCharacter->GetID());
		HashCombine(mHash, inBodyID2);
		HashCombine(mHash, inSubShapeID2.GetValue());
		HashCombine(mHash, inContactPosition);
		HashCombine(mHash, inContactNormal);
	}
	virtual void			OnContactRemoved(const CharacterVirtual *inCharacter, const BodyID &inBodyID2, const SubShapeID &inSubShapeID2) override
	{
		HashCombine(mHash, 3);
		HashCombine(mHash, inCharacter->GetID());
		HashCombine(mHash, inBodyID2);
		HashCombine(mHash, inSubShapeID2.GetValue());
	}
	virtual void			OnCharacterContactAdded(const CharacterVirtual *inCharacter, const CharacterVirtual *inOtherCharacter, const SubShapeID &inSubShapeID2, RVec3Arg inContactPosition, Vec3Arg inContactNormal, CharacterContactSettings &ioSettings) override
	{
		HashCombine(mHash, 4);
		HashCombine(mHash, inCharacter->GetID());
		HashCombine(mHash, inOtherCharacter->GetID());
		HashCombine(mHash, inSubShapeID2.GetValue());
		HashCombine(mHash, inContactPosition);
		HashCombine(mHash, inContactNormal);
	}
	virtual void			OnCharacterContactPersisted(const CharacterVirtual *inCharacter, const CharacterVirtual *inOtherCharacter, const SubShapeID &inSubShapeID2, RVec3Arg inContactPosition, Vec3Arg inContactNormal, CharacterContactSettings &ioSettings) override
	{
		HashCombine(mHash, 5);
		HashCombine(mHash, inCharacter->GetID());
		HashCombine(mHash, inOtherCharacter->GetID());
		HashCombine(mHash, inSubShapeID2.GetValue());
		HashCombine(mHash, inContactPosition);
		HashCombine(mHash, inContactNormal);
	}
	virtual void			OnCharacterContactRemoved(const CharacterVirtual *inCharacter, const CharacterID &inOtherCharacterID, const SubShapeID &inSubShapeID2) override
	{
		HashCombine(mHash, 6);
		HashCombine(mHash, inCharacter->GetID());
		HashCombine(mHash, inOtherCharacterID);
		HashCombine(mHash, inSubShapeID2.GetValue());
	}

private:
	static constexpr int	cNumCharactersX = 10;
	static constexpr int	cNumCharactersY = 10;
	static constexpr float	cCharacterHeightStanding = 1.35f;
	static constexpr float	cCharacterRadiusStanding = 0.3f;
	static constexpr float	cInnerShapeFraction = 0.9f;
	static constexpr float	cStairsStepHeight = 0.3f;

	float					mTime = 0.0f;
	uint64					mHash = 0;
	Array<BodyCreationSettings> mWorld;
	Array<Ref<CharacterVirtual>> mCharacters;
	CharacterVsCharacterCollisionSimple mCharacterVsCharacterCollision;
};
