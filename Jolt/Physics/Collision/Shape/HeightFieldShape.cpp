// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Collision/Shape/HeightFieldShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexShape.h>
#include <Jolt/Physics/Collision/Shape/ScaleHelpers.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/ShapeCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/CollidePointResult.h>
#include <Jolt/Physics/Collision/ShapeFilter.h>
#include <Jolt/Physics/Collision/CastConvexVsTriangles.h>
#include <Jolt/Physics/Collision/CastSphereVsTriangles.h>
#include <Jolt/Physics/Collision/CollideConvexVsTriangles.h>
#include <Jolt/Physics/Collision/CollideSphereVsTriangles.h>
#include <Jolt/Physics/Collision/TransformedShape.h>
#include <Jolt/Physics/Collision/ActiveEdges.h>
#include <Jolt/Physics/Collision/CollisionDispatch.h>
#include <Jolt/Physics/Collision/SortReverseAndStore.h>
#include <Jolt/Core/Profiler.h>
#include <Jolt/Core/StringTools.h>
#include <Jolt/Core/StreamIn.h>
#include <Jolt/Core/StreamOut.h>
#include <Jolt/Geometry/AABox4.h>
#include <Jolt/Geometry/RayTriangle.h>
#include <Jolt/Geometry/RayAABox.h>
#include <Jolt/Geometry/OrientedBox.h>
#include <Jolt/ObjectStream/TypeDeclarations.h>

//#define JPH_DEBUG_HEIGHT_FIELD

JPH_NAMESPACE_BEGIN

#ifdef JPH_DEBUG_RENDERER
bool HeightFieldShape::sDrawTriangleOutlines = false;
#endif // JPH_DEBUG_RENDERER

using namespace HeightFieldShapeConstants;

JPH_IMPLEMENT_SERIALIZABLE_VIRTUAL(HeightFieldShapeSettings)
{
	JPH_ADD_BASE_CLASS(HeightFieldShapeSettings, ShapeSettings)

	JPH_ADD_ATTRIBUTE(HeightFieldShapeSettings, mHeightSamples)
	JPH_ADD_ATTRIBUTE(HeightFieldShapeSettings, mOffset)
	JPH_ADD_ATTRIBUTE(HeightFieldShapeSettings, mScale)
	JPH_ADD_ATTRIBUTE(HeightFieldShapeSettings, mSampleCount)
	JPH_ADD_ATTRIBUTE(HeightFieldShapeSettings, mBlockSize)
	JPH_ADD_ATTRIBUTE(HeightFieldShapeSettings, mBitsPerSample)
	JPH_ADD_ATTRIBUTE(HeightFieldShapeSettings, mMaterialIndices)
	JPH_ADD_ATTRIBUTE(HeightFieldShapeSettings, mMaterials)
}

const uint HeightFieldShape::sGridOffsets[] = 
{
	0,			// level:  0, max x/y:     0, offset: 0
	1,			// level:  1, max x/y:     1, offset: 1
	5,			// level:  2, max x/y:     3, offset: 1 + 4
	21,			// level:  3, max x/y:     7, offset: 1 + 4 + 16
	85,			// level:  4, max x/y:    15, offset: 1 + 4 + 64
	341,		// level:  5, max x/y:    31, offset: 1 + 4 + 64 + 256
	1365,		// level:  6, max x/y:    63, offset: 1 + 4 + 64 + 256 + 1024
	5461,		// level:  7, max x/y:   127, offset: 1 + 4 + 64 + 256 + 1024 + 4096
	21845,		// level:  8, max x/y:   255, offset: 1 + 4 + 64 + 256 + 1024 + 4096 + ...
	87381,		// level:  9, max x/y:   511, offset: 1 + 4 + 64 + 256 + 1024 + 4096 + ...
	349525,		// level: 10, max x/y:  1023, offset: 1 + 4 + 64 + 256 + 1024 + 4096 + ...
	1398101,	// level: 11, max x/y:  2047, offset: 1 + 4 + 64 + 256 + 1024 + 4096 + ...
	5592405,	// level: 12, max x/y:  4095, offset: 1 + 4 + 64 + 256 + 1024 + 4096 + ...
	22369621,	// level: 13, max x/y:  8191, offset: 1 + 4 + 64 + 256 + 1024 + 4096 + ...
	89478485,	// level: 14, max x/y: 16383, offset: 1 + 4 + 64 + 256 + 1024 + 4096 + ...
};

HeightFieldShapeSettings::HeightFieldShapeSettings(const float *inSamples, Vec3Arg inOffset, Vec3Arg inScale, uint32 inSampleCount, const uint8 *inMaterialIndices, const PhysicsMaterialList &inMaterialList) :
	mOffset(inOffset),
	mScale(inScale),
	mSampleCount(inSampleCount)
{
	mHeightSamples.resize(inSampleCount * inSampleCount);
	memcpy(&mHeightSamples[0], inSamples, inSampleCount * inSampleCount * sizeof(float));

	if (!inMaterialList.empty() && inMaterialIndices != nullptr)
	{
		mMaterialIndices.resize(Square(inSampleCount - 1));
		memcpy(&mMaterialIndices[0], inMaterialIndices, Square(inSampleCount - 1) * sizeof(uint8));
		mMaterials = inMaterialList;
	}
	else
	{
		JPH_ASSERT(inMaterialList.empty());
		JPH_ASSERT(inMaterialIndices == nullptr);
	}
}

ShapeSettings::ShapeResult HeightFieldShapeSettings::Create() const
{
	if (mCachedResult.IsEmpty())
		Ref<Shape> shape = new HeightFieldShape(*this, mCachedResult); 
	return mCachedResult;
}

void HeightFieldShapeSettings::DetermineMinAndMaxSample(float &outMinValue, float &outMaxValue, float &outQuantizationScale) const
{
	// Determine min and max value
	outMinValue = FLT_MAX;
	outMaxValue = -FLT_MAX;
	for (float h : mHeightSamples)
		if (h != cNoCollisionValue)
		{
			outMinValue = min(outMinValue, h);
			outMaxValue = max(outMaxValue, h);
		}

	// Prevent dividing by zero by setting a minimal height difference
	float height_diff = max(outMaxValue - outMinValue, 1.0e-6f);

	// Calculate the scale factor to quantize to 16 bits
	outQuantizationScale = float(cMaxHeightValue16) / height_diff;
}

uint32 HeightFieldShapeSettings::CalculateBitsPerSampleForError(float inMaxError) const
{
	// Start with 1 bit per sample
	uint32 bits_per_sample = 1;

	// Determine total range
	float min_value, max_value, scale;
	DetermineMinAndMaxSample(min_value, max_value, scale);
	if (min_value < max_value)
	{
		// Loop over all blocks
		for (uint y = 0; y < mSampleCount; y += mBlockSize)
			for (uint x = 0; x < mSampleCount; x += mBlockSize)
			{
				// Determine min and max block value + take 1 sample border just like we do while building the hierarchical grids
				float block_min_value = FLT_MAX, block_max_value = -FLT_MAX;
				for (uint bx = x; bx < min(x + mBlockSize + 1, mSampleCount); ++bx)
					for (uint by = y; by < min(y + mBlockSize + 1, mSampleCount); ++by)
					{
						float h = mHeightSamples[by * mSampleCount + bx];
						if (h != cNoCollisionValue)
						{
							block_min_value = min(block_min_value, h);
							block_max_value = max(block_max_value, h);
						}
					}

				if (block_min_value < block_max_value)
				{
					// Quantize then dequantize block min/max value
					block_min_value = min_value + floor((block_min_value - min_value) * scale) / scale;
					block_max_value = min_value + ceil((block_max_value - min_value) * scale) / scale;
					float block_height = block_max_value - block_min_value;

					// Loop over the block again
					for (uint bx = x; bx < x + mBlockSize; ++bx)
						for (uint by = y; by < y + mBlockSize; ++by)
						{
							// Get the height
							float height = mHeightSamples[by * mSampleCount + bx];
							if (height != cNoCollisionValue)
							{
								for (;;)
								{
									// Determine bitmask for sample
									uint32 sample_mask = (1 << bits_per_sample) - 1;

									// Quantize
									float quantized_height = floor((height - block_min_value) * float(sample_mask) / block_height);
									quantized_height = Clamp(quantized_height, 0.0f, float(sample_mask - 1));

									// Dequantize and check error
									float dequantized_height = block_min_value + (quantized_height + 0.5f) * block_height / float(sample_mask);
									if (abs(dequantized_height - height) <= inMaxError)
										break;

									// Not accurate enough, increase bits per sample
									bits_per_sample++;

									// Don't go above 8 bits per sample
									if (bits_per_sample == 8)
										return bits_per_sample;
								}
							}
						}
				}
			}

	}

	return bits_per_sample;
}

void HeightFieldShape::CalculateActiveEdges()
{
	// Store active edges. The triangles are organized like this:
	//  +       +
	//  | \ T1B | \ T2B 
	// e0   e2  |   \
	//  | T1A \ | T2A \
	//  +--e1---+-------+
	//  | \ T3B | \ T4B 
	//  |   \   |   \
	//  | T3A \ | T4A \
	//  +-------+-------+
	// We store active edges e0 .. e2 as bits 0 .. 2. 
	// We store triangles horizontally then vertically (order T1A, T2A, T3A and T4A).
	// The top edge and right edge of the heightfield are always active so we do not need to store them, 
	// therefore we only need to store (mSampleCount - 1)^2 * 3-bit
	// The triangles T1B, T2B, T3B and T4B do not need to be stored, their active edges can be constructed from adjacent triangles.
	// Add 1 byte padding so we can always read 1 uint16 to get the bits that cross an 8 bit boundary
	uint count_min_1 = mSampleCount - 1;
	uint count_min_1_sq = Square(count_min_1);
	mActiveEdges.resize((count_min_1_sq * 3 + 7) / 8 + 1); 
	memset(&mActiveEdges[0], 0, mActiveEdges.size());

	// Calculate triangle normals and make normals zero for triangles that are missing
	vector<Vec3> normals;
	normals.resize(2 * count_min_1_sq);
	memset(&normals[0], 0, normals.size() * sizeof(Vec3));
	for (uint y = 0; y < count_min_1; ++y)
		for (uint x = 0; x < count_min_1; ++x)
			if (!IsNoCollision(x, y) && !IsNoCollision(x + 1, y + 1))
			{
				Vec3 x1y1 = GetPosition(x, y);
				Vec3 x2y2 = GetPosition(x + 1, y + 1);

				uint offset = 2 * (count_min_1 * y + x);

				if (!IsNoCollision(x, y + 1))
				{
					Vec3 x1y2 = GetPosition(x, y + 1);
					normals[offset] = (x2y2 - x1y2).Cross(x1y1 - x1y2).Normalized();
				}

				if (!IsNoCollision(x + 1, y))
				{
					Vec3 x2y1 = GetPosition(x + 1, y);
					normals[offset + 1] = (x1y1 - x2y1).Cross(x2y2 - x2y1).Normalized();
				}
			}

	// Calculate active edges
	for (uint y = 0; y < count_min_1; ++y)
		for (uint x = 0; x < count_min_1; ++x)
		{
			// Calculate vertex positions. 
			// We don't check 'no colliding' since those normals will be zero and sIsEdgeActive will return true
			Vec3 x1y1 = GetPosition(x, y);
			Vec3 x1y2 = GetPosition(x, y + 1);
			Vec3 x2y2 = GetPosition(x + 1, y + 1);

			// Calculate the edge flags (3 bits)
			uint offset = 2 * (count_min_1 * y + x);
			bool edge0_active = x == 0 || ActiveEdges::IsEdgeActive(normals[offset], normals[offset - 1], x1y2 - x1y1);
			bool edge1_active = y == count_min_1 - 1 || ActiveEdges::IsEdgeActive(normals[offset], normals[offset + 2 * count_min_1 + 1], x2y2 - x1y2);
			bool edge2_active = ActiveEdges::IsEdgeActive(normals[offset], normals[offset + 1], x1y1 - x2y2);
			uint16 edge_flags = (edge0_active? 0b001 : 0) | (edge1_active? 0b010 : 0) | (edge2_active? 0b100 : 0);

			// Store the edge flags in the array
			uint bit_pos = 3 * (y * count_min_1 + x);
			uint byte_pos = bit_pos >> 3;
			bit_pos &= 0b111;
			edge_flags <<= bit_pos;
			mActiveEdges[byte_pos] |= uint8(edge_flags);
			mActiveEdges[byte_pos + 1] |= uint8(edge_flags >> 8);
		}
}

void HeightFieldShape::StoreMaterialIndices(const vector<uint8> &inMaterialIndices)
{
	uint count_min_1 = mSampleCount - 1;

	mNumBitsPerMaterialIndex = 32 - CountLeadingZeros((uint32)mMaterials.size() - 1);
	mMaterialIndices.resize(((Square(count_min_1) * mNumBitsPerMaterialIndex + 7) >> 3) + 1); // Add 1 byte so we don't read out of bounds when reading an uint16

	for (uint y = 0; y < count_min_1; ++y)
		for (uint x = 0; x < count_min_1; ++x)
		{
			// Read material
			uint sample_pos = x + y * count_min_1;
			uint16 material_index = uint16(inMaterialIndices[sample_pos]);

			// Calculate byte and bit position where the material index needs to go
			uint bit_pos = sample_pos * mNumBitsPerMaterialIndex;
			uint byte_pos = bit_pos >> 3;
			bit_pos &= 0b111;

			// Write the material index
			material_index <<= bit_pos;
			JPH_ASSERT(byte_pos + 1 < mMaterialIndices.size());
			mMaterialIndices[byte_pos] |= uint8(material_index);
			mMaterialIndices[byte_pos + 1] |= uint8(material_index >> 8);
		}
}

void HeightFieldShape::CacheValues()
{
	mSampleMask = uint8((uint32(1) << mBitsPerSample) - 1);
}

HeightFieldShape::HeightFieldShape(const HeightFieldShapeSettings &inSettings, ShapeResult &outResult) :
	Shape(EShapeType::HeightField, EShapeSubType::HeightField, inSettings, outResult),
	mOffset(inSettings.mOffset),
	mScale(inSettings.mScale),
	mSampleCount(inSettings.mSampleCount),
	mBlockSize(inSettings.mBlockSize),
	mBitsPerSample(uint8(inSettings.mBitsPerSample)),
	mMaterials(inSettings.mMaterials)
{
	CacheValues();

	// Check block size
	if (mBlockSize < 2 || mBlockSize > 8)
	{
		outResult.SetError("HeightFieldShape: Block size must be in the range [2, 8]!");
		return;
	}

	// Check sample count
	if (mSampleCount % mBlockSize != 0)
	{
		outResult.SetError("HeightFieldShape: Sample count must be a multiple of block size!");
		return;
	}

	// Check bits per sample
	if (inSettings.mBitsPerSample < 1 || inSettings.mBitsPerSample > 8)
	{
		outResult.SetError("HeightFieldShape: Bits per sample must be in the range [1, 8]!");
		return;
	}

	// We stop at mBlockSize x mBlockSize height sample blocks
	uint n = GetNumBlocks();

	// Required to be power of two to allow creating a hierarchical grid
	if (!IsPowerOf2(n))
	{
		outResult.SetError("HeightFieldShape: Sample count / block size must be power of 2!");
		return;
	}

	// We want at least 1 grid layer
	if (n < 2)
	{
		outResult.SetError("HeightFieldShape: Sample count too low!");
		return;
	}

	// Check that we don't overflow our 32 bit 'properties'
	if (n > (1 << cNumBitsXY))
	{
		outResult.SetError("HeightFieldShape: Sample count too high!");
		return;
	}

	// Check if we're not exceeding the amount of sub shape id bits
	if (GetSubShapeIDBitsRecursive() > SubShapeID::MaxBits)
	{
		outResult.SetError("HeightFieldShape: Size exceeds the amount of available sub shape ID bits!");
		return;
	}

	if (!mMaterials.empty())
	{
		// Validate materials
		if (mMaterials.size() > 256)
		{
			outResult.SetError("Supporting max 256 materials per height field");
			return;
		}
		for (uint8 s : inSettings.mMaterialIndices)
			if (s >= mMaterials.size())
			{
				outResult.SetError(StringFormat("Material %u is beyond material list (size: %u)", s, (uint)mMaterials.size()));
				return;
			}
	}
	else
	{
		// No materials assigned, validate that no materials have been specified
		if (!inSettings.mMaterialIndices.empty())
		{
			outResult.SetError("No materials present, mMaterialIndices should be empty");
			return;
		}
	}

	// Determine range
	float min_value, max_value, scale;
	inSettings.DetermineMinAndMaxSample(min_value, max_value, scale);
	if (min_value > max_value)
	{
		// If there is no collision with this heightmap, leave everything empty
		mMaterials.clear();
		outResult.Set(this);
		return;
	}

	// Quantize to uint16
	vector<uint16> quantized_samples;
	quantized_samples.reserve(mSampleCount * mSampleCount);
	for (float h : inSettings.mHeightSamples)
		if (h == cNoCollisionValue)
		{
			quantized_samples.push_back(cNoCollisionValue16);
		}
		else
		{
			// Floor the quantized height to get a lower bound for the quantized value
			int quantized_height = (int)floor(scale * (h - min_value));

			// Ensure that the height says below the max height value so we can safely add 1 to get the upper bound for the quantized value
			quantized_height = Clamp(quantized_height, 0, int(cMaxHeightValue16 - 1));

			quantized_samples.push_back(uint16(quantized_height)); 
		}

	// Update offset and scale to account for the compression to uint16
	if (min_value <= max_value) // Only when there was collision
	{
		// In GetPosition we always add 0.5 to the quantized sample in order to reduce the average error.
		// We want to be able to exactly quantize min_value (this is important in case the heightfield is entirely flat) so we subtract that value from min_value.
		min_value -= 0.5f / (scale * mSampleMask);

		mOffset.SetY(mOffset.GetY() + mScale.GetY() * min_value);
	}
	mScale.SetY(mScale.GetY() / scale);

	// Calculate amount of grids
	uint max_level = sGetMaxLevel(n);

	// Temporary data structure used during creating of a hierarchy of grids 
	struct Range
	{
		uint16	mMin;
		uint16	mMax;
	};

	// Reserve size for temporary range data + reserve 1 extra for a 1x1 grid that we won't store but use for calculating the bounding box
	vector<vector<Range>> ranges;
	ranges.resize(max_level + 1);

	// Calculate highest detail grid by combining mBlockSize x mBlockSize height samples
	vector<Range> *cur_range_vector = &ranges.back();
	cur_range_vector->resize(n * n);
	Range *range_dst = &cur_range_vector->front();
	for (uint y = 0; y < n; ++y)
		for (uint x = 0; x < n; ++x)
		{
			range_dst->mMin = 0xffff;
			range_dst->mMax = 0;
			uint max_bx = x == n - 1? mBlockSize : mBlockSize + 1; // for interior blocks take 1 more because the triangles connect to the next block so we must include their height too
			uint max_by = y == n - 1? mBlockSize : mBlockSize + 1;
			for (uint by = 0; by < max_by; ++by)
				for (uint bx = 0; bx < max_bx; ++bx)
				{
					uint16 h = quantized_samples[(y * mBlockSize + by) * mSampleCount + (x * mBlockSize + bx)];
					if (h != cNoCollisionValue16)
					{
						range_dst->mMin = min(range_dst->mMin, h);
						range_dst->mMax = max(range_dst->mMax, uint16(h + 1)); // Add 1 to the max so we know the real value is between mMin and mMax
					}
				}
			++range_dst;
		}
		
	// Calculate remaining grids
	while (n > 1)
	{
		// Get source buffer
		const Range *range_src = &cur_range_vector->front();

		// Previous array element
		--cur_range_vector;

		// Make space for this grid
		n >>= 1;
		cur_range_vector->resize(n * n);

		// Get target buffer
		range_dst = &cur_range_vector->front();

		// Combine the results of 2x2 ranges
		for (uint y = 0; y < n; ++y)
			for (uint x = 0; x < n; ++x)
			{
				range_dst->mMin = 0xffff;
				range_dst->mMax = 0;
				for (uint by = 0; by < 2; ++by)
					for (uint bx = 0; bx < 2; ++bx)
					{
						const Range &r = range_src[(y * 2 + by) * n * 2 + x * 2 + bx];
						range_dst->mMin = min(range_dst->mMin, r.mMin);
						range_dst->mMax = max(range_dst->mMax, r.mMax);
					}
				++range_dst;
			}
	}
	JPH_ASSERT(cur_range_vector == &ranges.front());

	// Store global range for bounding box calculation
	mMinSample = ranges[0][0].mMin;
	mMaxSample = ranges[0][0].mMax;

#ifdef JPH_ENABLE_ASSERTS
	// Validate that we did not lose range along the way
	uint16 minv = 0xffff, maxv = 0;
	for (uint16 v : quantized_samples)
		if (v != cNoCollisionValue16)
		{
			minv = min(minv, v);
			maxv = max(maxv, uint16(v + 1));
		}
	JPH_ASSERT(mMinSample == minv && mMaxSample == maxv);
#endif

	// Now erase the first element, we need a 2x2 grid to start with
	ranges.erase(ranges.begin());

	// Create blocks
	mRangeBlocks.reserve(sGridOffsets[ranges.size()]);
	for (uint level = 0; level < ranges.size(); ++level)
	{
		JPH_ASSERT(mRangeBlocks.size() == sGridOffsets[level]);

		n = 1 << level;

		for (uint y = 0; y < n; ++y)
			for (uint x = 0; x < n; ++x)
			{
				// Convert from 2x2 Range structure to 1 RangeBlock structure
				RangeBlock rb;
				for (uint by = 0; by < 2; ++by)
					for (uint bx = 0; bx < 2; ++bx)
					{
						uint src_pos = (y * 2 + by) * n * 2 + (x * 2 + bx);
						uint dst_pos = by * 2 + bx;
						rb.mMin[dst_pos] = ranges[level][src_pos].mMin;
						rb.mMax[dst_pos] = ranges[level][src_pos].mMax;
					}

				// Add this block
				mRangeBlocks.push_back(rb);
			}
	}	
	JPH_ASSERT(mRangeBlocks.size() == sGridOffsets[ranges.size()]);

	// Quantize height samples
	mHeightSamples.resize((mSampleCount * mSampleCount * inSettings.mBitsPerSample + 7) / 8 + 1);
	int sample = 0;
	for (uint y = 0; y < mSampleCount; ++y)
		for (uint x = 0; x < mSampleCount; ++x)
		{
			uint32 output_value;

			float h = inSettings.mHeightSamples[y * mSampleCount + x];
			if (h == cNoCollisionValue)
			{
				// No collision
				output_value = mSampleMask;
			}
			else
			{
				// Get range of block so we know what range to compress to
				uint bx = x / mBlockSize;
				uint by = y / mBlockSize;
				const Range &range = ranges.back()[by * (mSampleCount / mBlockSize) + bx];
				JPH_ASSERT(range.mMin < range.mMax);

				// Quantize to mBitsPerSample bits, note that mSampleMask is reserved for indicating that there's no collision.
				// We divide the range into mSampleMask segments and use the mid points of these segments as the quantized values.
				// This results in a lower error than if we had quantized our data using the lowest point of all these segments.
				float h_min = min_value + range.mMin / scale;
				float h_delta = float(range.mMax - range.mMin) / scale;
				float quantized_height = floor((h - h_min) * float(mSampleMask) / h_delta);
				output_value = uint32(Clamp((int)quantized_height, 0, int(mSampleMask) - 1)); // mSampleMask is reserved as 'no collision value'
			}

			// Store the sample
			uint byte_pos = sample >> 3;
			uint bit_pos = sample & 0b111;
			output_value <<= bit_pos;
			mHeightSamples[byte_pos] |= uint8(output_value);
			mHeightSamples[byte_pos + 1] |= uint8(output_value >> 8);
			sample += inSettings.mBitsPerSample;
		}

	// Calculate the active edges
	CalculateActiveEdges();

	// Compress material indices
	if (mMaterials.size() > 1)
		StoreMaterialIndices(inSettings.mMaterialIndices);

	outResult.Set(this);
}

inline void HeightFieldShape::sGetRangeBlockOffsetAndStride(uint inNumBlocks, uint inMaxLevel, uint &outRangeBlockOffset, uint &outRangeBlockStride)
{
	outRangeBlockOffset = sGridOffsets[inMaxLevel - 1];
	outRangeBlockStride = inNumBlocks >> 1;
}

inline void HeightFieldShape::GetBlockOffsetAndScale(uint inBlockX, uint inBlockY, uint inRangeBlockOffset, uint inRangeBlockStride, float &outBlockOffset, float &outBlockScale) const
{
	JPH_ASSERT(inBlockX < GetNumBlocks() && inBlockY < GetNumBlocks());

	// Convert to location of range block
	uint rbx = inBlockX >> 1;
	uint rby = inBlockY >> 1;
	uint n = ((inBlockY & 1) << 1) + (inBlockX & 1);

	// Calculate offset and scale
	const RangeBlock &block = mRangeBlocks[inRangeBlockOffset + rby * inRangeBlockStride + rbx];
	outBlockOffset = float(block.mMin[n]);
	outBlockScale = float(block.mMax[n] - block.mMin[n]) / float(mSampleMask);
}

inline uint8 HeightFieldShape::GetHeightSample(uint inX, uint inY) const
{
	JPH_ASSERT(inX < mSampleCount); 
	JPH_ASSERT(inY < mSampleCount); 
	
	// Determine bit position of sample
	uint sample = (inY * mSampleCount + inX) * uint(mBitsPerSample);
	uint byte_pos = sample >> 3;
	uint bit_pos = sample & 0b111;

	// Fetch the height sample value
	JPH_ASSERT(byte_pos + 1 < mHeightSamples.size());
	const uint8 *height_samples = mHeightSamples.data() + byte_pos;
	uint16 height_sample = uint16(height_samples[0]) | uint16(uint16(height_samples[1]) << 8);
	return uint8(height_sample >> bit_pos) & mSampleMask; 
}

inline Vec3 HeightFieldShape::GetPosition(uint inX, uint inY, float inBlockOffset, float inBlockScale, bool &outNoCollision) const
{ 
	// Get quantized value
	uint8 height_sample = GetHeightSample(inX, inY);
	outNoCollision = height_sample == mSampleMask;

	// Add 0.5 to the quantized value to minimize the error (see constructor)
	return mOffset + mScale * Vec3(float(inX), inBlockOffset + (0.5f + height_sample) * inBlockScale, float(inY)); 
}

Vec3 HeightFieldShape::GetPosition(uint inX, uint inY) const
{
	// Test if there are any samples
	if (mHeightSamples.empty())
		return mOffset + mScale * Vec3(float(inX), 0.0f, float(inY));

	// Get block location
	uint bx = inX / mBlockSize;
	uint by = inY / mBlockSize;

	// Calculate offset and stride
	uint num_blocks = GetNumBlocks();
	uint range_block_offset, range_block_stride;
	sGetRangeBlockOffsetAndStride(num_blocks, sGetMaxLevel(num_blocks), range_block_offset, range_block_stride);

	float offset, scale;
	GetBlockOffsetAndScale(bx, by, range_block_offset, range_block_stride, offset, scale);

	bool no_collision;
	return GetPosition(inX, inY, offset, scale, no_collision);
}

bool HeightFieldShape::IsNoCollision(uint inX, uint inY) const
{ 
	return mHeightSamples.empty() || GetHeightSample(inX, inY) == mSampleMask;
}

bool HeightFieldShape::ProjectOntoSurface(Vec3Arg inLocalPosition, Vec3 &outSurfacePosition, SubShapeID &outSubShapeID) const
{
	// Check if we have collision
	if (mHeightSamples.empty())
		return false;

	// Convert coordinate to integer space
	Vec3 integer_space = (inLocalPosition - mOffset) / mScale;

	// Get x coordinate and fraction
	float x_frac = integer_space.GetX();
	if (x_frac < 0.0f || x_frac >= mSampleCount - 1)
		return false;
	uint x = (uint)floor(x_frac);
	x_frac -= x;

	// Get y coordinate and fraction
	float y_frac = integer_space.GetZ();
	if (y_frac < 0.0f || y_frac >= mSampleCount - 1)
		return false;
	uint y = (uint)floor(y_frac);
	y_frac -= y;

	// If one of the diagonal points doesn't have collision, we don't have a height at this location
	if (IsNoCollision(x, y) || IsNoCollision(x + 1, y + 1))
		return false;

	if (y_frac >= x_frac)
	{
		// Left bottom triangle, test the 3rd point
		if (IsNoCollision(x, y + 1))
			return false;

		// Interpolate height value
		Vec3 v1 = GetPosition(x, y);
		Vec3 v2 = GetPosition(x, y + 1);
		Vec3 v3 = GetPosition(x + 1, y + 1);
		outSurfacePosition = v1 + y_frac * (v2 - v1) + x_frac * (v3 - v2);
		SubShapeIDCreator creator;
		outSubShapeID = EncodeSubShapeID(creator, x, y, 0);
		return true;
	}
	else
	{
		// Right top triangle, test the third point
		if (IsNoCollision(x + 1, y))
			return false;

		// Interpolate height value
		Vec3 v1 = GetPosition(x, y);
		Vec3 v2 = GetPosition(x + 1, y + 1);
		Vec3 v3 = GetPosition(x + 1, y);
		outSurfacePosition = v1 + y_frac * (v2 - v3) + x_frac * (v3 - v1);
		SubShapeIDCreator creator;
		outSubShapeID = EncodeSubShapeID(creator, x, y, 1);
		return true;
	}
}

MassProperties HeightFieldShape::GetMassProperties() const
{
	// Object should always be static, return default mass properties
	return MassProperties();
}

const PhysicsMaterial *HeightFieldShape::GetMaterial(uint inX, uint inY) const
{
	if (mMaterials.empty())
		return PhysicsMaterial::sDefault;
	if (mMaterials.size() == 1)
		return mMaterials[0];

	uint count_min_1 = mSampleCount - 1;
	JPH_ASSERT(inX < count_min_1);
	JPH_ASSERT(inY < count_min_1);

	// Calculate at which bit the material index starts
	uint bit_pos = (inX + inY * count_min_1) * mNumBitsPerMaterialIndex;
	uint byte_pos = bit_pos >> 3;
	bit_pos &= 0b111;

	// Read the material index
	JPH_ASSERT(byte_pos + 1 < mMaterialIndices.size());
	const uint8 *material_indices = mMaterialIndices.data() + byte_pos;
	uint16 material_index = uint16(material_indices[0]) + uint16(uint16(material_indices[1]) << 8);
	material_index >>= bit_pos;
	material_index &= (1 << mNumBitsPerMaterialIndex) - 1;

	// Return the material
	return mMaterials[material_index];
}

uint HeightFieldShape::GetSubShapeIDBits() const
{
	// Need to store X, Y and 1 extra bit to specify the triangle number in the quad
	return 2 * (32 - CountLeadingZeros(mSampleCount - 1)) + 1;
}

SubShapeID HeightFieldShape::EncodeSubShapeID(const SubShapeIDCreator &inCreator, uint inX, uint inY, uint inTriangle) const
{
	return inCreator.PushID((inX + inY * mSampleCount) * 2 + inTriangle, GetSubShapeIDBits()).GetID();
}

void HeightFieldShape::DecodeSubShapeID(const SubShapeID &inSubShapeID, uint &outX, uint &outY, uint &outTriangle) const
{
	// Decode sub shape id
	SubShapeID remainder;
	uint32 id = inSubShapeID.PopID(GetSubShapeIDBits(), remainder);
	JPH_ASSERT(remainder.IsEmpty(), "Invalid subshape ID");

	// Get triangle index
	outTriangle = id & 1;
	id >>= 1;

	// Fetch the x and y coordinate
	outX = id % mSampleCount;
	outY = id / mSampleCount;
}

const PhysicsMaterial *HeightFieldShape::GetMaterial(const SubShapeID &inSubShapeID) const
{
	// Decode ID
	uint x, y, triangle;
	DecodeSubShapeID(inSubShapeID, x, y, triangle);

	// Fetch the material
	return GetMaterial(x, y);
}

Vec3 HeightFieldShape::GetSurfaceNormal(const SubShapeID &inSubShapeID, Vec3Arg inLocalSurfacePosition) const 
{ 
	// Decode ID
	uint x, y, triangle;
	DecodeSubShapeID(inSubShapeID, x, y, triangle);

	// Fetch vertices that both triangles share
	Vec3 x1y1 = GetPosition(x, y);
	Vec3 x2y2 = GetPosition(x + 1, y + 1);

	// Get normal depending on which triangle was selected
	Vec3 normal;
	if (triangle == 0)
	{
		Vec3 x1y2 = GetPosition(x, y + 1);
		normal = (x2y2 - x1y2).Cross(x1y1 - x1y2);
	}
	else
	{
		Vec3 x2y1 = GetPosition(x + 1, y);
		normal = (x1y1 - x2y1).Cross(x2y2 - x2y1);
	}

	return normal.Normalized();
}

inline uint8 HeightFieldShape::GetEdgeFlags(uint inX, uint inY, uint inTriangle) const
{
	if (inTriangle == 0)
	{
		// The edge flags for this triangle are directly stored, find the right 3 bits
		uint bit_pos = 3 * (inX + inY * (mSampleCount - 1));
		uint byte_pos = bit_pos >> 3;
		bit_pos &= 0b111;
		JPH_ASSERT(byte_pos + 1 < mActiveEdges.size());
		const uint8 *active_edges = mActiveEdges.data() + byte_pos;
		uint16 edge_flags = uint16(active_edges[0]) + uint16(uint16(active_edges[1]) << 8);
		return uint8(edge_flags >> bit_pos) & 0b111;
	}
	else
	{
		// We don't store this triangle directly, we need to look at our three neighbours to construct the edge flags
		uint8 edge0 = (GetEdgeFlags(inX, inY, 0) & 0b100) != 0? 0b001 : 0; // Diagonal edge
		uint8 edge1 = inX == mSampleCount - 1 || (GetEdgeFlags(inX + 1, inY, 0) & 0b001) != 0? 0b010 : 0; // Vertical edge
		uint8 edge2 = inY == 0 || (GetEdgeFlags(inX, inY - 1, 0) & 0b010) != 0? 0b100 : 0; // Horizontal edge
		return edge0 | edge1 | edge2;
	}
}

AABox HeightFieldShape::GetLocalBounds() const
{
	if (mMinSample == cNoCollisionValue16)
	{
		// This whole height field shape doesn't have any collision, return the center point
		Vec3 center = mOffset + 0.5f * mScale * Vec3(float(mSampleCount - 1), 0.0f, float(mSampleCount - 1));
		return AABox(center, center);
	}
	else
	{
		// Bounding box based on min and max sample height
		Vec3 bmin = mOffset + mScale * Vec3(0.0f, float(mMinSample), 0.0f);
		Vec3 bmax = mOffset + mScale * Vec3(float(mSampleCount - 1), float(mMaxSample), float(mSampleCount - 1));
		return AABox(bmin, bmax);
	}
}

#ifdef JPH_DEBUG_RENDERER
void HeightFieldShape::Draw(DebugRenderer *inRenderer, Mat44Arg inCenterOfMassTransform, Vec3Arg inScale, ColorArg inColor, bool inUseMaterialColors, bool inDrawWireframe) const
{
	// Don't draw anything if we don't have any collision
	if (mHeightSamples.empty())
		return;

	// Reset the batch if we switch coloring mode
	if (mCachedUseMaterialColors != inUseMaterialColors)
	{
		mGeometry.clear();
		mCachedUseMaterialColors = inUseMaterialColors;
	}

	if (mGeometry.empty())
	{
		// Divide terrain in triangle batches of max 64x64x2 triangles to allow better culling of the terrain
		uint32 block_size = min<uint32>(mSampleCount, 64);
		for (uint32 by = 0; by < mSampleCount; by += block_size)
			for (uint32 bx = 0; bx < mSampleCount; bx += block_size)
			{
				// Create vertices for a block
				vector<DebugRenderer::Triangle> triangles;
				triangles.resize(block_size * block_size * 2);
				DebugRenderer::Triangle *out_tri = &triangles[0];
				for (uint32 y = by, max_y = min(by + block_size, mSampleCount - 1); y < max_y; ++y)
					for (uint32 x = bx, max_x = min(bx + block_size, mSampleCount - 1); x < max_x; ++x)
						if (!IsNoCollision(x, y) && !IsNoCollision(x + 1, y + 1))
						{
							Vec3 x1y1 = GetPosition(x, y);
							Vec3 x2y2 = GetPosition(x + 1, y + 1);
							Color color = inUseMaterialColors? GetMaterial(x, y)->GetDebugColor() : Color::sWhite;

							if (!IsNoCollision(x, y + 1))
							{
								Vec3 x1y2 = GetPosition(x, y + 1);

								x1y1.StoreFloat3(&out_tri->mV[0].mPosition);
								x1y2.StoreFloat3(&out_tri->mV[1].mPosition);
								x2y2.StoreFloat3(&out_tri->mV[2].mPosition);

								Vec3 normal = (x2y2 - x1y2).Cross(x1y1 - x1y2).Normalized();
								for (DebugRenderer::Vertex &v : out_tri->mV)
								{
									v.mColor = color;
									v.mUV = Float2(0, 0);
									normal.StoreFloat3(&v.mNormal);
								}

								++out_tri;
							}

							if (!IsNoCollision(x + 1, y))
							{
								Vec3 x2y1 = GetPosition(x + 1, y);

								x1y1.StoreFloat3(&out_tri->mV[0].mPosition);
								x2y2.StoreFloat3(&out_tri->mV[1].mPosition);
								x2y1.StoreFloat3(&out_tri->mV[2].mPosition);

								Vec3 normal = (x1y1 - x2y1).Cross(x2y2 - x2y1).Normalized();
								for (DebugRenderer::Vertex &v : out_tri->mV)
								{
									v.mColor = color;
									v.mUV = Float2(0, 0);
									normal.StoreFloat3(&v.mNormal);
								}

								++out_tri;
							}
						}

				// Resize triangles array to actual amount of triangles written
				size_t num_triangles = out_tri - &triangles[0];
				triangles.resize(num_triangles);

				// Create batch
				if (num_triangles > 0)
					mGeometry.push_back(new DebugRenderer::Geometry(inRenderer->CreateTriangleBatch(triangles), DebugRenderer::sCalculateBounds(&triangles[0].mV[0], int(3 * num_triangles))));
			}
	}

	// Get transform including scale
	Mat44 transform = inCenterOfMassTransform * Mat44::sScale(inScale);

	// Test if the shape is scaled inside out
	DebugRenderer::ECullMode cull_mode = ScaleHelpers::IsInsideOut(inScale)? DebugRenderer::ECullMode::CullFrontFace : DebugRenderer::ECullMode::CullBackFace;

	// Determine the draw mode
	DebugRenderer::EDrawMode draw_mode = inDrawWireframe? DebugRenderer::EDrawMode::Wireframe : DebugRenderer::EDrawMode::Solid;

	// Draw the geometry
	for (const DebugRenderer::GeometryRef &b : mGeometry)
		inRenderer->DrawGeometry(transform, inColor, b, cull_mode, DebugRenderer::ECastShadow::On, draw_mode);

	if (sDrawTriangleOutlines)
	{
		struct Visitor
		{
			JPH_INLINE explicit		Visitor(const HeightFieldShape *inShape, DebugRenderer *inRenderer, Mat44Arg inTransform) :
				mShape(inShape),
				mRenderer(inRenderer),
				mTransform(inTransform)
			{
			}

			JPH_INLINE bool			ShouldAbort() const
			{
				return false;
			}

			JPH_INLINE bool			ShouldVisitRangeBlock([[maybe_unused]] int inStackTop) const
			{
				return true;
			}

			JPH_INLINE int			VisitRangeBlock(Vec4Arg inBoundsMinX, Vec4Arg inBoundsMinY, Vec4Arg inBoundsMinZ, Vec4Arg inBoundsMaxX, Vec4Arg inBoundsMaxY, Vec4Arg inBoundsMaxZ, UVec4 &ioProperties, [[maybe_unused]] int inStackTop) const
			{
				UVec4 valid = UVec4::sOr(UVec4::sOr(Vec4::sLess(inBoundsMinX, inBoundsMaxX), Vec4::sLess(inBoundsMinY, inBoundsMaxY)), Vec4::sLess(inBoundsMinZ, inBoundsMaxZ));
				return CountAndSortTrues(valid, ioProperties);
			}

			JPH_INLINE void			VisitTriangle(uint inX, uint inY, uint inTriangle, Vec3Arg inV0, Vec3Arg inV1, Vec3Arg inV2) const
			{			
				// Determine active edges
				uint8 active_edges = mShape->GetEdgeFlags(inX, inY, inTriangle);

				// Loop through edges
				Vec3 v[] = { inV0, inV1, inV2 };
				for (uint edge_idx = 0; edge_idx < 3; ++edge_idx)
				{
					Vec3 v1 = mTransform * v[edge_idx];
					Vec3 v2 = mTransform * v[(edge_idx + 1) % 3];

					// Draw active edge as a green arrow, other edges as grey
					if (active_edges & (1 << edge_idx))
						mRenderer->DrawArrow(v1, v2, Color::sGreen, 0.01f);
					else
						mRenderer->DrawLine(v1, v2, Color::sGrey);
				}
			}

			const HeightFieldShape *mShape;
			DebugRenderer *			mRenderer;
			Mat44					mTransform;
		};

		Visitor visitor(this, inRenderer, inCenterOfMassTransform * Mat44::sScale(inScale));
		WalkHeightField(visitor);
	}
}
#endif // JPH_DEBUG_RENDERER

class HeightFieldShape::DecodingContext
{
public:
	JPH_INLINE explicit			DecodingContext(const HeightFieldShape *inShape) :
		mShape(inShape)
	{
		static_assert(sizeof(sGridOffsets) / sizeof(uint) == cNumBitsXY + 1, "Offsets array is not long enough");
	
		// Construct root stack entry
		mPropertiesStack[0] = 0; // level: 0, x: 0, y: 0
	}

	template <class Visitor>
	JPH_INLINE void				WalkHeightField(Visitor &ioVisitor)
	{
		// Early out if there's no collision
		if (mShape->mHeightSamples.empty())
			return;

		// Precalculate values relating to sample count
		uint32 sample_count = mShape->mSampleCount;
		UVec4 sample_count_min_1 = UVec4::sReplicate(sample_count - 1);

		// Precalculate values relating to block size
		uint32 block_size = mShape->mBlockSize;
		uint32 block_size_plus_1 = block_size + 1;
		uint num_blocks = mShape->GetNumBlocks();
		uint num_blocks_min_1 = num_blocks - 1;
		uint max_level = HeightFieldShape::sGetMaxLevel(num_blocks);

		// Precalculate range block offset and stride for GetBlockOffsetAndScale
		uint range_block_offset, range_block_stride;
		sGetRangeBlockOffsetAndStride(num_blocks, max_level, range_block_offset, range_block_stride);

		// Allocate space for vertices and 'no collision' flags
		int array_size = Square(block_size_plus_1);
		Vec3 *vertices = reinterpret_cast<Vec3 *>(JPH_STACK_ALLOC(array_size * sizeof(Vec3)));
		bool *no_collision = reinterpret_cast<bool *>(JPH_STACK_ALLOC(array_size * sizeof(bool)));

		// Splat offsets
		Vec4 ox = mShape->mOffset.SplatX();
		Vec4 oy = mShape->mOffset.SplatY();
		Vec4 oz = mShape->mOffset.SplatZ();

		// Splat scales
		Vec4 sx = mShape->mScale.SplatX();
		Vec4 sy = mShape->mScale.SplatY();
		Vec4 sz = mShape->mScale.SplatZ();

		do
		{
			// Decode properties
			uint32 properties_top = mPropertiesStack[mTop];
			uint32 x = properties_top & cMaskBitsXY;
			uint32 y = (properties_top >> cNumBitsXY) & cMaskBitsXY;
			uint32 level = properties_top >> cLevelShift;

			if (level >= max_level)
			{
				// Determine actual range of samples (minus one because we eventually want to iterate over the triangles, not the samples)
				uint32 min_x = x * block_size;
				uint32 max_x = min_x + block_size;
				uint32 min_y = y * block_size;
				uint32 max_y = min_y + block_size;

				// Decompress vertices of block at (x, y)
				Vec3 *dst_vertex = vertices;
				bool *dst_no_collision = no_collision;
				float block_offset, block_scale;
				mShape->GetBlockOffsetAndScale(x, y, range_block_offset, range_block_stride, block_offset, block_scale);
				for (uint32 v_y = min_y; v_y < max_y; ++v_y)
				{
					for (uint32 v_x = min_x; v_x < max_x; ++v_x)
					{
						*dst_vertex = mShape->GetPosition(v_x, v_y, block_offset, block_scale, *dst_no_collision);
						++dst_vertex;
						++dst_no_collision;
					}

					// Skip last column, these values come from a different block
					++dst_vertex;
					++dst_no_collision;
				}

				// Decompress block (x + 1, y)
				uint32 max_x_decrement = 0;
				if (x < num_blocks_min_1)
				{
					dst_vertex = vertices + block_size;
					dst_no_collision = no_collision + block_size;
					mShape->GetBlockOffsetAndScale(x + 1, y, range_block_offset, range_block_stride, block_offset, block_scale);
					for (uint32 v_y = min_y; v_y < max_y; ++v_y)
					{
						*dst_vertex = mShape->GetPosition(max_x, v_y, block_offset, block_scale, *dst_no_collision);
						dst_vertex += block_size_plus_1;
						dst_no_collision += block_size_plus_1;
					}
				}
				else
					max_x_decrement = 1; // We don't have a next block, one less triangle to test

				// Decompress block (x, y + 1)
				if (y < num_blocks_min_1)
				{
					uint start = block_size * block_size_plus_1;
					dst_vertex = vertices + start;
					dst_no_collision = no_collision + start;
					mShape->GetBlockOffsetAndScale(x, y + 1, range_block_offset, range_block_stride, block_offset, block_scale);
					for (uint32 v_x = min_x; v_x < max_x; ++v_x)
					{
						*dst_vertex = mShape->GetPosition(v_x, max_y, block_offset, block_scale, *dst_no_collision);
						++dst_vertex;
						++dst_no_collision;
					}

					// Decompress single sample of block at (x + 1, y + 1)
					if (x < num_blocks_min_1)
					{
						mShape->GetBlockOffsetAndScale(x + 1, y + 1, range_block_offset, range_block_stride, block_offset, block_scale);
						*dst_vertex = mShape->GetPosition(max_x, max_y, block_offset, block_scale, *dst_no_collision);
					}
				}
				else
					--max_y; // We don't have a next block, one less triangle to test

				// Update max_x (we've been using it so we couldn't update it earlier)
				max_x -= max_x_decrement;

				// We're going to divide the vertices in 4 blocks to do one more runtime sub-division, calculate the ranges of those blocks
				struct Range
				{
					uint32 mMinX, mMinY, mNumTrianglesX, mNumTrianglesY;
				};
				uint32 half_block_size = block_size >> 1;
				uint32 block_size_x = max_x - min_x - half_block_size;
				uint32 block_size_y = max_y - min_y - half_block_size;
				Range ranges[] = 
				{
					{ 0, 0,									half_block_size, half_block_size },
					{ half_block_size, 0,					block_size_x, half_block_size },
					{ 0, half_block_size,					half_block_size, block_size_y },
					{ half_block_size, half_block_size,		block_size_x, block_size_y },
				};

				// Calculate the min and max of each of the blocks
				Mat44 block_min, block_max;
				for (int block = 0; block < 4; ++block)
				{
					// Get the range for this block
					const Range &range = ranges[block];
					uint32 start = range.mMinX + range.mMinY * block_size_plus_1;
					uint32 size_x_plus_1 = range.mNumTrianglesX + 1;
					uint32 size_y_plus_1 = range.mNumTrianglesY + 1;

					// Calculate where to start reading
					const Vec3 *src_vertex = vertices + start;
					const bool *src_no_collision = no_collision + start;
					uint32 stride = block_size_plus_1 - size_x_plus_1;

					// Start range with a very large inside-out box
					Vec3 value_min = Vec3::sReplicate(1.0e30f);
					Vec3 value_max = Vec3::sReplicate(-1.0e30f);

					// Loop over the samples to determine the min and max of this block
					for (uint32 block_y = 0; block_y < size_y_plus_1; ++block_y)
					{
						for (uint32 block_x = 0; block_x < size_x_plus_1; ++block_x)
						{
							if (!*src_no_collision)
							{
								value_min = Vec3::sMin(value_min, *src_vertex);
								value_max = Vec3::sMax(value_max, *src_vertex);
							}
							++src_vertex;
							++src_no_collision;
						}
						src_vertex += stride;
						src_no_collision += stride;
					}
					block_min.SetColumn4(block, Vec4(value_min));
					block_max.SetColumn4(block, Vec4(value_max));
				}

			#ifdef JPH_DEBUG_HEIGHT_FIELD
				// Draw the bounding boxes of the sub-nodes
				for (int block = 0; block < 4; ++block)
				{
					AABox bounds(block_min.GetColumn3(block), block_max.GetColumn3(block));
					if (bounds.IsValid())
						DebugRenderer::sInstance->DrawWireBox(bounds, Color::sYellow);
				}
			#endif // JPH_DEBUG_HEIGHT_FIELD

				// Transpose so we have the mins and maxes of each of the blocks in rows instead of columns
				Mat44 transposed_min = block_min.Transposed();
				Mat44 transposed_max = block_max.Transposed();
				
				// Check which blocks collide
				// Note: At this point we don't use our own stack but we do allow the visitor to use its own stack
				// to store collision distances so that we can still early out when no closer hits have been found.
				UVec4 colliding_blocks(0, 1, 2, 3);
				int num_results = ioVisitor.VisitRangeBlock(transposed_min.GetColumn4(0), transposed_min.GetColumn4(1), transposed_min.GetColumn4(2), transposed_max.GetColumn4(0), transposed_max.GetColumn4(1), transposed_max.GetColumn4(2), colliding_blocks, mTop);

				// Loop through the results backwards (closest first)
				int result = num_results - 1;
				while (result >= 0)
				{
					// Calculate the min and max of this block
					uint32 block = colliding_blocks[result];
					const Range &range = ranges[block];
					uint32 block_min_x = min_x + range.mMinX;
					uint32 block_max_x = block_min_x + range.mNumTrianglesX;
					uint32 block_min_y = min_y + range.mMinY;
					uint32 block_max_y = block_min_y + range.mNumTrianglesY;

					// Loop triangles
					for (uint32 v_y = block_min_y; v_y < block_max_y; ++v_y)
						for (uint32 v_x = block_min_x; v_x < block_max_x; ++v_x)
						{
							// Get first vertex
							const int offset = (v_y - min_y) * block_size_plus_1 + (v_x - min_x);
							const Vec3 *start_vertex = vertices + offset;
							const bool *start_no_collision = no_collision + offset;

							// Check if vertices shared by both triangles have collision
							if (!start_no_collision[0] && !start_no_collision[block_size_plus_1 + 1])
							{
								// Loop 2 triangles
								for (uint t = 0; t < 2; ++t)
								{
									// Determine triangle vertices
									Vec3 v0, v1, v2;
									if (t == 0)
									{
										// Check third vertex
										if (start_no_collision[block_size_plus_1])
											continue;

										// Get vertices for triangle
										v0 = start_vertex[0];
										v1 = start_vertex[block_size_plus_1];
										v2 = start_vertex[block_size_plus_1 + 1];
									}
									else
									{
										// Check third vertex
										if (start_no_collision[1])
											continue;

										// Get vertices for triangle
										v0 = start_vertex[0];
										v1 = start_vertex[block_size_plus_1 + 1];
										v2 = start_vertex[1];
									}

								#ifdef JPH_DEBUG_HEIGHT_FIELD
									DebugRenderer::sInstance->DrawWireTriangle(v0, v1, v2, Color::sWhite);
								#endif

									// Call visitor
									ioVisitor.VisitTriangle(v_x, v_y, t, v0, v1, v2);

									// Check if we're done
									if (ioVisitor.ShouldAbort())
										return;
								}
							}
						}

					// Fetch next block until we find one that the visitor wants to see
					do 
						--result;
					while (result >= 0 && !ioVisitor.ShouldVisitRangeBlock(mTop + result));
				}
			}
			else
			{
				// Visit child grid
				uint32 offset = sGridOffsets[level] + (1 << level) * y + x;

				// Decode min/max height
				UVec4 block = UVec4::sLoadInt4Aligned(reinterpret_cast<const uint32 *>(&mShape->mRangeBlocks[offset]));
				Vec4 bounds_miny = oy + sy * block.Expand4Uint16Lo().ToFloat();
				Vec4 bounds_maxy = oy + sy * block.Expand4Uint16Hi().ToFloat();

				// Calculate size of one cell at this grid level
				UVec4 internal_cell_size = UVec4::sReplicate(block_size << (max_level - level - 1)); // subtract 1 from level because we have an internal grid of 2x2

				// Calculate min/max x and z
				UVec4 two_x = UVec4::sReplicate(2 * x); // multiply by two because we have an internal grid of 2x2
				Vec4 bounds_minx = ox + sx * (internal_cell_size * (two_x + UVec4(0, 1, 0, 1))).ToFloat();
				Vec4 bounds_maxx = ox + sx * UVec4::sMin(internal_cell_size * (two_x + UVec4(1, 2, 1, 2)), sample_count_min_1).ToFloat();

				UVec4 two_y = UVec4::sReplicate(2 * y);
				Vec4 bounds_minz = oz + sz * (internal_cell_size * (two_y + UVec4(0, 0, 1, 1))).ToFloat();
				Vec4 bounds_maxz = oz + sz * UVec4::sMin(internal_cell_size * (two_y + UVec4(1, 1, 2, 2)), sample_count_min_1).ToFloat();

				// Calculate properties of child blocks
				UVec4 properties = UVec4::sReplicate(((level + 1) << cLevelShift) + (y << (cNumBitsXY + 1)) + (x << 1)) + UVec4(0, 1, 1 << cNumBitsXY, (1 << cNumBitsXY) + 1);

			#ifdef JPH_DEBUG_HEIGHT_FIELD
				// Draw boxes
				for (int i = 0; i < 4; ++i)
				{
					AABox b(Vec3(bounds_minx[i], bounds_miny[i], bounds_minz[i]), Vec3(bounds_maxx[i], bounds_maxy[i], bounds_maxz[i]));
					if (b.IsValid())
						DebugRenderer::sInstance->DrawWireBox(b, Color::sGreen);
				}
			#endif

				// Check which sub nodes to visit
				int num_results = ioVisitor.VisitRangeBlock(bounds_minx, bounds_miny, bounds_minz, bounds_maxx, bounds_maxy, bounds_maxz, properties, mTop);

				// Push them onto the stack
				JPH_ASSERT(mTop + 4 < cStackSize);
				properties.StoreInt4(&mPropertiesStack[mTop]);
				mTop += num_results;		
			}

			// Check if we're done
			if (ioVisitor.ShouldAbort())
				return;

			// Fetch next node until we find one that the visitor wants to see
			do 
				--mTop;
			while (mTop >= 0 && !ioVisitor.ShouldVisitRangeBlock(mTop));
		}
		while (mTop >= 0);
	}

	// This can be used to have the visitor early out (ioVisitor.ShouldAbort() returns true) and later continue again (call WalkHeightField() again)
	JPH_INLINE bool				IsDoneWalking() const
	{
		return mTop < 0;
	}

private:
	const HeightFieldShape *	mShape;
	int							mTop = 0;
	uint32						mPropertiesStack[cStackSize];
};

template <class Visitor>
JPH_INLINE void HeightFieldShape::WalkHeightField(Visitor &ioVisitor) const
{
	DecodingContext ctx(this);
	ctx.WalkHeightField(ioVisitor);
}

bool HeightFieldShape::CastRay(const RayCast &inRay, const SubShapeIDCreator &inSubShapeIDCreator, RayCastResult &ioHit) const
{
	JPH_PROFILE_FUNCTION();

	struct Visitor
	{
		JPH_INLINE explicit		Visitor(const HeightFieldShape *inShape, const RayCast &inRay, const SubShapeIDCreator &inSubShapeIDCreator, RayCastResult &ioHit) : 
			mHit(ioHit),
			mRayOrigin(inRay.mOrigin),
			mRayDirection(inRay.mDirection),
			mRayInvDirection(inRay.mDirection),
			mShape(inShape),
			mSubShapeIDCreator(inSubShapeIDCreator)
		{ 
		}

		JPH_INLINE bool			ShouldAbort() const
		{
			return mHit.mFraction <= 0.0f;
		}

		JPH_INLINE bool			ShouldVisitRangeBlock(int inStackTop) const
		{
			return mDistanceStack[inStackTop] < mHit.mFraction;
		}

		JPH_INLINE int			VisitRangeBlock(Vec4Arg inBoundsMinX, Vec4Arg inBoundsMinY, Vec4Arg inBoundsMinZ, Vec4Arg inBoundsMaxX, Vec4Arg inBoundsMaxY, Vec4Arg inBoundsMaxZ, UVec4 &ioProperties, int inStackTop)
		{
			// Test bounds of 4 children
			Vec4 distance = RayAABox4(mRayOrigin, mRayInvDirection, inBoundsMinX, inBoundsMinY, inBoundsMinZ, inBoundsMaxX, inBoundsMaxY, inBoundsMaxZ);

			// Sort so that highest values are first (we want to first process closer hits and we process stack top to bottom)
			return SortReverseAndStore(distance, mHit.mFraction, ioProperties, &mDistanceStack[inStackTop]);
		}

		JPH_INLINE void			VisitTriangle(uint inX, uint inY, uint inTriangle, Vec3Arg inV0, Vec3Arg inV1, Vec3Arg inV2) 
		{			
			float fraction = RayTriangle(mRayOrigin, mRayDirection, inV0, inV1, inV2);
			if (fraction < mHit.mFraction)
			{
				// It's a closer hit
				mHit.mFraction = fraction;
				mHit.mSubShapeID2 = mShape->EncodeSubShapeID(mSubShapeIDCreator, inX, inY, inTriangle);
				mReturnValue = true;
			}
		}

		RayCastResult &			mHit;
		Vec3					mRayOrigin;
		Vec3					mRayDirection;
		RayInvDirection			mRayInvDirection;
		const HeightFieldShape *mShape;
		SubShapeIDCreator		mSubShapeIDCreator;
		bool					mReturnValue = false;
		float					mDistanceStack[cStackSize];
	};

	Visitor visitor(this, inRay, inSubShapeIDCreator, ioHit);
	WalkHeightField(visitor);

	return visitor.mReturnValue;
}

void HeightFieldShape::CastRay(const RayCast &inRay, const RayCastSettings &inRayCastSettings, const SubShapeIDCreator &inSubShapeIDCreator, CastRayCollector &ioCollector) const
{
	JPH_PROFILE_FUNCTION();

	struct Visitor
	{
		JPH_INLINE explicit		Visitor(const HeightFieldShape *inShape, const RayCast &inRay, const RayCastSettings &inRayCastSettings, const SubShapeIDCreator &inSubShapeIDCreator, CastRayCollector &ioCollector) : 
			mCollector(ioCollector),
			mRayOrigin(inRay.mOrigin),
			mRayDirection(inRay.mDirection),
			mRayInvDirection(inRay.mDirection),
			mBackFaceMode(inRayCastSettings.mBackFaceMode),
			mShape(inShape),
			mSubShapeIDCreator(inSubShapeIDCreator)
		{ 
		}

		JPH_INLINE bool			ShouldAbort() const
		{
			return mCollector.ShouldEarlyOut();
		}

		JPH_INLINE bool			ShouldVisitRangeBlock(int inStackTop) const
		{
			return mDistanceStack[inStackTop] < mCollector.GetEarlyOutFraction();
		}

		JPH_INLINE int			VisitRangeBlock(Vec4Arg inBoundsMinX, Vec4Arg inBoundsMinY, Vec4Arg inBoundsMinZ, Vec4Arg inBoundsMaxX, Vec4Arg inBoundsMaxY, Vec4Arg inBoundsMaxZ, UVec4 &ioProperties, int inStackTop) 
		{
			// Test bounds of 4 children
			Vec4 distance = RayAABox4(mRayOrigin, mRayInvDirection, inBoundsMinX, inBoundsMinY, inBoundsMinZ, inBoundsMaxX, inBoundsMaxY, inBoundsMaxZ);
	
			// Sort so that highest values are first (we want to first process closer hits and we process stack top to bottom)
			return SortReverseAndStore(distance, mCollector.GetEarlyOutFraction(), ioProperties, &mDistanceStack[inStackTop]);
		}

		JPH_INLINE void			VisitTriangle(uint inX, uint inY, uint inTriangle, Vec3Arg inV0, Vec3Arg inV1, Vec3Arg inV2) const
		{			
			// Back facing check
			if (mBackFaceMode == EBackFaceMode::IgnoreBackFaces && (inV2 - inV0).Cross(inV1 - inV0).Dot(mRayDirection) < 0)
				return;

			// Check the triangle
			float fraction = RayTriangle(mRayOrigin, mRayDirection, inV0, inV1, inV2);
			if (fraction < mCollector.GetEarlyOutFraction())
			{
				RayCastResult hit;
				hit.mBodyID = TransformedShape::sGetBodyID(mCollector.GetContext());
				hit.mFraction = fraction;
				hit.mSubShapeID2 = mShape->EncodeSubShapeID(mSubShapeIDCreator, inX, inY, inTriangle);
				mCollector.AddHit(hit);
			}
		}
		
		CastRayCollector &		mCollector;
		Vec3					mRayOrigin;
		Vec3					mRayDirection;
		RayInvDirection			mRayInvDirection;
		EBackFaceMode			mBackFaceMode;
		const HeightFieldShape *mShape;
		SubShapeIDCreator		mSubShapeIDCreator;
		float					mDistanceStack[cStackSize];
	};

	Visitor visitor(this, inRay, inRayCastSettings, inSubShapeIDCreator, ioCollector);
	WalkHeightField(visitor);
}

void HeightFieldShape::CollidePoint(Vec3Arg inPoint, const SubShapeIDCreator &inSubShapeIDCreator, CollidePointCollector &ioCollector) const
{
	// A height field doesn't have volume, so we can't test insideness
}

void HeightFieldShape::sCastConvexVsHeightField(const ShapeCast &inShapeCast, const ShapeCastSettings &inShapeCastSettings, const Shape *inShape, Vec3Arg inScale, const ShapeFilter &inShapeFilter, Mat44Arg inCenterOfMassTransform2, const SubShapeIDCreator &inSubShapeIDCreator1, const SubShapeIDCreator &inSubShapeIDCreator2, CastShapeCollector &ioCollector)
{
	JPH_PROFILE_FUNCTION();

	struct Visitor : public CastConvexVsTriangles
	{
		using CastConvexVsTriangles::CastConvexVsTriangles;

		JPH_INLINE bool				ShouldAbort() const
		{
			return mCollector.ShouldEarlyOut();
		}

		JPH_INLINE bool				ShouldVisitRangeBlock(int inStackTop) const
		{
			return mDistanceStack[inStackTop] < mCollector.GetEarlyOutFraction();
		}

		JPH_INLINE int				VisitRangeBlock(Vec4Arg inBoundsMinX, Vec4Arg inBoundsMinY, Vec4Arg inBoundsMinZ, Vec4Arg inBoundsMaxX, Vec4Arg inBoundsMaxY, Vec4Arg inBoundsMaxZ, UVec4 &ioProperties, int inStackTop) 
		{
			// Scale the bounding boxes of this node 
			Vec4 bounds_min_x, bounds_min_y, bounds_min_z, bounds_max_x, bounds_max_y, bounds_max_z;
			AABox4Scale(mScale, inBoundsMinX, inBoundsMinY, inBoundsMinZ, inBoundsMaxX, inBoundsMaxY, inBoundsMaxZ, bounds_min_x, bounds_min_y, bounds_min_z, bounds_max_x, bounds_max_y, bounds_max_z);

			// Enlarge them by the casted shape's box extents
			AABox4EnlargeWithExtent(mBoxExtent, bounds_min_x, bounds_min_y, bounds_min_z, bounds_max_x, bounds_max_y, bounds_max_z);

			// Test bounds of 4 children
			Vec4 distance = RayAABox4(mBoxCenter, mInvDirection, bounds_min_x, bounds_min_y, bounds_min_z, bounds_max_x, bounds_max_y, bounds_max_z);
	
			// Sort so that highest values are first (we want to first process closer hits and we process stack top to bottom)
			return SortReverseAndStore(distance, mCollector.GetEarlyOutFraction(), ioProperties, &mDistanceStack[inStackTop]);
		}

		JPH_INLINE void				VisitTriangle(uint inX, uint inY, uint inTriangle, Vec3Arg inV0, Vec3Arg inV1, Vec3Arg inV2)
		{			
			// Create sub shape id for this part
			SubShapeID triangle_sub_shape_id = mShape2->EncodeSubShapeID(mSubShapeIDCreator2, inX, inY, inTriangle);

			// Determine active edges
			uint8 active_edges = mShape2->GetEdgeFlags(inX, inY, inTriangle);

			Cast(inV0, inV1, inV2, active_edges, triangle_sub_shape_id);
		}

		const HeightFieldShape *	mShape2;
		RayInvDirection				mInvDirection;
		Vec3						mBoxCenter;
		Vec3						mBoxExtent;
		SubShapeIDCreator			mSubShapeIDCreator2;
		float						mDistanceStack[cStackSize];
	};

	JPH_ASSERT(inShape->GetSubType() == EShapeSubType::HeightField);
	const HeightFieldShape *shape = static_cast<const HeightFieldShape *>(inShape);

	Visitor visitor(inShapeCast, inShapeCastSettings, inScale, inShapeFilter, inCenterOfMassTransform2, inSubShapeIDCreator1, ioCollector);
	visitor.mShape2 = shape;
	visitor.mInvDirection.Set(inShapeCast.mDirection);
	visitor.mBoxCenter = inShapeCast.mShapeWorldBounds.GetCenter();
	visitor.mBoxExtent = inShapeCast.mShapeWorldBounds.GetExtent();
	visitor.mSubShapeIDCreator2 = inSubShapeIDCreator2;
	shape->WalkHeightField(visitor);
}

void HeightFieldShape::sCastSphereVsHeightField(const ShapeCast &inShapeCast, const ShapeCastSettings &inShapeCastSettings, const Shape *inShape, Vec3Arg inScale, const ShapeFilter &inShapeFilter, Mat44Arg inCenterOfMassTransform2, const SubShapeIDCreator &inSubShapeIDCreator1, const SubShapeIDCreator &inSubShapeIDCreator2, CastShapeCollector &ioCollector)
{
	JPH_PROFILE_FUNCTION();

	struct Visitor : public CastSphereVsTriangles
	{
		using CastSphereVsTriangles::CastSphereVsTriangles;

		JPH_INLINE bool				ShouldAbort() const
		{
			return mCollector.ShouldEarlyOut();
		}

		JPH_INLINE bool				ShouldVisitRangeBlock(int inStackTop) const
		{
			return mDistanceStack[inStackTop] < mCollector.GetEarlyOutFraction();
		}

		JPH_INLINE int				VisitRangeBlock(Vec4Arg inBoundsMinX, Vec4Arg inBoundsMinY, Vec4Arg inBoundsMinZ, Vec4Arg inBoundsMaxX, Vec4Arg inBoundsMaxY, Vec4Arg inBoundsMaxZ, UVec4 &ioProperties, int inStackTop) 
		{
			// Scale the bounding boxes of this node 
			Vec4 bounds_min_x, bounds_min_y, bounds_min_z, bounds_max_x, bounds_max_y, bounds_max_z;
			AABox4Scale(mScale, inBoundsMinX, inBoundsMinY, inBoundsMinZ, inBoundsMaxX, inBoundsMaxY, inBoundsMaxZ, bounds_min_x, bounds_min_y, bounds_min_z, bounds_max_x, bounds_max_y, bounds_max_z);

			// Enlarge them by the radius of the sphere
			AABox4EnlargeWithExtent(Vec3::sReplicate(mRadius), bounds_min_x, bounds_min_y, bounds_min_z, bounds_max_x, bounds_max_y, bounds_max_z);

			// Test bounds of 4 children
			Vec4 distance = RayAABox4(mStart, mInvDirection, bounds_min_x, bounds_min_y, bounds_min_z, bounds_max_x, bounds_max_y, bounds_max_z);
	
			// Sort so that highest values are first (we want to first process closer hits and we process stack top to bottom)
			return SortReverseAndStore(distance, mCollector.GetEarlyOutFraction(), ioProperties, &mDistanceStack[inStackTop]);
		}

		JPH_INLINE void				VisitTriangle(uint inX, uint inY, uint inTriangle, Vec3Arg inV0, Vec3Arg inV1, Vec3Arg inV2)
		{			
			// Create sub shape id for this part
			SubShapeID triangle_sub_shape_id = mShape2->EncodeSubShapeID(mSubShapeIDCreator2, inX, inY, inTriangle);

			// Determine active edges
			uint8 active_edges = mShape2->GetEdgeFlags(inX, inY, inTriangle);

			Cast(inV0, inV1, inV2, active_edges, triangle_sub_shape_id);
		}

		const HeightFieldShape *	mShape2;
		RayInvDirection				mInvDirection;
		SubShapeIDCreator			mSubShapeIDCreator2;
		float						mDistanceStack[cStackSize];
	};

	JPH_ASSERT(inShape->GetSubType() == EShapeSubType::HeightField);
	const HeightFieldShape *shape = static_cast<const HeightFieldShape *>(inShape);

	Visitor visitor(inShapeCast, inShapeCastSettings, inScale, inShapeFilter, inCenterOfMassTransform2, inSubShapeIDCreator1, ioCollector);
	visitor.mShape2 = shape;
	visitor.mInvDirection.Set(inShapeCast.mDirection);
	visitor.mSubShapeIDCreator2 = inSubShapeIDCreator2;
	shape->WalkHeightField(visitor);
}

struct HeightFieldShape::HSGetTrianglesContext
{
			HSGetTrianglesContext(const HeightFieldShape *inShape, const AABox &inBox, Vec3Arg inPositionCOM, QuatArg inRotation, Vec3Arg inScale) : 
		mDecodeCtx(inShape),
		mShape(inShape),
		mLocalBox(Mat44::sInverseRotationTranslation(inRotation, inPositionCOM), inBox),
		mHeightFieldScale(inScale),
		mLocalToWorld(Mat44::sRotationTranslation(inRotation, inPositionCOM) * Mat44::sScale(inScale)),
		mIsInsideOut(ScaleHelpers::IsInsideOut(inScale))
	{
	}

	bool	ShouldAbort() const
	{
		return mShouldAbort;
	}

	bool	ShouldVisitRangeBlock([[maybe_unused]] int inStackTop) const
	{
		return true;
	}

	int		VisitRangeBlock(Vec4Arg inBoundsMinX, Vec4Arg inBoundsMinY, Vec4Arg inBoundsMinZ, Vec4Arg inBoundsMaxX, Vec4Arg inBoundsMaxY, Vec4Arg inBoundsMaxZ, UVec4 &ioProperties, [[maybe_unused]] int inStackTop) const
	{
		// Scale the bounding boxes of this node 
		Vec4 bounds_min_x, bounds_min_y, bounds_min_z, bounds_max_x, bounds_max_y, bounds_max_z;
		AABox4Scale(mHeightFieldScale, inBoundsMinX, inBoundsMinY, inBoundsMinZ, inBoundsMaxX, inBoundsMaxY, inBoundsMaxZ, bounds_min_x, bounds_min_y, bounds_min_z, bounds_max_x, bounds_max_y, bounds_max_z);

		// Test which nodes collide
		UVec4 collides = AABox4VsBox(mLocalBox, bounds_min_x, bounds_min_y, bounds_min_z, bounds_max_x, bounds_max_y, bounds_max_z);
		return CountAndSortTrues(collides, ioProperties);
	}

	void	VisitTriangle(uint inX, uint inY, [[maybe_unused]] uint inTriangle, Vec3Arg inV0, Vec3Arg inV1, Vec3Arg inV2) 
	{			
		// When the buffer is full and we cannot process the triangles, abort the height field walk. The next time GetTrianglesNext is called we will continue here.
		if (mNumTrianglesFound + 1 > mMaxTrianglesRequested)
		{
			mShouldAbort = true;
			return;
		}

		// Store vertices as Float3
		if (mIsInsideOut)
		{
			// Reverse vertices
			(mLocalToWorld * inV0).StoreFloat3(mTriangleVertices++);
			(mLocalToWorld * inV2).StoreFloat3(mTriangleVertices++);
			(mLocalToWorld * inV1).StoreFloat3(mTriangleVertices++);
		}
		else
		{
			// Normal scale
			(mLocalToWorld * inV0).StoreFloat3(mTriangleVertices++);
			(mLocalToWorld * inV1).StoreFloat3(mTriangleVertices++);
			(mLocalToWorld * inV2).StoreFloat3(mTriangleVertices++);
		}

		// Decode material
		if (mMaterials != nullptr)
			*mMaterials++ = mShape->GetMaterial(inX, inY);

		// Accumulate triangles found
		mNumTrianglesFound++;
	}

	DecodingContext				mDecodeCtx;
	const HeightFieldShape *	mShape;
	OrientedBox					mLocalBox;
	Vec3						mHeightFieldScale;
	Mat44						mLocalToWorld;
	int							mMaxTrianglesRequested;
	Float3 *					mTriangleVertices;
	int							mNumTrianglesFound;
	const PhysicsMaterial **	mMaterials;
	bool						mShouldAbort;
	bool						mIsInsideOut;
};

void HeightFieldShape::GetTrianglesStart(GetTrianglesContext &ioContext, const AABox &inBox, Vec3Arg inPositionCOM, QuatArg inRotation, Vec3Arg inScale) const
{
	static_assert(sizeof(HSGetTrianglesContext) <= sizeof(GetTrianglesContext), "GetTrianglesContext too small");
	JPH_ASSERT(IsAligned(&ioContext, alignof(HSGetTrianglesContext)));

	new (&ioContext) HSGetTrianglesContext(this, inBox, inPositionCOM, inRotation, inScale);
}

int HeightFieldShape::GetTrianglesNext(GetTrianglesContext &ioContext, int inMaxTrianglesRequested, Float3 *outTriangleVertices, const PhysicsMaterial **outMaterials) const
{
	static_assert(cGetTrianglesMinTrianglesRequested >= 1, "cGetTrianglesMinTrianglesRequested is too small");
	JPH_ASSERT(inMaxTrianglesRequested >= cGetTrianglesMinTrianglesRequested);

	// Check if we're done
	HSGetTrianglesContext &context = (HSGetTrianglesContext &)ioContext;
	if (context.mDecodeCtx.IsDoneWalking())
		return 0;

	// Store parameters on context
	context.mMaxTrianglesRequested = inMaxTrianglesRequested;
	context.mTriangleVertices = outTriangleVertices;
	context.mMaterials = outMaterials;
	context.mShouldAbort = false; // Reset the abort flag
	context.mNumTrianglesFound = 0;
	
	// Continue (or start) walking the height field
	context.mDecodeCtx.WalkHeightField(context);
	return context.mNumTrianglesFound;
}

void HeightFieldShape::sCollideConvexVsHeightField(const Shape *inShape1, const Shape *inShape2, Vec3Arg inScale1, Vec3Arg inScale2, Mat44Arg inCenterOfMassTransform1, Mat44Arg inCenterOfMassTransform2, const SubShapeIDCreator &inSubShapeIDCreator1, const SubShapeIDCreator &inSubShapeIDCreator2, const CollideShapeSettings &inCollideShapeSettings, CollideShapeCollector &ioCollector)
{
	JPH_PROFILE_FUNCTION();

	// Get the shapes
	JPH_ASSERT(inShape1->GetType() == EShapeType::Convex);
	JPH_ASSERT(inShape2->GetType() == EShapeType::HeightField);
	const ConvexShape *shape1 = static_cast<const ConvexShape *>(inShape1);
	const HeightFieldShape *shape2 = static_cast<const HeightFieldShape *>(inShape2);

	struct Visitor : public CollideConvexVsTriangles
	{
		using CollideConvexVsTriangles::CollideConvexVsTriangles;

		JPH_INLINE bool				ShouldAbort() const
		{
			return mCollector.ShouldEarlyOut();
		}

		JPH_INLINE bool				ShouldVisitRangeBlock([[maybe_unused]] int inStackTop) const
		{
			return true;
		}

		JPH_INLINE int				VisitRangeBlock(Vec4Arg inBoundsMinX, Vec4Arg inBoundsMinY, Vec4Arg inBoundsMinZ, Vec4Arg inBoundsMaxX, Vec4Arg inBoundsMaxY, Vec4Arg inBoundsMaxZ, UVec4 &ioProperties, [[maybe_unused]] int inStackTop) const
		{
			// Scale the bounding boxes of this node
			Vec4 bounds_min_x, bounds_min_y, bounds_min_z, bounds_max_x, bounds_max_y, bounds_max_z;
			AABox4Scale(mScale2, inBoundsMinX, inBoundsMinY, inBoundsMinZ, inBoundsMaxX, inBoundsMaxY, inBoundsMaxZ, bounds_min_x, bounds_min_y, bounds_min_z, bounds_max_x, bounds_max_y, bounds_max_z);

			// Test which nodes collide
			UVec4 collides = AABox4VsBox(mBoundsOf1InSpaceOf2, bounds_min_x, bounds_min_y, bounds_min_z, bounds_max_x, bounds_max_y, bounds_max_z);
			return CountAndSortTrues(collides, ioProperties);
		}

		JPH_INLINE void				VisitTriangle(uint inX, uint inY, uint inTriangle, Vec3Arg inV0, Vec3Arg inV1, Vec3Arg inV2)
		{			
			// Create ID for triangle
			SubShapeID triangle_sub_shape_id = mShape2->EncodeSubShapeID(mSubShapeIDCreator2, inX, inY, inTriangle);

			// Determine active edges
			uint8 active_edges = mShape2->GetEdgeFlags(inX, inY, inTriangle);

			Collide(inV0, inV1, inV2, active_edges, triangle_sub_shape_id);
		}

		const HeightFieldShape *	mShape2;
		SubShapeIDCreator			mSubShapeIDCreator2;
	};

	Visitor visitor(shape1, inScale1, inScale2, inCenterOfMassTransform1, inCenterOfMassTransform2, inSubShapeIDCreator1.GetID(), inCollideShapeSettings, ioCollector);
	visitor.mShape2 = shape2;
	visitor.mSubShapeIDCreator2 = inSubShapeIDCreator2;
	shape2->WalkHeightField(visitor);
}

void HeightFieldShape::sCollideSphereVsHeightField(const Shape *inShape1, const Shape *inShape2, Vec3Arg inScale1, Vec3Arg inScale2, Mat44Arg inCenterOfMassTransform1, Mat44Arg inCenterOfMassTransform2, const SubShapeIDCreator &inSubShapeIDCreator1, const SubShapeIDCreator &inSubShapeIDCreator2, const CollideShapeSettings &inCollideShapeSettings, CollideShapeCollector &ioCollector)
{
	JPH_PROFILE_FUNCTION();

	// Get the shapes
	JPH_ASSERT(inShape1->GetSubType() == EShapeSubType::Sphere);
	JPH_ASSERT(inShape2->GetType() == EShapeType::HeightField);
	const SphereShape *shape1 = static_cast<const SphereShape *>(inShape1);
	const HeightFieldShape *shape2 = static_cast<const HeightFieldShape *>(inShape2);

	struct Visitor : public CollideSphereVsTriangles
	{
		using CollideSphereVsTriangles::CollideSphereVsTriangles;

		JPH_INLINE bool				ShouldAbort() const
		{
			return mCollector.ShouldEarlyOut();
		}

		JPH_INLINE bool				ShouldVisitRangeBlock([[maybe_unused]] int inStackTop) const
		{
			return true;
		}

		JPH_INLINE int				VisitRangeBlock(Vec4Arg inBoundsMinX, Vec4Arg inBoundsMinY, Vec4Arg inBoundsMinZ, Vec4Arg inBoundsMaxX, Vec4Arg inBoundsMaxY, Vec4Arg inBoundsMaxZ, UVec4 &ioProperties, [[maybe_unused]] int inStackTop) const
		{
			// Scale the bounding boxes of this node
			Vec4 bounds_min_x, bounds_min_y, bounds_min_z, bounds_max_x, bounds_max_y, bounds_max_z;
			AABox4Scale(mScale2, inBoundsMinX, inBoundsMinY, inBoundsMinZ, inBoundsMaxX, inBoundsMaxY, inBoundsMaxZ, bounds_min_x, bounds_min_y, bounds_min_z, bounds_max_x, bounds_max_y, bounds_max_z);

			// Test which nodes collide
			UVec4 collides = AABox4VsSphere(mSphereCenterIn2, mRadiusPlusMaxSeparationSq, bounds_min_x, bounds_min_y, bounds_min_z, bounds_max_x, bounds_max_y, bounds_max_z);
			return CountAndSortTrues(collides, ioProperties);
		}

		JPH_INLINE void				VisitTriangle(uint inX, uint inY, uint inTriangle, Vec3Arg inV0, Vec3Arg inV1, Vec3Arg inV2)
		{			
			// Create ID for triangle
			SubShapeID triangle_sub_shape_id = mShape2->EncodeSubShapeID(mSubShapeIDCreator2, inX, inY, inTriangle);

			// Determine active edges
			uint8 active_edges = mShape2->GetEdgeFlags(inX, inY, inTriangle);

			Collide(inV0, inV1, inV2, active_edges, triangle_sub_shape_id);
		}

		const HeightFieldShape *	mShape2;
		SubShapeIDCreator			mSubShapeIDCreator2;
	};

	Visitor visitor(shape1, inScale1, inScale2, inCenterOfMassTransform1, inCenterOfMassTransform2, inSubShapeIDCreator1.GetID(), inCollideShapeSettings, ioCollector);
	visitor.mShape2 = shape2;
	visitor.mSubShapeIDCreator2 = inSubShapeIDCreator2;
	shape2->WalkHeightField(visitor);
}

void HeightFieldShape::SaveBinaryState(StreamOut &inStream) const
{
	Shape::SaveBinaryState(inStream);

	inStream.Write(mOffset);
	inStream.Write(mScale);
	inStream.Write(mSampleCount);
	inStream.Write(mBlockSize);
	inStream.Write(mBitsPerSample);
	inStream.Write(mMinSample);
	inStream.Write(mMaxSample);
	inStream.Write(mRangeBlocks);
	inStream.Write(mHeightSamples);
	inStream.Write(mActiveEdges);
	inStream.Write(mMaterialIndices);
	inStream.Write(mNumBitsPerMaterialIndex);
}

void HeightFieldShape::RestoreBinaryState(StreamIn &inStream)
{
	Shape::RestoreBinaryState(inStream);

	inStream.Read(mOffset);
	inStream.Read(mScale);
	inStream.Read(mSampleCount);
	inStream.Read(mBlockSize);
	inStream.Read(mBitsPerSample);
	inStream.Read(mMinSample);
	inStream.Read(mMaxSample);
	inStream.Read(mRangeBlocks);
	inStream.Read(mHeightSamples);
	inStream.Read(mActiveEdges);
	inStream.Read(mMaterialIndices);
	inStream.Read(mNumBitsPerMaterialIndex);

	CacheValues();
}

void HeightFieldShape::SaveMaterialState(PhysicsMaterialList &outMaterials) const
{ 
	outMaterials = mMaterials;
}

void HeightFieldShape::RestoreMaterialState(const PhysicsMaterialRefC *inMaterials, uint inNumMaterials) 
{ 
	mMaterials.assign(inMaterials, inMaterials + inNumMaterials);
}

Shape::Stats HeightFieldShape::GetStats() const 
{ 
	return Stats(
		sizeof(*this) 
			+ mMaterials.size() * sizeof(Ref<PhysicsMaterial>) 
			+ mRangeBlocks.size() * sizeof(RangeBlock) 
			+ mHeightSamples.size() * sizeof(uint8) 
			+ mActiveEdges.size() * sizeof(uint8) 
			+ mMaterialIndices.size() * sizeof(uint8), 
		mHeightSamples.empty()? 0 : Square(mSampleCount - 1) * 2);
}

void HeightFieldShape::sRegister()
{
	ShapeFunctions &f = ShapeFunctions::sGet(EShapeSubType::HeightField);
	f.mConstruct = []() -> Shape * { return new HeightFieldShape; };
	f.mColor = Color::sPurple;

	for (EShapeSubType s : sConvexSubShapeTypes)
	{
		CollisionDispatch::sRegisterCollideShape(s, EShapeSubType::HeightField, sCollideConvexVsHeightField);
		CollisionDispatch::sRegisterCastShape(s, EShapeSubType::HeightField, sCastConvexVsHeightField);
	}

	// Specialized collision functions
	CollisionDispatch::sRegisterCollideShape(EShapeSubType::Sphere, EShapeSubType::HeightField, sCollideSphereVsHeightField);
	CollisionDispatch::sRegisterCastShape(EShapeSubType::Sphere, EShapeSubType::HeightField, sCastSphereVsHeightField);
}

JPH_NAMESPACE_END
