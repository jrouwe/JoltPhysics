// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Shapes/HeightFieldShapeTest.h>
#include <External/Perlin.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/PhysicsMaterialSimple.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Application/DebugUI.h>
#include <Utils/ReadData.h>
#include <Utils/Log.h>
#include <Utils/DebugRendererSP.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(HeightFieldShapeTest)
{
	JPH_ADD_BASE_CLASS(HeightFieldShapeTest, Test)
}

static int sTerrainType = 0;

static const char *sTerrainTypes[] = {
	"Procedural Terrain 2^N",
	"Procedural Terrain 2^N + 1",
	"Heightfield 1",
	"Flat 2^N",
	"Flat 2^N + 1",
	"No Collision 2^N",
	"No Collision 2^N + 1"
};

void HeightFieldShapeTest::Initialize()
{
	if (sTerrainType == 0 || sTerrainType == 1)
	{
		const int n = sTerrainType == 0? 128 : 129;
		const float cell_size = 1.0f;
		const float max_height = 5.0f;

		// Create height samples
		mTerrainSize = n;
		mTerrain.resize(n * n);
		for (int y = 0; y < n; ++y)
			for (int x = 0; x < n; ++x)
				mTerrain[y * n + x] = max_height * PerlinNoise3(float(x) * 8.0f / n, 0, float(y) * 8.0f / n, 256, 256, 256);

		// Make some holes
		mTerrain[2 * n + 2] = HeightFieldShapeConstants::cNoCollisionValue;
		for (int y = 4; y < 33; ++y)
			for (int x = 4; x < 33; ++x)
				mTerrain[y * n + x] = HeightFieldShapeConstants::cNoCollisionValue;

		// Make material indices
		uint8 max_material_index = 0;
		mMaterialIndices.resize(Square(n - 1));
		for (int y = 0; y < n - 1; ++y)
			for (int x = 0; x < n - 1; ++x)
			{
				uint8 material_index = uint8(round((Vec3(x * cell_size, 0, y * cell_size) - Vec3(n * cell_size / 2, 0, n * cell_size / 2)).Length() / 10.0f));
				max_material_index = max(max_material_index, material_index);
				mMaterialIndices[y * (n - 1) + x] = material_index;
			}

		// Mark the corners to validate that materials and heights match
		mTerrain[0] = 0.0f;
		mTerrain[n - 1] = 10.0f;
		mTerrain[(n - 1) * n] = 20.0f;
		mTerrain[n * n - 1] = 30.0f;
		mMaterialIndices[0] = 0;
		mMaterialIndices[n - 2] = 1;
		mMaterialIndices[(n - 2) * (n - 1)] = 2;
		mMaterialIndices[Square(n - 1) - 1] = 3;

		// Create materials
		for (uint8 i = 0; i <= max_material_index; ++i)
			mMaterials.push_back(new PhysicsMaterialSimple("Material " + ConvertToString(uint(i)), Color::sGetDistinctColor(i)));

		// Determine scale and offset (deliberately apply extra offset and scale in Y direction)
		mTerrainOffset = Vec3(-0.5f * cell_size * n, -2.0f, -0.5f * cell_size * n);
		mTerrainScale = Vec3(cell_size, 1.5f, cell_size);
	}
	else if (sTerrainType == 2)
	{
		const int n = 1024;
		const float cell_size = 0.5f;

		// Get height samples
		Array<uint8> data = ReadData("heightfield1.bin");
		if (data.size() != sizeof(float) * n * n)
			FatalError("Invalid file size");
		mTerrainSize = n;
		mTerrain.resize(n * n);
		memcpy(mTerrain.data(), data.data(), n * n * sizeof(float));

		// Determine scale and offset
		mTerrainOffset = Vec3(-0.5f * cell_size * n, 0.0f, -0.5f * cell_size * n);
		mTerrainScale = Vec3(cell_size, 1.0f, cell_size);
	}
	else if (sTerrainType == 3 || sTerrainType == 4)
	{
		const int n = sTerrainType == 3? 128 : 129;
		const float cell_size = 1.0f;
		const float height = JPH_PI;

		// Determine scale and offset
		mTerrainOffset = Vec3(-0.5f * cell_size * n, 0.0f, -0.5f * cell_size * n);
		mTerrainScale = Vec3(cell_size, 1.0f, cell_size);

		// Mark the entire terrain as single height
		mTerrainSize = n;
		mTerrain.resize(n * n);
		for (float &v : mTerrain)
			v = height;
	}
	else if (sTerrainType == 5 || sTerrainType == 6)
	{
		const int n = sTerrainType == 4? 128 : 129;
		const float cell_size = 1.0f;

		// Determine scale and offset
		mTerrainOffset = Vec3(-0.5f * cell_size * n, 0.0f, -0.5f * cell_size * n);
		mTerrainScale = Vec3(cell_size, 1.0f, cell_size);

		// Mark the entire terrain as no collision
		mTerrainSize = n;
		mTerrain.resize(n * n);
		for (float &v : mTerrain)
			v = HeightFieldShapeConstants::cNoCollisionValue;
	}

	// Create height field
	HeightFieldShapeSettings settings(mTerrain.data(), mTerrainOffset, mTerrainScale, mTerrainSize, mMaterialIndices.data(), mMaterials);
	settings.mBlockSize = 1 << sBlockSizeShift;
	settings.mBitsPerSample = sBitsPerSample;
	mHeightField = StaticCast<HeightFieldShape>(settings.Create().Get());
	mBodyInterface->CreateAndAddBody(BodyCreationSettings(mHeightField, RVec3::sZero(), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);

	// Validate it
	float max_diff = -1.0f;
	uint max_diff_x = 0, max_diff_y = 0;
	float min_height = FLT_MAX, max_height = -FLT_MAX, avg_diff = 0.0f;
	for (uint y = 0; y < mTerrainSize; ++y)
		for (uint x = 0; x < mTerrainSize; ++x)
		{
			float h1 = mTerrain[y * mTerrainSize + x];
			if (h1 != HeightFieldShapeConstants::cNoCollisionValue)
			{
				h1 = mTerrainOffset.GetY() + mTerrainScale.GetY() * h1;
				if (mHeightField->IsNoCollision(x, y))
					FatalError("No collision where there should be");
				float h2 = mHeightField->GetPosition(x, y).GetY();
				float diff = abs(h2 - h1);
				if (diff > max_diff)
				{
					max_diff = diff;
					max_diff_x = x;
					max_diff_y = y;
				}
				min_height = min(min_height, h1);
				max_height = max(max_height, h1);
				avg_diff += diff;
			}
			else
			{
				if (!mHeightField->IsNoCollision(x, y))
					FatalError("Collision where there shouldn't be");
			}
		}

	// Calculate relative error
	float rel_error = min_height < max_height? 100.0f * max_diff / (max_height - min_height) : 0.0f;

	// Max error we expect given sBitsPerSample (normally the error should be much lower because we quantize relative to the block rather than the full height)
	float max_error = 0.5f * 100.0f / ((1 << sBitsPerSample) - 1);

	// Calculate average
	avg_diff /= mTerrainSize * mTerrainSize;

	// Calculate amount of memory used
	Shape::Stats stats = mHeightField->GetStats();

	// Trace stats
	Trace("Block size: %d, bits per sample: %d, min height: %g, max height: %g, avg diff: %g, max diff: %g at (%d, %d), relative error: %g%%, size: %u bytes", 1 << sBlockSizeShift, sBitsPerSample, (double)min_height, (double)max_height, (double)avg_diff, (double)max_diff, max_diff_x, max_diff_y, (double)rel_error, stats.mSizeBytes);
	if (rel_error > max_error)
		FatalError("Error too big!");

	// Determine terrain height
	RayCastResult result;
	RVec3 start(0, 1000, 0);
	Vec3 direction(0, -2000, 0);
	RRayCast ray { start, direction };
	if (mPhysicsSystem->GetNarrowPhaseQuery().CastRay(ray, result, SpecifiedBroadPhaseLayerFilter(BroadPhaseLayers::NON_MOVING), SpecifiedObjectLayerFilter(Layers::NON_MOVING)))
		mHitPos = ray.GetPointOnRay(result.mFraction);

	// Dynamic body
	mBodyInterface->CreateAndAddBody(BodyCreationSettings(new BoxShape(Vec3(0.5f, 1.0f, 2.0f)), mHitPos + Vec3(0, 10, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING), EActivation::Activate);
}

void HeightFieldShapeTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	// Test the 'GetHeight' function and draw a marker on the surface
	Vec3 test_pos = Vec3(inParams.mCameraState.mPos) + 10.0f * inParams.mCameraState.mForward, surface_pos;
	SubShapeID sub_shape_id;
	if (mHeightField->ProjectOntoSurface(test_pos, surface_pos, sub_shape_id))
	{
		Vec3 surface_normal = mHeightField->GetSurfaceNormal(sub_shape_id, surface_pos);
		DrawMarkerSP(mDebugRenderer, surface_pos, Color::sWhite, 1.0f);
		DrawArrowSP(mDebugRenderer, surface_pos, surface_pos + surface_normal, Color::sRed, 0.1f);
	}

	// Draw the original uncompressed terrain
	if (sShowOriginalTerrain)
		for (uint y = 0; y < mTerrainSize; ++y)
			for (uint x = 0; x < mTerrainSize; ++x)
			{
				// Get original height
				float h = mTerrain[y * mTerrainSize + x];
				if (h == HeightFieldShapeConstants::cNoCollisionValue)
					continue;

				// Get original position
				Vec3 original = mTerrainOffset + mTerrainScale * Vec3(float(x), h, float(y));

				// Get compressed position
				Vec3 compressed = mHeightField->GetPosition(x, y);

				// Draw marker that is red when error is too big and green when not
				const float cMaxError = 0.1f;
				float error = (original - compressed).Length();
				uint8 c = uint8(round(255.0f * min(error / cMaxError, 1.0f)));
				DrawMarkerSP(mDebugRenderer, original, Color(c, 255 - c, 0, 255), 0.1f);
			}
}

void HeightFieldShapeTest::GetInitialCamera(CameraState &ioState) const
{
	// Correct camera pos for hit position
	ioState.mPos += mHitPos;
}

void HeightFieldShapeTest::CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu)
{
	inUI->CreateTextButton(inSubMenu, "Select Terrain", [this, inUI]() {
		UIElement *terrain_name = inUI->CreateMenu();
		for (uint i = 0; i < size(sTerrainTypes); ++i)
			inUI->CreateTextButton(terrain_name, sTerrainTypes[i], [this, i]() { sTerrainType = i; RestartTest(); });
		inUI->ShowMenu(terrain_name);
	});

	inUI->CreateTextButton(inSubMenu, "Configuration Settings", [this, inUI]() {
		UIElement *terrain_settings = inUI->CreateMenu();
		inUI->CreateComboBox(terrain_settings, "Block Size", { "2", "4", "8" }, sBlockSizeShift - 1, [=](int inItem) { sBlockSizeShift = inItem + 1; });
		inUI->CreateSlider(terrain_settings, "Bits Per Sample", (float)sBitsPerSample, 1.0f, 8.0f, 1.0f, [=](float inValue) { sBitsPerSample = (int)inValue; });
		inUI->CreateTextButton(terrain_settings, "Accept", [this]() { RestartTest(); });
		inUI->ShowMenu(terrain_settings);
	});

	inUI->CreateCheckBox(inSubMenu, "Show Original Terrain", sShowOriginalTerrain, [](UICheckBox::EState inState) { sShowOriginalTerrain = inState == UICheckBox::STATE_CHECKED; });
}

