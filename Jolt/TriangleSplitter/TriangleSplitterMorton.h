// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/TriangleSplitter/TriangleSplitter.h>

namespace JPH {

/// Splitter using Morton codes, see: http://devblogs.nvidia.com/parallelforall/thinking-parallel-part-iii-tree-construction-gpu/
class TriangleSplitterMorton : public TriangleSplitter
{
public:
	/// Constructor
							TriangleSplitterMorton(const VertexList &inVertices, const IndexedTriangleList &inTriangles);

	// See TriangleSplitter::GetStats
	virtual void			GetStats(Stats &outStats) const override
	{
		outStats.mSplitterName = "TriangleSplitterMorton";
	}

	// See TriangleSplitter::Split
	virtual bool			Split(const Range &inTriangles, Range &outLeft, Range &outRight) override;

private:
	// Precalculated Morton codes
	vector<uint32>			mMortonCodes;
};

} // JPH