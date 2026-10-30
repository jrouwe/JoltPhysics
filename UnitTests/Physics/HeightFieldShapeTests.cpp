// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include "PhysicsTestContext.h"
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/Shape/HeightFieldShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/CollisionDispatch.h>
#include <Jolt/Physics/Collision/CollideShape.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Jolt/Physics/Collision/PhysicsMaterialSimple.h>
#include <Jolt/Physics/Collision/CollideSoftBodyVertexIterator.h>

TEST_SUITE("HeightFieldShapeTests")
{
	TEST_CASE("TestHeightFieldCreateErrors")
	{
		// Creates valid settings that can be modified to trigger an error
		auto make_settings = []() {
			HeightFieldShapeSettings settings;
			settings.mSampleCount = 32;
			settings.mBlockSize = 2;
			settings.mBitsPerSample = 8;
			settings.mHeightSamples.resize(Square(settings.mSampleCount), 0.0f);
			return settings;
		};

		// Sanity check that the unmodified settings are valid
		CHECK(!make_settings().Create().HasError());

		// Block size out of range
		for (uint32 block_size : { 1u, 9u })
		{
			HeightFieldShapeSettings settings = make_settings();
			settings.mBlockSize = block_size;
			CHECK(settings.Create().HasError());
		}

		// Bits per sample out of range
		for (uint32 bits_per_sample : { 0u, HeightFieldShapeConstants::cMaxBitsPerSample + 1 })
		{
			HeightFieldShapeSettings settings = make_settings();
			settings.mBitsPerSample = bits_per_sample;
			CHECK(settings.Create().HasError());
		}

		// Sample count too low, we need at least 2 blocks
		{
			HeightFieldShapeSettings settings = make_settings();
			settings.mSampleCount = 2;
			settings.mHeightSamples.resize(Square(settings.mSampleCount));
			CHECK(settings.Create().HasError());
		}

		// Sample count too high, the number of blocks doesn't fit in the properties of a range block.
		{
			HeightFieldShapeSettings settings = make_settings();
			settings.mSampleCount = settings.mBlockSize * ((1 << HeightFieldShapeConstants::cNumBitsXY) + 1);
			// Samples are not actually read before error and allocating them causes WASM to go out of memory
			CHECK(settings.Create().HasError());
		}

		// Sample count too high to fit the X and Y coordinates in a sub shape ID.
		// With a block size of 8, the number of blocks is still small enough to fit in the range block properties.
		{
			HeightFieldShapeSettings settings = make_settings();
			settings.mBlockSize = 8;
			settings.mSampleCount = (1 << ((SubShapeID::MaxBits - 1) / 2)) + settings.mBlockSize; // 1 bit for triangle, equal amount of bits for X and Y
			CHECK(settings.Create().HasError());
		}

		// Too many materials
		{
			HeightFieldShapeSettings settings = make_settings();
			for (uint i = 0; i < 257; ++i)
				settings.mMaterials.push_back(new PhysicsMaterialSimple("Material " + ConvertToString(i), Color::sGetDistinctColor(i)));
			CHECK(settings.Create().HasError());
		}

		// Material index beyond the material list
		{
			HeightFieldShapeSettings settings = make_settings();
			settings.mMaterials.push_back(new PhysicsMaterialSimple("Material 0", Color::sGetDistinctColor(0)));
			settings.mMaterialIndices.resize(Square(settings.mSampleCount - 1), 0);
			settings.mMaterialIndices[10] = 1;
			CHECK(settings.Create().HasError());
		}

		// Material indices without materials
		{
			HeightFieldShapeSettings settings = make_settings();
			settings.mMaterialIndices.resize(Square(settings.mSampleCount - 1), 0);
			CHECK(settings.Create().HasError());
		}
	}

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
		Ref<HeightFieldShape> shape = StaticCast<HeightFieldShape>(inSettings.Create().Get());

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
						Vec3 hit_pos = ray.GetPointOnRay(hit.mFraction);
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

		// Check if bits per sample is ok
		for (uint32 bits_per_sample = 1; bits_per_sample <= HeightFieldShapeConstants::cMaxBitsPerSample; ++bits_per_sample)
		{
			UnitTestRandom random;
			uniform_real_distribution<float> height_distribution(cMinHeight, cMaxHeight);

			// Create height field with random samples
			HeightFieldShapeSettings settings;
			settings.mOffset = Vec3(0.3f, 0.5f, 0.7f);
			settings.mScale = Vec3(1.1f, 1.2f, 1.3f);
			settings.mSampleCount = 32;
			settings.mBitsPerSample = bits_per_sample;
			settings.mBlockSize = 4;
			settings.mHeightSamples.resize(Square(settings.mSampleCount));
			for (float &h : settings.mHeightSamples)
				h = height_distribution(random);

			// Calculate maximum error you can get if you quantize using bits_per_sample.
			// We ignore the fact that we have range blocks that give much better compression, although
			// with random input data there shouldn't be much benefit of that.
			float max_error = 0.5f * (cMaxHeight - cMinHeight) / ((1 << bits_per_sample) - 1);
			uint32 calculated_bits_per_sample = settings.CalculateBitsPerSampleForError(max_error);
			CHECK(calculated_bits_per_sample <= bits_per_sample);

			sRandomizeMaterials(settings, 1);
			sValidateGetPosition(settings, settings.mScale.GetY() * (cMaxHeight - cMinHeight) / ((1 << settings.mBitsPerSample) - 1));
		}
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

	TEST_CASE("TestGetHeights")
	{
		const float cMinHeight = -5.0f;
		const float cMaxHeight = 10.0f;
		const uint cSampleCount = 32;
		const uint cNoCollisionIndex = 10;

		UnitTestRandom random;
		uniform_real_distribution<float> height_distribution(cMinHeight, cMaxHeight);

		// Create height field with random samples
		HeightFieldShapeSettings settings;
		settings.mOffset = Vec3(0.3f, 0.5f, 0.7f);
		settings.mScale = Vec3(1.1f, 1.2f, 1.3f);
		settings.mSampleCount = cSampleCount;
		settings.mBitsPerSample = 8;
		settings.mBlockSize = 4;
		settings.mHeightSamples.resize(Square(cSampleCount));
		for (float &h : settings.mHeightSamples)
			h = height_distribution(random);

		// Add 1 sample that has no collision
		settings.mHeightSamples[cNoCollisionIndex] = HeightFieldShapeConstants::cNoCollisionValue;

		// Create shape
		ShapeRefC shape = settings.Create().Get();
		const HeightFieldShape *height_field = StaticCast<HeightFieldShape>(shape);

		{
			// Check that the GetHeights function returns the same values as the original height samples
			Array<float> sampled_heights;
			sampled_heights.resize(Square(cSampleCount));
			height_field->GetHeights(0, 0, cSampleCount, cSampleCount, sampled_heights.data(), cSampleCount);
			for (uint i = 0; i < Square(cSampleCount); ++i)
				if (i == cNoCollisionIndex)
					CHECK(sampled_heights[i] == HeightFieldShapeConstants::cNoCollisionValue);
				else
					CHECK_APPROX_EQUAL(sampled_heights[i], settings.mOffset.GetY() + settings.mScale.GetY() * settings.mHeightSamples[i], 0.05f);
		}

		{
			// With a random height field the max error is going to be limited by the amount of bits we have per sample as we will not get any benefit from a reduced range per block
			float tolerance = (cMaxHeight - cMinHeight) / ((1 << settings.mBitsPerSample) - 2);

			// Check a sub rect of the height field
			uint sx = 4, sy = 8, cx = 16, cy = 8;
			Array<float> sampled_heights;
			sampled_heights.resize(cx * cy);
			height_field->GetHeights(sx, sy, cx, cy, sampled_heights.data(), cx);
			for (uint y = 0; y < cy; ++y)
				for (uint x = 0; x < cx; ++x)
					CHECK_APPROX_EQUAL(sampled_heights[y * cx + x], settings.mOffset.GetY() + settings.mScale.GetY() * settings.mHeightSamples[(sy + y) * cSampleCount + sx + x], tolerance);
		}
	}

	TEST_CASE("TestSetHeights")
	{
		const float cMinHeight = -5.0f;
		const float cMaxHeight = 10.0f;
		const uint cSampleCount = 32;

		UnitTestRandom random;
		uniform_real_distribution<float> height_distribution(cMinHeight, cMaxHeight);

		// Create height field with random samples
		HeightFieldShapeSettings settings;
		settings.mOffset = Vec3(0.3f, 0.5f, 0.7f);
		settings.mScale = Vec3(1.1f, 1.2f, 1.3f);
		settings.mSampleCount = cSampleCount;
		settings.mBitsPerSample = 8;
		settings.mBlockSize = 4;
		settings.mHeightSamples.resize(Square(cSampleCount));
		settings.mMinHeightValue = cMinHeight;
		settings.mMaxHeightValue = cMaxHeight;
		for (float &h : settings.mHeightSamples)
			h = height_distribution(random);

		// Create shape
		Ref<Shape> shape = settings.Create().Get();
		HeightFieldShape *height_field = StaticCast<HeightFieldShape>(shape);

		// Get the original (quantized) heights
		Array<float> original_heights;
		original_heights.resize(Square(cSampleCount));
		height_field->GetHeights(0, 0, cSampleCount, cSampleCount, original_heights.data(), cSampleCount);

		// Create new data for height field
		Array<float> patched_heights;
		uint sx = 4, sy = 16, cx = 16, cy = 8;
		patched_heights.resize(cx * cy);
		for (uint y = 0; y < cy; ++y)
			for (uint x = 0; x < cx; ++x)
				patched_heights[y * cx + x] = height_distribution(random);

		// Add 1 sample that has no collision
		uint no_collision_idx = (sy + 1) * cSampleCount + sx + 2;
		patched_heights[1 * cx + 2] = HeightFieldShapeConstants::cNoCollisionValue;

		// Update the height field
		TempAllocatorMalloc temp_allocator;
		height_field->SetHeights(sx, sy, cx, cy, patched_heights.data(), cx, temp_allocator);

		// With a random height field the max error is going to be limited by the amount of bits we have per sample as we will not get any benefit from a reduced range per block
		float tolerance = (cMaxHeight - cMinHeight) / ((1 << settings.mBitsPerSample) - 2);

		// Check a sub rect of the height field
		Array<float> verify_heights;
		verify_heights.resize(cSampleCount * cSampleCount);
		height_field->GetHeights(0, 0, cSampleCount, cSampleCount, verify_heights.data(), cSampleCount);
		for (uint y = 0; y < cSampleCount; ++y)
			for (uint x = 0; x < cSampleCount; ++x)
			{
				uint idx = y * cSampleCount + x;
				if (idx == no_collision_idx)
					CHECK(verify_heights[idx] == HeightFieldShapeConstants::cNoCollisionValue);
				else if (x >= sx && x < sx + cx && y >= sy && y < sy + cy)
					CHECK_APPROX_EQUAL(verify_heights[y * cSampleCount + x], patched_heights[(y - sy) * cx + x - sx], tolerance);
				else if (x >= sx - settings.mBlockSize && x < sx + cx && y >= sy - settings.mBlockSize && y < sy + cy)
					CHECK_APPROX_EQUAL(verify_heights[idx], original_heights[idx], tolerance); // We didn't modify this but it has been quantized again
				else
					CHECK(verify_heights[idx] == original_heights[idx]); // We didn't modify this and it is outside of the affected range
			}
	}

	TEST_CASE("TestSetHeightsNonPowerOf2BlockCount")
	{
		// 20 x 20 samples with a block size of 2 gives 10 blocks per side, which is not a power of 2.
		// The coarser levels of the range block hierarchy are stored at stride 1 << level, only the most
		// detailed level is stored at stride (num_blocks + 1) / 2. SetHeights must update them at the same
		// strides, otherwise the updated ranges land on unrelated nodes and the cells below those nodes
		// stop colliding until something big enough overlaps the wrong range.
		const uint cSampleCount = 20;

		HeightFieldShapeSettings settings;
		settings.mSampleCount = cSampleCount;
		settings.mBlockSize = 2;
		settings.mMinHeightValue = 0.0f;
		settings.mMaxHeightValue = 200.0f;
		settings.mHeightSamples.resize(Square(cSampleCount), 0.0f);

		Ref<Shape> shape = settings.Create().Get();
		HeightFieldShape *height_field = StaticCast<HeightFieldShape>(shape);

		// Raise a block aligned rectangle in one corner of the height field
		const uint sx = 0, sy = 16, cx = 6, cy = 4;
		const float cRaisedHeight = 100.0f;
		Array<float> raised_heights(cx * cy, cRaisedHeight);
		TempAllocatorMalloc temp_allocator;
		height_field->SetHeights(sx, sy, cx, cy, raised_heights.data(), cx, temp_allocator);

		// Heights as the shape quantized them, so the sphere below can be placed on the actual surface
		Array<float> heights(Square(cSampleCount));
		height_field->GetHeights(0, 0, cSampleCount, cSampleCount, heights.data(), cSampleCount);

		// A sphere that dips 0.25 below the highest corner of a cell must collide with that cell.
		// Its bounding box is 1 high, so it only overlaps the range of the node it is really under.
		const float cSphereRadius = 0.5f;
		Ref<Shape> sphere = new SphereShape(cSphereRadius);
		CollideShapeSettings collide_settings;
		auto test_cell = [&](uint inX, uint inY)
		{
			float top = max(max(heights[inY * cSampleCount + inX], heights[inY * cSampleCount + inX + 1]), max(heights[(inY + 1) * cSampleCount + inX], heights[(inY + 1) * cSampleCount + inX + 1]));
			Vec3 sphere_position(float(inX) + 0.5f, top + cSphereRadius - 0.25f, float(inY) + 0.5f);
			AllHitCollisionCollector<CollideShapeCollector> collector;
			CollisionDispatch::sCollideShapeVsShape(sphere, shape, Vec3::sOne(), Vec3::sOne(), Mat44::sTranslation(sphere_position), Mat44::sIdentity(), SubShapeIDCreator(), SubShapeIDCreator(), collide_settings, collector);
			return collector.HadHit();
		};

		// Every cell that was not raised must still collide
		for (uint y = 0; y + 1 < cSampleCount; ++y)
			for (uint x = 0; x + 1 < cSampleCount; ++x)
			{
				// Skip the raised rectangle and the cells that connect to it
				if (x < sx + cx && y + 1 >= sy)
					continue;

				CAPTURE(x);
				CAPTURE(y);
				CHECK(test_cell(x, y));
			}

		// And the raised part must collide at its new height
		CHECK(test_cell(sx + 1, sy + 1));
	}

	TEST_CASE("TestSetMaterials")
	{
		constexpr uint cSampleCount = 32;

		PhysicsMaterialRefC material_0 = new PhysicsMaterialSimple("Material 0", Color::sGetDistinctColor(0));
		PhysicsMaterialRefC material_1 = new PhysicsMaterialSimple("Material 1", Color::sGetDistinctColor(1));
		PhysicsMaterialRefC material_2 = new PhysicsMaterialSimple("Material 2", Color::sGetDistinctColor(2));
		PhysicsMaterialRefC material_3 = new PhysicsMaterialSimple("Material 3", Color::sGetDistinctColor(3));
		PhysicsMaterialRefC material_4 = new PhysicsMaterialSimple("Material 4", Color::sGetDistinctColor(4));
		PhysicsMaterialRefC material_5 = new PhysicsMaterialSimple("Material 5", Color::sGetDistinctColor(5));

		// Create height field with a single material
		HeightFieldShapeSettings settings;
		settings.mSampleCount = cSampleCount;
		settings.mBitsPerSample = 8;
		settings.mBlockSize = 4;
		settings.mHeightSamples.resize(Square(cSampleCount));
		for (float &h : settings.mHeightSamples)
			h = 0.0f;
		settings.mMaterials.push_back(material_0);
		settings.mMaterialIndices.resize(Square(cSampleCount - 1));
		for (uint8 &m : settings.mMaterialIndices)
			m = 0;

		// Store the current state
		Array<const PhysicsMaterial *> current_state;
		current_state.resize(Square(cSampleCount - 1));
		for (const PhysicsMaterial *&m : current_state)
			m = material_0;

		// Create shape
		Ref<Shape> shape = settings.Create().Get();
		HeightFieldShape *height_field = StaticCast<HeightFieldShape>(shape);

		// Check that the material is set
		auto check_materials = [height_field, &current_state]() {
			const PhysicsMaterialList &material_list = height_field->GetMaterialList();

			uint sample_count_min_1 = height_field->GetSampleCount() - 1;

			Array<uint8> material_indices;
			material_indices.resize(Square(sample_count_min_1));
			height_field->GetMaterials(0, 0, sample_count_min_1, sample_count_min_1, material_indices.data(), sample_count_min_1);

			for (uint i = 0; i < (uint)current_state.size(); ++i)
				CHECK(current_state[i] == material_list[material_indices[i]]);
		};
		check_materials();

		// Function to randomize materials
		auto update_materials = [height_field, &current_state](uint inStartX, uint inStartY, uint inSizeX, uint inSizeY, const PhysicsMaterialList *inMaterialList) {
			TempAllocatorMalloc temp_allocator;

			const PhysicsMaterialList &material_list = inMaterialList != nullptr? *inMaterialList : height_field->GetMaterialList();

			UnitTestRandom random;
			uniform_int_distribution<uint> index_distribution(0, uint(material_list.size()) - 1);

			uint sample_count_min_1 = height_field->GetSampleCount() - 1;

			Array<uint8> patched_materials;
			patched_materials.resize(inSizeX * inSizeY);
			for (uint y = 0; y < inSizeY; ++y)
				for (uint x = 0; x < inSizeX; ++x)
				{
					// Initialize the patch
					uint8 index = uint8(index_distribution(random));
					patched_materials[y * inSizeX + x] = index;

					// Update reference state
					current_state[(inStartY + y) * sample_count_min_1 + inStartX + x] = material_list[index];
				}
			CHECK(height_field->SetMaterials(inStartX, inStartY, inSizeX, inSizeY, patched_materials.data(), inSizeX, inMaterialList, temp_allocator));
		};

		{
			// Add material 1
			PhysicsMaterialList patched_materials_list;
			patched_materials_list.push_back(material_1);
			patched_materials_list.push_back(material_0);
			update_materials(4, 16, 16, 8, &patched_materials_list);
			check_materials();
		}

		{
			// Add material 2
			PhysicsMaterialList patched_materials_list;
			patched_materials_list.push_back(material_0);
			patched_materials_list.push_back(material_2);
			update_materials(8, 16, 16, 8, &patched_materials_list);
			check_materials();
		}

		{
			// Add material 3
			PhysicsMaterialList patched_materials_list;
			patched_materials_list.push_back(material_0);
			patched_materials_list.push_back(material_1);
			patched_materials_list.push_back(material_2);
			patched_materials_list.push_back(material_3);
			update_materials(8, 8, 16, 8, &patched_materials_list);
			check_materials();
		}

		{
			// Add material 4
			PhysicsMaterialList patched_materials_list;
			patched_materials_list.push_back(material_0);
			patched_materials_list.push_back(material_1);
			patched_materials_list.push_back(material_4);
			patched_materials_list.push_back(material_2);
			patched_materials_list.push_back(material_3);
			update_materials(0, 0, 30, 30, &patched_materials_list);
			check_materials();
		}

		{
			// Add material 5
			PhysicsMaterialList patched_materials_list;
			patched_materials_list.push_back(material_4);
			patched_materials_list.push_back(material_3);
			patched_materials_list.push_back(material_0);
			patched_materials_list.push_back(material_1);
			patched_materials_list.push_back(material_2);
			patched_materials_list.push_back(material_5);
			update_materials(1, 1, 30, 30, &patched_materials_list);
			check_materials();
		}

		{
			// Update materials without new material list
			update_materials(2, 5, 10, 15, nullptr);
			check_materials();
		}

		// Check materials using GetMaterial call
		for (uint y = 0; y < cSampleCount - 1; ++y)
			for (uint x = 0; x < cSampleCount - 1; ++x)
				CHECK(height_field->GetMaterial(x, y) == current_state[y * (cSampleCount - 1) + x]);
	}

	// Smooth height field with a hole, used to test queries against triangles where the expected result can be calculated analytically
	class SinCosHeightField
	{
	public:
		static constexpr uint	cHoleX = 10;
		static constexpr uint	cHoleY = 13;

		explicit				SinCosHeightField(uint inSampleCount)
		{
			mSettings.mOffset = Vec3(-20.0f, 3.0f, -15.0f);
			mSettings.mScale = Vec3(1.5f, 2.0f, 1.25f);
			mSettings.mSampleCount = inSampleCount;
			mSettings.mBlockSize = 2;
			mSettings.mBitsPerSample = HeightFieldShapeConstants::cMaxBitsPerSample; // Minimize quantization error so we can compare with the analytic surface
			mSettings.mHeightSamples.resize(Square(inSampleCount));
			for (uint y = 0; y < inSampleCount; ++y)
				for (uint x = 0; x < inSampleCount; ++x)
					mSettings.mHeightSamples[y * inSampleCount + x] = IsHole(x, y)? HeightFieldShapeConstants::cNoCollisionValue : sHeight(x, y);

			mShape = StaticCast<HeightFieldShape>(mSettings.Create().Get());
		}

		// Analytic height function
		static float			sHeight(uint inX, uint inY)
		{
			return Sin(0.3f * float(inX)) * Cos(0.25f * float(inY));
		}

		// Check if a sample is the hole
		static bool				IsHole(uint inX, uint inY)
		{
			return inX == cHoleX && inY == cHoleY;
		}

		// Get the position of a sample in the local space of the height field
		Vec3					GetPosition(uint inX, uint inY) const
		{
			return mSettings.mOffset + mSettings.mScale * Vec3(float(inX), sHeight(inX, inY), float(inY));
		}

		// Get the vertices of a triangle, triangle 0 is (x, y), (x, y + 1), (x + 1, y + 1) and triangle 1 is (x, y), (x + 1, y + 1), (x + 1, y)
		// @return False if the triangle has no collision because it touches the hole
		bool					GetTriangle(uint inX, uint inY, uint inTriangle, Vec3 &outV0, Vec3 &outV1, Vec3 &outV2) const
		{
			uint x2 = inTriangle == 0? inX : inX + 1;
			uint y2 = inTriangle == 0? inY + 1 : inY;
			outV0 = GetPosition(inX, inY);
			outV1 = GetPosition(x2, y2);
			outV2 = GetPosition(inX + 1, inY + 1);
			if (inTriangle == 1)
				std::swap(outV1, outV2); // Triangle 1 is (x, y), (x + 1, y + 1), (x + 1, y) to keep the normal pointing up
			return !IsHole(inX, inY) && !IsHole(x2, y2) && !IsHole(inX + 1, inY + 1);
		}

		// Iterate all triangles and call inFunction(x, y, triangle, has_collision, centroid, normal)
		template <class F>
		void					ForEachTriangle(const F &inFunction) const
		{
			for (uint y = 0; y < mSettings.mSampleCount - 1; ++y)
				for (uint x = 0; x < mSettings.mSampleCount - 1; ++x)
					for (uint t = 0; t < 2; ++t)
					{
						Vec3 v0, v1, v2;
						bool has_collision = GetTriangle(x, y, t, v0, v1, v2);
						Vec3 centroid = (v0 + v1 + v2) / 3.0f;
						Vec3 normal = (v1 - v0).Cross(v2 - v0).Normalized();
						CHECK(normal.GetY() > 0.0f); // Normal should point up
						inFunction(x, y, t, has_collision, centroid, normal);
					}
		}

		// Check that a sub shape ID refers to the expected triangle
		void					CheckSubShapeID(const SubShapeID &inSubShapeID, uint inX, uint inY, uint inTriangle) const
		{
			uint x, y, t;
			mShape->GetSubShapeCoordinates(inSubShapeID, x, y, t);
			CHECK(x == inX);
			CHECK(y == inY);
			CHECK(t == inTriangle);
		}

		HeightFieldShapeSettings mSettings;
		Ref<HeightFieldShape>	mShape;
	};

	TEST_CASE("TestHeightFieldCastSphere")
	{
		constexpr float cRadius = 0.1f; // Small enough so that the sphere fits inside a triangle and doesn't touch neighboring triangles
		constexpr float cDistance = 0.5f;

		Ref<Shape> sphere = new SphereShape(cRadius);

		for (uint sample_count : { 32u, 33u }) // Test power of 2 and non-power of 2 sample counts
		{
			SinCosHeightField height_field(sample_count);

			uint num_hits = 0, num_misses = 0;
			height_field.ForEachTriangle([&](uint inX, uint inY, uint inTriangle, bool inHasCollision, Vec3Arg inCentroid, Vec3Arg inNormal) {
				// Cast the sphere along the normal towards the centroid of the triangle, it should touch the triangle halfway
				Vec3 start = inCentroid + (cRadius + cDistance) * inNormal;
				Vec3 direction = -2.0f * cDistance * inNormal;
				ShapeCast shape_cast = ShapeCast::sFromWorldTransform(sphere, Vec3::sOne(), Mat44::sTranslation(start), direction);
				ShapeCastSettings settings;
				ClosestHitCollisionCollector<CastShapeCollector> collector;
				CollisionDispatch::sCastShapeVsShapeWorldSpace(shape_cast, settings, height_field.mShape, Vec3::sOne(), ShapeFilter(), Mat44::sIdentity(), SubShapeIDCreator(), SubShapeIDCreator(), collector);

				if (inHasCollision)
				{
					++num_hits;
					CHECK(collector.HadHit());
					const ShapeCastResult &hit = collector.mHit;
					CHECK_APPROX_EQUAL(hit.mFraction, 0.5f, 1.0e-5f);
					CHECK_APPROX_EQUAL(hit.mContactPointOn1, inCentroid, 1.0e-5f);
					CHECK_APPROX_EQUAL(hit.mContactPointOn2, inCentroid, 1.0e-5f);
					CHECK_APPROX_EQUAL(-hit.mPenetrationAxis.Normalized(), inNormal, 1.0e-4f);
					CHECK(hit.mPenetrationDepth == 0.0f);
					height_field.CheckSubShapeID(hit.mSubShapeID2, inX, inY, inTriangle);
					CHECK(!hit.mIsBackFaceHit);
				}
				else
				{
					// Sphere passes through the hole
					++num_misses;
					CHECK(!collector.HadHit());
				}
			});

			// The hole removes 6 triangles
			CHECK(num_misses == 6);
			CHECK(num_hits == 2 * Square(sample_count - 1) - 6);
		}
	}

	TEST_CASE("TestHeightFieldCastRay")
	{
		for (uint sample_count : { 32u, 33u }) // Test power of 2 and non-power of 2 sample counts
		{
			SinCosHeightField height_field(sample_count);

			uint num_hits = 0, num_misses = 0;
			height_field.ForEachTriangle([&](uint inX, uint inY, uint inTriangle, bool inHasCollision, Vec3Arg inCentroid, Vec3Arg inNormal) {
				// Cast a ray along the normal towards the centroid of the triangle, it should hit halfway
				RayCast ray { inCentroid + inNormal, -2.0f * inNormal };

				// Cast ray without collector
				RayCastResult hit;
				bool had_hit = height_field.mShape->CastRay(ray, SubShapeIDCreator(), hit);
				CHECK(had_hit == inHasCollision);
				if (inHasCollision)
					CHECK_APPROX_EQUAL(hit.mFraction, 0.5f, 1.0e-5f);
				else
					CHECK(hit.mFraction > 1.0f);

				// Cast ray with collector
				AllHitCollisionCollector<CastRayCollector> collector;
				height_field.mShape->CastRay(ray, RayCastSettings(), SubShapeIDCreator(), collector);
				if (inHasCollision)
				{
					++num_hits;

					// Check result without collector
					CHECK_APPROX_EQUAL(hit.mFraction, 0.5f, 1.0e-5f);
					height_field.CheckSubShapeID(hit.mSubShapeID2, inX, inY, inTriangle);
					Vec3 hit_pos = ray.GetPointOnRay(hit.mFraction);
					CHECK_APPROX_EQUAL(hit_pos, inCentroid, 1.0e-5f);
					CHECK_APPROX_EQUAL(height_field.mShape->GetSurfaceNormal(hit.mSubShapeID2, hit_pos), inNormal, 1.0e-4f);

					// Check result with collector, back faces are ignored by default so we should only hit the front face
					CHECK(collector.mHits.size() == 1);
					if (collector.mHits.size() == 1)
					{
						const RayCastResult &collector_hit = collector.mHits.front();
						CHECK_APPROX_EQUAL(collector_hit.mFraction, 0.5f, 1.0e-5f);
						height_field.CheckSubShapeID(collector_hit.mSubShapeID2, inX, inY, inTriangle);
						Vec3 collector_hit_pos = ray.GetPointOnRay(collector_hit.mFraction);
						CHECK_APPROX_EQUAL(collector_hit_pos, inCentroid, 1.0e-5f);
						CHECK_APPROX_EQUAL(height_field.mShape->GetSurfaceNormal(collector_hit.mSubShapeID2, collector_hit_pos), inNormal, 1.0e-4f);
					}
				}
				else
				{
					// Ray passes through the hole
					++num_misses;
					CHECK(collector.mHits.empty());
				}
			});

			// The hole removes 6 triangles
			CHECK(num_misses == 6);
			CHECK(num_hits == 2 * Square(sample_count - 1) - 6);
		}
	}

	TEST_CASE("TestHeightFieldProjectOntoSurface")
	{
		for (uint sample_count : { 32u, 33u }) // Test power of 2 and non-power of 2 sample counts
		{
			SinCosHeightField height_field(sample_count);

			uint num_hits = 0, num_misses = 0;
			height_field.ForEachTriangle([&](uint inX, uint inY, uint inTriangle, bool inHasCollision, Vec3Arg inCentroid, [[maybe_unused]] Vec3Arg inNormal) {
				// Project a point above the centroid onto the surface, it should land on the centroid
				Vec3 surface_pos;
				SubShapeID sub_shape_id;
				bool had_hit = height_field.mShape->ProjectOntoSurface(inCentroid + Vec3(0, 10, 0), surface_pos, sub_shape_id);
				CHECK(had_hit == inHasCollision);

				if (inHasCollision)
				{
					++num_hits;
					CHECK_APPROX_EQUAL(surface_pos, inCentroid, 1.0e-5f);
					height_field.CheckSubShapeID(sub_shape_id, inX, inY, inTriangle);
				}
				else
				{
					// Point is above the hole
					++num_misses;
				}
			});

			// The hole removes 6 triangles
			CHECK(num_misses == 6);
			CHECK(num_hits == 2 * Square(sample_count - 1) - 6);

			// Points outside of the height field should not project
			const HeightFieldShapeSettings &settings = height_field.mSettings;
			Vec3 surface_pos;
			SubShapeID sub_shape_id;
			float max_coord = float(sample_count - 1) + 0.5f; // Note that for 33 samples, this tests the padding that is added internally to round up to a multiple of the block size
			constexpr float cInsideCoord = 5.5f; // Inside the height field and away from the hole
			CHECK(!height_field.mShape->ProjectOntoSurface(settings.mOffset + settings.mScale * Vec3(-0.5f, 0, cInsideCoord), surface_pos, sub_shape_id));
			CHECK(!height_field.mShape->ProjectOntoSurface(settings.mOffset + settings.mScale * Vec3(cInsideCoord, 0, -0.5f), surface_pos, sub_shape_id));
			CHECK(!height_field.mShape->ProjectOntoSurface(settings.mOffset + settings.mScale * Vec3(max_coord, 0, cInsideCoord), surface_pos, sub_shape_id));
			CHECK(!height_field.mShape->ProjectOntoSurface(settings.mOffset + settings.mScale * Vec3(cInsideCoord, 0, max_coord), surface_pos, sub_shape_id));
		}
	}

	TEST_CASE("TestHeightFieldCollideSoftBodyVertices")
	{
		constexpr float cDistance = 0.05f; // Small enough that the closest point is inside the triangle below the vertex
		constexpr int cCollidingShapeIndex = 3;

		for (uint sample_count : { 32u, 33u }) // Test power of 2 and non-power of 2 sample counts
		{
			SinCosHeightField height_field(sample_count);

			// Create a vertex slightly above the centroid of every triangle
			struct Expected
			{
				bool			mHasCollision;
				Vec3			mCentroid;
				Vec3			mNormal;
			};
			Array<Expected> expected;
			Array<SoftBodyVertex> vertices;
			height_field.ForEachTriangle([&](uint, uint, uint, bool inHasCollision, Vec3Arg inCentroid, Vec3Arg inNormal) {
				expected.push_back({ inHasCollision, inCentroid, inNormal });

				SoftBodyVertex v;
				v.mPreviousPosition = v.mPosition = inCentroid + cDistance * inNormal;
				v.mVelocity = Vec3::sZero();
				v.mInvMass = 1.0f;
				v.ResetCollision();
				vertices.push_back(v);
			});

			height_field.mShape->CollideSoftBodyVertices(Mat44::sIdentity(), Vec3::sOne(), CollideSoftBodyVertexIterator(vertices.data()), uint(vertices.size()), cCollidingShapeIndex);

			for (size_t i = 0; i < vertices.size(); ++i)
			{
				const SoftBodyVertex &v = vertices[i];
				const Expected &e = expected[i];

				// The vertex is always above the surface, so we should find a collision with a negative penetration
				CHECK(v.mCollidingShapeIndex == cCollidingShapeIndex);
				CHECK(v.mLargestPenetration < 0.0f);
				CHECK(v.mCollisionPlane.SignedDistance(v.mPosition) > 0.0f);

				if (e.mHasCollision)
				{
					// The vertex should collide with the plane of the triangle below it
					CHECK_APPROX_EQUAL(v.mCollisionPlane.GetNormal(), e.mNormal, 1.0e-4f);
					CHECK_APPROX_EQUAL(v.mCollisionPlane.SignedDistance(e.mCentroid), 0.0f, 1.0e-4f);
					CHECK_APPROX_EQUAL(v.mLargestPenetration, -cDistance, 1.0e-4f);
				}
			}
		}
	}

	TEST_CASE("TestHeightFieldGetSupportingFace")
	{
		for (uint sample_count : { 32u, 33u }) // Test power of 2 and non-power of 2 sample counts
		{
			SinCosHeightField height_field(sample_count);

			height_field.ForEachTriangle([&](uint inX, uint inY, uint inTriangle, bool inHasCollision, Vec3Arg inCentroid, [[maybe_unused]] Vec3Arg inNormal) {
				if (!inHasCollision)
					return;

				// Get the sub shape ID of this triangle
				Vec3 surface_pos;
				SubShapeID sub_shape_id;
				CHECK(height_field.mShape->ProjectOntoSurface(inCentroid + Vec3(0, 10, 0), surface_pos, sub_shape_id));
				height_field.CheckSubShapeID(sub_shape_id, inX, inY, inTriangle);

				Vec3 v0, v1, v2;
				height_field.GetTriangle(inX, inY, inTriangle, v0, v1, v2);

				// Get the supporting face in local space
				Shape::SupportingFace face;
				height_field.mShape->GetSupportingFace(sub_shape_id, Vec3::sAxisY(), Vec3::sOne(), Mat44::sIdentity(), face);
				CHECK(face.size() == 3);
				if (face.size() == 3)
				{
					CHECK_APPROX_EQUAL(face[0], v0, 1.0e-5f);
					CHECK_APPROX_EQUAL(face[1], v1, 1.0e-5f);
					CHECK_APPROX_EQUAL(face[2], v2, 1.0e-5f);
				}

				// Get the supporting face with a transform and an inside out scale, the winding order should be reversed
				Vec3 scale(-1.0f, 2.0f, 3.0f);
				Mat44 transform = Mat44::sRotationTranslation(Quat::sRotation(Vec3::sAxisY(), 0.3f), Vec3(1, 2, 3));
				Mat44 scaled_transform = transform.PreScaled(scale);
				height_field.mShape->GetSupportingFace(sub_shape_id, Vec3::sAxisY(), scale, transform, face);
				CHECK(face.size() == 3);
				if (face.size() == 3)
				{
					CHECK_APPROX_EQUAL(face[0], scaled_transform * v0, 1.0e-4f);
					CHECK_APPROX_EQUAL(face[1], scaled_transform * v2, 1.0e-4f);
					CHECK_APPROX_EQUAL(face[2], scaled_transform * v1, 1.0e-4f);
				}
			});
		}
	}

	TEST_CASE("TestHeightFieldClone")
	{
		for (uint sample_count : { 32u, 33u }) // Test power of 2 and non-power of 2 sample counts
		{
			SinCosHeightField height_field(sample_count);
			const HeightFieldShape *original = height_field.mShape;

			Ref<HeightFieldShape> clone = original->Clone();
			CHECK(clone != original);
			CHECK(clone->GetSampleCount() == original->GetSampleCount());
			CHECK(clone->GetBlockSize() == original->GetBlockSize());

			// Check that all positions and holes match (also the padding for the non-power of 2 sample count)
			for (uint y = 0; y < original->GetSampleCount(); ++y)
				for (uint x = 0; x < original->GetSampleCount(); ++x)
				{
					bool no_collision = original->IsNoCollision(x, y);
					CHECK(clone->IsNoCollision(x, y) == no_collision);
					if (!no_collision)
						CHECK(clone->GetPosition(x, y) == original->GetPosition(x, y));
				}
		}
	}

	TEST_CASE("TestHeightFieldGetTriangles")
	{
		constexpr float cHalfExtent = 0.01f; // Small enough that the box only overlaps with the quad that contains the centroid

		for (uint sample_count : { 32u, 33u }) // Test power of 2 and non-power of 2 sample counts
		{
			SinCosHeightField height_field(sample_count);

			height_field.ForEachTriangle([&](uint inX, uint inY, uint inTriangle, bool inHasCollision, Vec3Arg inCentroid, [[maybe_unused]] Vec3Arg inNormal) {
				// Get the triangles of the quad that contains the centroid.
				// Note that the height field culls at the level of a quad, so we can get both triangles of the quad.
				Vec3 quad_vertices[2][3];
				bool quad_has_collision[2];
				for (uint t = 0; t < 2; ++t)
					quad_has_collision[t] = height_field.GetTriangle(inX, inY, t, quad_vertices[t][0], quad_vertices[t][1], quad_vertices[t][2]);

				// Collect triangles in a small box around the centroid
				Shape::GetTrianglesContext context;
				height_field.mShape->GetTrianglesStart(context, AABox::sFromTwoPoints(inCentroid - Vec3::sReplicate(cHalfExtent), inCentroid + Vec3::sReplicate(cHalfExtent)), Vec3::sZero(), Quat::sIdentity(), Vec3::sOne());
				Float3 vertices[3 * Shape::cGetTrianglesMinTrianglesRequested];
				int num_triangles = height_field.mShape->GetTrianglesNext(context, Shape::cGetTrianglesMinTrianglesRequested, vertices);
				CHECK(height_field.mShape->GetTrianglesNext(context, Shape::cGetTrianglesMinTrianglesRequested, vertices + 3 * num_triangles) == 0);

				// Check that all returned triangles are in this quad
				bool found[2] = { false, false };
				for (int i = 0; i < num_triangles; ++i)
				{
					Vec3 v0(vertices[3 * i]), v1(vertices[3 * i + 1]), v2(vertices[3 * i + 2]);
					int match = -1;
					for (uint t = 0; t < 2; ++t)
						if (quad_has_collision[t] && v0.IsClose(quad_vertices[t][0], 1.0e-8f) && v1.IsClose(quad_vertices[t][1], 1.0e-8f) && v2.IsClose(quad_vertices[t][2], 1.0e-8f))
							match = t;
					CHECK(match != -1);
					if (match != -1)
					{
						CHECK(!found[match]); // Each triangle should be returned only once
						found[match] = true;
					}
				}

				// When the triangle has collision, it should be returned together with the other triangle in the quad (if it has collision)
				if (inHasCollision)
				{
					CHECK(found[inTriangle]);
					CHECK(num_triangles == int(quad_has_collision[0]) + int(quad_has_collision[1]));
				}
				else
					CHECK(!found[inTriangle]);
			});
		}
	}
}
