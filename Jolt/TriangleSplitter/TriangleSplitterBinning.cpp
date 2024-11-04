// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/TriangleSplitter/TriangleSplitterBinning.h>

 JPH_NAMESPACE_BEGIN

TriangleSplitterBinning::TriangleSplitterBinning(const VertexList &inVertices, const IndexedTriangleList &inTriangles, uint inMinNumBins, uint inMaxNumBins, uint inNumTrianglesPerBin) :
	TriangleSplitter(inVertices, inTriangles),
	mMinNumBins(inMinNumBins),
	mMaxNumBins(inMaxNumBins),
	mNumTrianglesPerBin(inNumTrianglesPerBin)
{
	mBins.resize(mMaxNumBins * 3); // mMaxNumBins per dimension
}

bool TriangleSplitterBinning::Split(const Range &inTriangles, Range &outLeft, Range &outRight)
{
	// Calculate bounds for this range
	AABox centroid_bounds;
	for (uint t = inTriangles.mBegin; t < inTriangles.mEnd; ++t)
		centroid_bounds.Encapsulate(Vec3(mCentroids[mSortedTriangleIdx[t]]));

	float best_cp = FLT_MAX;
	uint best_dim = 0xffffffff;
	float best_split = 0;

	// Bin in all dimensions
	uint num_bins = Clamp(inTriangles.Count() / mNumTrianglesPerBin, mMinNumBins, mMaxNumBins);

	// Initialize bins
	Vec3 bounds_min = centroid_bounds.mMin;
	Vec3 bounds_size = centroid_bounds.mMax - bounds_min;
	for (uint dim = 0; dim < 3; ++dim)
	{
		for (uint b = 0; b < num_bins; ++b)
		{
			Bin &bin = mBins[num_bins * dim + b];
			bin.mBounds.SetEmpty();
			bin.mMinCentroid = bounds_min + bounds_size * (float)(b + 1) / (float)num_bins;
			bin.mNumTriangles = 0;
		}
	}

	// Bin all triangles in all dimensions at once
	for (uint t = inTriangles.mBegin; t < inTriangles.mEnd; ++t)
	{
		AABox tri_t_bounds;
		tri_t_bounds.Encapsulate(mVertices, GetTriangle(t));

		Vec3 tri_t_centroid = Vec3(mCentroids[mSortedTriangleIdx[t]]);

		Vec3 bin_vec = ((tri_t_centroid - bounds_min) / bounds_size) * (float)num_bins;

		// Dimension 0:
		{
			// Select bin
			uint bin_no = min(uint(bin_vec.GetX()), num_bins - 1);
			Bin &bin = mBins[bin_no];

			// Accumulate triangle in bin
			bin.mBounds.Encapsulate(tri_t_bounds);
			bin.mMinCentroid = Vec3::sMin(bin.mMinCentroid, tri_t_centroid);
			bin.mNumTriangles++;
		}

		// Dimension 1:
		{
			// Select bin
			uint bin_no = num_bins + min(uint(bin_vec.GetY()), num_bins - 1);
			Bin &bin = mBins[bin_no];

			// Accumulate triangle in bin
			bin.mBounds.Encapsulate(tri_t_bounds);
			bin.mMinCentroid = Vec3::sMin(bin.mMinCentroid, tri_t_centroid);
			bin.mNumTriangles++;
		}

		// Dimension 2:
		{
			// Select bin
			uint bin_no = num_bins * 2 + min(uint(bin_vec.GetZ()), num_bins - 1);
			Bin &bin = mBins[bin_no];

			// Accumulate triangle in bin
			bin.mBounds.Encapsulate(tri_t_bounds);
			bin.mMinCentroid = Vec3::sMin(bin.mMinCentroid, tri_t_centroid);
			bin.mNumTriangles++;
		}
	}

	for (uint dim = 0; dim < 3; ++dim)
	{
		// Calculate totals left to right
		AABox prev_bounds;
		int prev_triangles = 0;
		for (uint b = 0; b < num_bins; ++b)
		{
			Bin &bin = mBins[num_bins * dim + b];
			bin.mBoundsAccumulatedLeft = prev_bounds; // Don't include this node as we'll take a split on the left side of the bin
			bin.mNumTrianglesAccumulatedLeft = prev_triangles;
			prev_bounds.Encapsulate(bin.mBounds);
			prev_triangles += bin.mNumTriangles;
		}

		// Calculate totals right to left
		prev_bounds.SetEmpty();
		prev_triangles = 0;
		for (int b = num_bins - 1; b >= 0; --b)
		{
			Bin &bin = mBins[num_bins * dim + b];
			prev_bounds.Encapsulate(bin.mBounds);
			prev_triangles += bin.mNumTriangles;
			bin.mBoundsAccumulatedRight = prev_bounds;
			bin.mNumTrianglesAccumulatedRight = prev_triangles;
		}

		// Get best splitting plane
		for (uint b = 1; b < num_bins; ++b) // Start at 1 since selecting bin 0 would result in everything ending up on the right side
		{
			// Calculate surface area heuristic and see if it is better than the current best
			const Bin &bin = mBins[num_bins * dim + b];
			float cp = bin.mBoundsAccumulatedLeft.GetSurfaceArea() * bin.mNumTrianglesAccumulatedLeft + bin.mBoundsAccumulatedRight.GetSurfaceArea() * bin.mNumTrianglesAccumulatedRight;
			if (cp < best_cp)
			{
				best_cp = cp;
				best_dim = dim;
				best_split = bin.mMinCentroid[dim];
			}
		}
	}

	// No split found?
	if (best_dim == 0xffffffff)
		return false;

	return SplitInternal(inTriangles, best_dim, best_split, outLeft, outRight);
}

JPH_NAMESPACE_END
