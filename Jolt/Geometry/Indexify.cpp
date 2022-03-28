// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Geometry/Indexify.h>

JPH_SUPPRESS_WARNINGS_STD_BEGIN
#include <unordered_map>
JPH_SUPPRESS_WARNINGS_STD_END

JPH_NAMESPACE_BEGIN

void Indexify(const TriangleList &inTriangles, VertexList &outVertices, IndexedTriangleList &outTriangles, float inVertexWeldDistance)
{
	float weld_dist_sq = Square(inVertexWeldDistance);

	// Ensure that output vertices are empty before we begin
	outVertices.clear();

	// Find unique vertices
	unordered_map<Float3, uint32> vertex_map;
	for (const Triangle &t : inTriangles)
		for (const Float3 &v : t.mV)
		{
			// Try to insert element
			auto insert = vertex_map.insert(pair<Float3, uint32>(v, 0));
			if (insert.second)
			{
				// Newly inserted, see if we can share
				bool found = false;
				for (size_t i = 0; i < outVertices.size(); ++i)
				{
					const Float3 &other = outVertices[i];
					if (Square(other.x - v.x) + Square(other.y - v.y) + Square(other.z - v.z) <= weld_dist_sq)
					{
						insert.first->second = (uint32)i;
						found = true;
						break;
					}
				}

				if (!found)
				{
					// Can't share, add vertex
					insert.first->second = (uint32)outVertices.size();
					outVertices.push_back(v);
				}
			}
		}

	// Create indexed triangles
	outTriangles.clear();
	outTriangles.reserve(inTriangles.size());
	for (const Triangle &t : inTriangles)
	{
		IndexedTriangle it;
		it.mMaterialIndex = t.mMaterialIndex;
		for (int j = 0; j < 3; ++j)
			it.mIdx[j] = vertex_map[t.mV[j]];
		if (!it.IsDegenerate())
			outTriangles.push_back(it);
	}
}

void Deindexify(const VertexList &inVertices, const IndexedTriangleList &inTriangles, TriangleList &outTriangles)
{
	outTriangles.resize(inTriangles.size());
	for (size_t t = 0; t < inTriangles.size(); ++t)
		for (int v = 0; v < 3; ++v)
			outTriangles[t].mV[v] = inVertices[inTriangles[t].mIdx[v]];
}

JPH_NAMESPACE_END
