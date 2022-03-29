// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/TriangleGrouper/TriangleGrouper.h>

JPH_NAMESPACE_BEGIN

/// A class that groups triangles in batches of N according to morton code of centroid.
/// Time complexity: O(N log(N))
class TriangleGrouperMorton : public TriangleGrouper
{
public:
	// See: TriangleGrouper::Group
	virtual void			Group(const VertexList &inVertices, const IndexedTriangleList &inTriangles, int inGroupSize, vector<uint> &outGroupedTriangleIndices) override;
};

JPH_NAMESPACE_END
