// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <TriangleSplitter/TriangleSplitter.h>

namespace JPH {

/// Splitter using center of bounding box with longest axis
class TriangleSplitterLongestAxis : public TriangleSplitter
{
public:
	/// Constructor
							TriangleSplitterLongestAxis(const VertexList &inVertices, const IndexedTriangleList &inTriangles);

	// See TriangleSplitter::GetStats
	virtual void			GetStats(Stats &outStats) const override
	{
		outStats.mSplitterName = "TriangleSplitterLongestAxis";
	}

	// See TriangleSplitter::Split
	virtual bool			Split(const Range &inTriangles, Range &outLeft, Range &outRight) override;
};

} // JPH