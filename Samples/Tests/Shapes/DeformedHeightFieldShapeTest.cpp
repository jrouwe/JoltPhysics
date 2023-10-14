// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Shapes/DeformedHeightFieldShapeTest.h>
#include <Math/Perlin.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(DeformedHeightFieldShapeTest)
{
	JPH_ADD_BASE_CLASS(DeformedHeightFieldShapeTest, Test)
}

void DeformedHeightFieldShapeTest::Initialize()
{
	const float cCellSize = 1.0f;
	const float cMaxHeight = 2.5f;

	// Create height samples
	mHeightSamples.resize(cSampleCount * cSampleCount);
	for (int y = 0; y < cSampleCount; ++y)
		for (int x = 0; x < cSampleCount; ++x)
			mHeightSamples[y * cSampleCount + x] = cMaxHeight * PerlinNoise3(float(x) * 8.0f / cSampleCount, 0, float(y) * 8.0f / cSampleCount, 256, 256, 256);

	// Determine scale and offset
	Vec3 offset(-0.5f * cCellSize * cSampleCount, 0, -0.5f * cCellSize * cSampleCount);
	Vec3 scale(cCellSize, 1.0f, cCellSize);

	// Create height field
	HeightFieldShapeSettings settings(mHeightSamples.data(), offset, scale, cSampleCount);
	settings.mBlockSize = cBlockSize;
	settings.mBitsPerSample = 8;
	settings.mMinHeightValue = -15.0f;
	mHeightField = static_cast<HeightFieldShape *>(settings.Create().Get().GetPtr());
	mTerrainID = mBodyInterface->CreateAndAddBody(BodyCreationSettings(mHeightField, RVec3::sZero(), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);

	// Spheres on top of the terrain
	RefConst<Shape> sphere_shape = new SphereShape(2.0f);
	for (int z = 0; z < 11; ++z)
		for (int x = 0; x < 11; ++x)
		{
			BodyCreationSettings bcs(sphere_shape, RVec3(-50.0f + 10.0f * x, 2.0f + cMaxHeight, -50.0f + 10.0f * z), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
			bcs.mAllowSleeping = false;
			mBodyInterface->CreateAndAddBody(bcs, EActivation::Activate);
		}
}

void DeformedHeightFieldShapeTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	constexpr float cSpeedScale = 2.0f;
	constexpr float cRadiusX = 60.0f;
	constexpr float cRadiusY = 30.0f;
	constexpr float cBumpRadius = 4.0f;
	constexpr float cBumpHeight = 1.0f;
	constexpr float cFallOff = 0.1f;
	constexpr float cAngularSpeed = 2.0f;
	constexpr float cDisplacementSpeed = 10.0f;
	constexpr int cBlockMask = cBlockSize - 1;

	// Calculate center of bump
	float time = cSpeedScale * mTime;
	float fall_off = exp(-cFallOff * time);
	float angle = cAngularSpeed * time;
	float center_x = cRadiusX * Cos(angle) * fall_off + 64.0f;
	float center_y = cRadiusY * Sin(angle) * fall_off + cDisplacementSpeed * time;
	mTime += inParams.mDeltaTime;

	// Calculate affected area
	int sx = max((int)floor(center_x - cBumpRadius) & ~cBlockMask, 0);
	int sy = max((int)floor(center_y - cBumpRadius) & ~cBlockMask, 0);
	int sx_end = min(((int)ceil(center_x + cBumpRadius) + cBlockMask) & ~cBlockMask, cSampleCount);
	int sy_end = min(((int)ceil(center_y + cBumpRadius) + cBlockMask) & ~cBlockMask, cSampleCount);
	int cx = sx_end - sx;
	int cy = sy_end - sy;

	if (cx > 0 && cy > 0)
	{
		// Remember COM before we change the height field
		Vec3 old_com = mHeightField->GetCenterOfMass();

		constexpr float cHalfPi = 0.5f * JPH_PI;
		auto bump_shape = [=](float inDistance) { return Cos(min(abs(inDistance) * cHalfPi / cBumpRadius, cHalfPi)); };

		// Update the height field
		for (int y = 0; y < cy; ++y)
			for (int x = 0; x < cx; ++x)
			{
				float delta = bump_shape(float(sx) + x - center_x) * bump_shape(float(sy) + y - center_y) * cBumpHeight;
				mHeightSamples[(sy + y) * cSampleCount + sx + x] -= delta;
			}
		mHeightField->SetHeights(sx, sy, cx, cy, mHeightSamples.data() + sy * cSampleCount + sx, cSampleCount, *mTempAllocator);

		// Notify the shape that it has changed its bounding box
		mBodyInterface->NotifyShapeChanged(mTerrainID, old_com, false, EActivation::DontActivate);
	}
}
