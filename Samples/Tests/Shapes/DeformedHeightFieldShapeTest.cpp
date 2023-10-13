// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Shapes/DeformedHeightFieldShapeTest.h>
#include <Math/Perlin.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(DeformedHeightFieldShapeTest)
{
	JPH_ADD_BASE_CLASS(DeformedHeightFieldShapeTest, Test)
}

void DeformedHeightFieldShapeTest::Initialize()
{
	const int n = 128;
	const float cell_size = 1.0f;
	const float max_height = 5.0f;

	// Create height samples
	Array<float> height_samples;
	height_samples.resize(n * n);
	for (int y = 0; y < n; ++y)
		for (int x = 0; x < n; ++x)
			height_samples[y * n + x] = max_height * PerlinNoise3(float(x) * 8.0f / n, 0, float(y) * 8.0f / n, 256, 256, 256);

	// Determine scale and offset (deliberately apply extra offset and scale in Y direction)
	Vec3 offset(-0.5f * cell_size * n, -2.0f, -0.5f * cell_size * n);
	Vec3 scale(cell_size, 1.5f, cell_size);

	// Create height field
	HeightFieldShapeSettings settings(height_samples.data(), offset, scale, n);
	settings.mBlockSize = 4;
	settings.mBitsPerSample = 8;
	settings.mMinHeightValue = -10.0f;
	settings.mMaxHeightValue = 10.0f;
	mHeightField = static_cast<HeightFieldShape *>(settings.Create().Get().GetPtr());
	BodyID terrain_id = mBodyInterface->CreateAndAddBody(BodyCreationSettings(mHeightField, RVec3::sZero(), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);

	// Remember COM before we change the height field
	Vec3 old_com = mHeightField->GetCenterOfMass();

	// Update the height field
	int s = 32, sx = 4, sy = 8;
	Array<float> patched_heights;
	patched_heights.resize(s * s);
	for (int y = 0; y < s; ++y)
		for (int x = 0; x < s; ++x)
			patched_heights[y * s + x] = 0.1f * (x - sx) + 0.2f * (y - sy);
	mHeightField->SetHeights(sx, sy, s, s, patched_heights.data(), s, *mTempAllocator);

	// Notify the shape that it has changed its bounding box
	mBodyInterface->NotifyShapeChanged(terrain_id, old_com, false, EActivation::DontActivate);

	// Verify that the update succeeded
	Array<float> verification;
	verification.resize(s * s);
	mHeightField->GetHeights(sx, sy, s, s, verification.data(), s);
	float delta = 0.0f;
	for (int i = 0; i < s * s; ++i)
		delta = max(delta, abs(verification[i] - patched_heights[i]));
	JPH_ASSERT(delta < 0.02f);
}

void DeformedHeightFieldShapeTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	// TODO: Deform over time
}
