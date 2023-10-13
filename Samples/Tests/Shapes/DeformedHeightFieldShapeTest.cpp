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
	mBodyInterface->CreateAndAddBody(BodyCreationSettings(mHeightField, RVec3::sZero(), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);

	// Update the height field
	Array<float> patched_heights;
	patched_heights.resize(32 * 32);
	for (int y = 0; y < 32; ++y)
		for (int x = 0; x < 32; ++x)
			patched_heights[y * 32 + x] = 0.1f * x + 0.2f * y;
	mHeightField->SetHeights(4, 4, 32, 32, patched_heights.data());

	// Verify that the update succeeded
	Array<float> verification;
	verification.resize(32 * 32);
	mHeightField->GetHeights(4, 4, 32, 32, verification.data());
	float delta = 0.0f;
	for (int i = 0; i < 32 * 32; ++i)
		delta = max(delta, abs(verification[i] - patched_heights[i]));
	JPH_ASSERT(delta < 2.0e-3f);
}

void DeformedHeightFieldShapeTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	// TODO: Deform over time
}
