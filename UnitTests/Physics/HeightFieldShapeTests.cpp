// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include "PhysicsTestContext.h"
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/Shape/HeightFieldShape.h>
#include <Jolt/Physics/Collision/PhysicsMaterialSimple.h>

TEST_SUITE("HeightFieldShapeTests")
{
	static void sRandomizeMaterials(HeightFieldShapeSettings &ioSettings, uint inMaxMaterials)
	{
		// Create materials
		for (uint i = 0; i < inMaxMaterials; ++i)
			ioSettings.mMaterials.push_back(new PhysicsMaterialSimple("Material " + ConvertToString(i), Color::sGetDistinctColor(i)));
		
		if (inMaxMaterials > 1)
		{
			// Make random material indices
			UnitTestRandom random;
			uniform_int_distribution<uint> index_distribution(0, inMaxMaterials - 1);
			ioSettings.mMaterialIndices.resize(Square(ioSettings.mSampleCount - 1));
			for (uint y = 0; y < ioSettings.mSampleCount - 1; ++y)
				for (uint x = 0; x < ioSettings.mSampleCount - 1; ++x)
					ioSettings.mMaterialIndices[y * (ioSettings.mSampleCount - 1) + x] = uint8(index_distribution(random));
		}
	}

	static Ref<HeightFieldShape> sValidateGetPosition(const HeightFieldShapeSettings &inSettings, float inMaxError)
	{
		// Create shape
		Ref<HeightFieldShape> shape = static_cast<HeightFieldShape *>(inSettings.Create().Get().GetPtr());

		// Validate it
		float max_diff = -1.0f;
		for (uint y = 0; y < inSettings.mSampleCount; ++y)
			for (uint x = 0; x < inSettings.mSampleCount; ++x)
			{
				// Perform a raycast from above the height field on this location
				RayCast ray { inSettings.mOffset + inSettings.mScale * Vec3((float)x, 100.0f, (float)y), inSettings.mScale.GetY() * Vec3(0, -200, 0) };
				RayCastResult hit;
				shape->CastRay(ray, SubShapeIDCreator(), hit);

				// Get original (unscaled) height
				float height = inSettings.mHeightSamples[y * inSettings.mSampleCount + x];
				if (height != HeightFieldShapeConstants::cNoCollisionValue)
				{
					// Check there is collision
					CHECK(!shape->IsNoCollision(x, y));

					// Calculate position
					Vec3 original_pos = inSettings.mOffset + inSettings.mScale * Vec3((float)x, height, (float)y);

					// Calculate position from the shape
					Vec3 shape_pos = shape->GetPosition(x, y);

					// Calculate delta
					float diff = (original_pos - shape_pos).Length();
					max_diff = max(max_diff, diff);

					// Materials are defined on the triangle, not on the sample points
					if (x < inSettings.mSampleCount - 1 && y < inSettings.mSampleCount - 1)
					{
						const PhysicsMaterial *m1 = PhysicsMaterial::sDefault;
						if (!inSettings.mMaterialIndices.empty())
							m1 = inSettings.mMaterials[inSettings.mMaterialIndices[y * (inSettings.mSampleCount - 1) + x]];
						else if (!inSettings.mMaterials.empty())
							m1 = inSettings.mMaterials.front();

						const PhysicsMaterial *m2 = shape->GetMaterial(x, y);
						CHECK(m1 == m2);
					}

					// Don't test borders, the ray may or may not hit
					if (x > 0 && y > 0 && x < inSettings.mSampleCount - 1 && y < inSettings.mSampleCount - 1)
					{
						// Check that the ray hit the height field
						Vec3 hit_pos = ray.mOrigin + ray.mDirection * hit.mFraction;
						CHECK_APPROX_EQUAL(hit_pos, shape_pos, 1.0e-3f);
					}
				}
				else
				{
					// Should be no collision here
					CHECK(shape->IsNoCollision(x, y));

					// Ray should not have given a hit
					CHECK(hit.mFraction > 1.0f);
				}
			}

		// Check error
		CHECK(max_diff <= inMaxError);

		return shape;
	}

	TEST_CASE("TestPlane")
	{
		// Create flat plane with offset and scale
		HeightFieldShapeSettings settings;
		settings.mOffset = Vec3(3, 5, 7);
		settings.mScale = Vec3(9, 13, 17);
		settings.mSampleCount = 32;
		settings.mBitsPerSample = 1;
		settings.mBlockSize = 4;
		settings.mHeightSamples.resize(Square(settings.mSampleCount));
		for (float &h : settings.mHeightSamples)
			h = 1.0f;

		// Make some random holes
		UnitTestRandom random;
		uniform_int_distribution<uint> index_distribution(0, (uint)settings.mHeightSamples.size() - 1);
		for (int i = 0; i < 10; ++i)
			settings.mHeightSamples[index_distribution(random)] = HeightFieldShapeConstants::cNoCollisionValue;

		// We should be able to encode a flat plane in 1 bit
		CHECK(settings.CalculateBitsPerSampleForError(0.0f) == 1);

		sRandomizeMaterials(settings, 256);
		sValidateGetPosition(settings, 0.0f);
	}
	
	TEST_CASE("TestPlaneCloseToOrigin")
	{
		// Create flat plane very close to origin, this tests that we don't introduce a quantization error on a flat plane
		HeightFieldShapeSettings settings;
		settings.mSampleCount = 32;
		settings.mBitsPerSample = 1;
		settings.mBlockSize = 4;
		settings.mHeightSamples.resize(Square(settings.mSampleCount));
		for (float &h : settings.mHeightSamples)
			h = 1.0e-6f;

		// We should be able to encode a flat plane in 1 bit
		CHECK(settings.CalculateBitsPerSampleForError(0.0f) == 1);

		sRandomizeMaterials(settings, 50);
		sValidateGetPosition(settings, 0.0f);
	}

	TEST_CASE("TestRandomHeightField")
	{
		const float cMinHeight = -5.0f;
		const float cMaxHeight = 10.0f;

		UnitTestRandom random;
		uniform_real_distribution<float> height_distribution(cMinHeight, cMaxHeight);

		// Create height field with random samples
		HeightFieldShapeSettings settings;
		settings.mOffset = Vec3(0.3f, 0.5f, 0.7f);
		settings.mScale = Vec3(1.1f, 1.2f, 1.3f);
		settings.mSampleCount = 32;
		settings.mBitsPerSample = 8;
		settings.mBlockSize = 4;
		settings.mHeightSamples.resize(Square(settings.mSampleCount));
		for (float &h : settings.mHeightSamples)
			h = height_distribution(random);

		// Check if bits per sample is ok
		for (uint32 bits_per_sample = 1; bits_per_sample <= 8; ++bits_per_sample)
		{
			// Calculate maximum error you can get if you quantize using bits_per_sample.
			// We ignore the fact that we have range blocks that give much better compression, although
			// with random input data there shouldn't be much benefit of that.
			float max_error = 0.5f * (cMaxHeight - cMinHeight) / ((1 << bits_per_sample) - 1);
			uint32 calculated_bits_per_sample = settings.CalculateBitsPerSampleForError(max_error);
			CHECK(calculated_bits_per_sample <= bits_per_sample);
		}

		sRandomizeMaterials(settings, 1);
		sValidateGetPosition(settings, settings.mScale.GetY() * (cMaxHeight - cMinHeight) / ((1 << settings.mBitsPerSample) - 1));
	}

	TEST_CASE("TestEmptyHeightField")
	{
		// Create height field with no collision
		HeightFieldShapeSettings settings;
		settings.mSampleCount = 32;
		settings.mHeightSamples.resize(Square(settings.mSampleCount));
		for (float &h : settings.mHeightSamples)
			h = HeightFieldShapeConstants::cNoCollisionValue;

		// This should use the minimum amount of bits
		CHECK(settings.CalculateBitsPerSampleForError(0.0f) == 1);

		sRandomizeMaterials(settings, 50);
		Ref<HeightFieldShape> shape = sValidateGetPosition(settings, 0.0f);

		// Check that we allocated the minimum amount of memory
		Shape::Stats stats = shape->GetStats();
		CHECK(stats.mNumTriangles == 0);
		CHECK(stats.mSizeBytes == sizeof(HeightFieldShape));
	}
}
