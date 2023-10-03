// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Geometry/Indexify.h>
#include <Jolt/Geometry/AABox.h>

JPH_NAMESPACE_BEGIN

static JPH_INLINE const Float3 &sIndexifyGetFloat3(const TriangleList &inTriangles, uint32 inVertexIndex)
{
	return inTriangles[inVertexIndex / 3].mV[inVertexIndex % 3];
}

static JPH_INLINE Vec3 sIndexifyGetVec3(const TriangleList &inTriangles, uint32 inVertexIndex)
{
	return Vec3::sLoadFloat3Unsafe(sIndexifyGetFloat3(inTriangles, inVertexIndex));
}

static void sIndexifyVerticesBruteForce(const TriangleList &inTriangles, const uint32 *inVertexIndices, const uint32 *inVertexIndicesEnd, Array<uint32> &ioWeldedVertices, float inVertexWeldDistance)
{
	float weld_dist_sq = Square(inVertexWeldDistance);

	// Compare every vertex
	for (const uint32 *v1_idx = inVertexIndices; v1_idx < inVertexIndicesEnd; ++v1_idx)
	{
		Vec3 v1 = sIndexifyGetVec3(inTriangles, *v1_idx);

		// with every other vertex...
		for (const uint32 *v2_idx = v1_idx + 1; v2_idx < inVertexIndicesEnd; ++v2_idx)
		{
			Vec3 v2 = sIndexifyGetVec3(inTriangles, *v2_idx);

			// If they're weldable
			if ((v2 - v1).LengthSq() <= weld_dist_sq)
			{
				// Order the vertices
				uint32 lowest = min(*v1_idx, *v2_idx);
				uint32 highest = max(*v1_idx, *v2_idx);

				// Find the lowest vertex
				uint32 new_lowest = ioWeldedVertices[lowest];
				while (new_lowest < lowest)
				{
					ioWeldedVertices[lowest] = new_lowest;
					lowest = new_lowest;
					new_lowest = ioWeldedVertices[lowest];
				}

				// Link highest to lowest
				ioWeldedVertices[highest] = lowest;
			}
		}
	}
}

static void sIndexifyVerticesRecursively(const TriangleList &inTriangles, uint32 *ioVertexIndices, uint inNumVertices, uint32 *ioScratch, Array<uint32> &ioWeldedVertices, float inVertexWeldDistance, uint inMaxRecursion)
{
	// Check if we have few enough vertices to do a brute force search
	// Or if we've recursed too deep (this means we chipped off a few vertices each iteration because all points are very close)
	if (inNumVertices <= 8 || inMaxRecursion == 0)
	{
		sIndexifyVerticesBruteForce(inTriangles, ioVertexIndices, ioVertexIndices + inNumVertices, ioWeldedVertices, inVertexWeldDistance);
		return;
	}

	// Calculate bounds
	AABox bounds;
	for (const uint32 *v = ioVertexIndices, *v_end = ioVertexIndices + inNumVertices; v < v_end; ++v)
		bounds.Encapsulate(sIndexifyGetVec3(inTriangles, *v));

	// Determine split plane
	int split_axis = bounds.GetExtent().GetHighestComponentIndex();
	float split_value = bounds.GetCenter()[split_axis];

	// Partition vertices
	uint32 *v_read = ioVertexIndices, *v_write = ioVertexIndices, *v_end = ioVertexIndices + inNumVertices;
	uint32 *scratch = ioScratch;
	while (v_read < v_end)
	{
		// Calculate distance to plane
		float distance_to_split_plane = sIndexifyGetFloat3(inTriangles, *v_read)[split_axis] - split_value;
		if (distance_to_split_plane < -inVertexWeldDistance)
		{
			// Vertex is on the right side
			*v_write = *v_read;
			++v_read;
			++v_write;
		}
		else if (distance_to_split_plane > inVertexWeldDistance)
		{
			// Vertex is on the wrong side, swap with the last vertex
			--v_end;
			swap(*v_read, *v_end);
		}
		else
		{
			// Vertex is too close to the split plane, it goes on both sides
			*scratch++ = *v_read++;
		}
	}

	// Check if we made any progress
	uint num_vertices_on_both_sides = (uint)(scratch - ioScratch);
	if (num_vertices_on_both_sides == inNumVertices)
	{
		sIndexifyVerticesBruteForce(inTriangles, ioVertexIndices, ioVertexIndices + inNumVertices, ioWeldedVertices, inVertexWeldDistance);
		return;
	}

	// Calculate how we classified the vertices
	uint num_vertices_left = (uint)(v_write - ioVertexIndices);
	uint num_vertices_right = (uint)(ioVertexIndices + inNumVertices - v_end);
	JPH_ASSERT(num_vertices_left + num_vertices_right + num_vertices_on_both_sides == inNumVertices);
	memcpy(v_write, ioScratch, num_vertices_on_both_sides * sizeof(uint32));

	// Recurse
	uint max_recursion = inMaxRecursion - 1;
	sIndexifyVerticesRecursively(inTriangles, ioVertexIndices, num_vertices_left + num_vertices_on_both_sides, ioScratch, ioWeldedVertices, inVertexWeldDistance, max_recursion);
	sIndexifyVerticesRecursively(inTriangles, ioVertexIndices + num_vertices_left, num_vertices_right + num_vertices_on_both_sides, ioScratch, ioWeldedVertices, inVertexWeldDistance, max_recursion);
}

void Indexify(const TriangleList &inTriangles, VertexList &outVertices, IndexedTriangleList &outTriangles, float inVertexWeldDistance)
{
	uint num_triangles = (uint)inTriangles.size();
	uint num_vertices = num_triangles * 3;

	// Create a list of all vertex indices
	Array<uint32> vertex_indices;
	vertex_indices.resize(num_vertices);
	for (uint i = 0; i < num_vertices; ++i)
		vertex_indices[i] = i;

	// Link each vertex to itself
	Array<uint32> welded_vertices;
	welded_vertices.resize(num_vertices);
	for (uint i = 0; i < num_vertices; ++i)
		welded_vertices[i] = i;

	// A scope to free memory used by the scratch array
	{
		// Some scratch memory, used for the vertices that fall in both partitions
		Array<uint32> scratch;
		scratch.resize(num_vertices);

		// Recursively split the vertices
		sIndexifyVerticesRecursively(inTriangles, vertex_indices.data(), num_vertices, scratch.data(), welded_vertices, inVertexWeldDistance, 32);
	}

	// Do a pass to complete the welding, linking each vertex to the vertex it is welded to
	// (and since we're going from 0 to N we can be sure that the vertex we're linking to is already linked to the lowest vertex)
	uint num_resulting_vertices = 0;
	for (uint i = 0; i < num_vertices; ++i)
	{
		welded_vertices[i] = welded_vertices[welded_vertices[i]];
		if (welded_vertices[i] == i)
			++num_resulting_vertices;
	}

	// Collect the vertices
	outVertices.clear();
	outVertices.reserve(num_resulting_vertices);
	for (uint i = 0; i < num_vertices; ++i)
		if (welded_vertices[i] == i)
		{
			// New vertex
			welded_vertices[i] = (uint32)outVertices.size();
			outVertices.push_back(sIndexifyGetFloat3(inTriangles, i));
		}
		else
		{
			// Reused vertex, remap index
			welded_vertices[i] = welded_vertices[welded_vertices[i]];
		}

	// Create indexed triangles
	outTriangles.clear();
	outTriangles.reserve(num_triangles);
	for (uint t = 0; t < num_triangles; ++t)
	{
		IndexedTriangle it;
		it.mMaterialIndex = inTriangles[t].mMaterialIndex;
		for (int v = 0; v < 3; ++v)
			it.mIdx[v] = welded_vertices[t * 3 + v];
		if (!it.IsDegenerate(outVertices))
			outTriangles.push_back(it);
	}
}

void Deindexify(const VertexList &inVertices, const IndexedTriangleList &inTriangles, TriangleList &outTriangles)
{
	outTriangles.resize(inTriangles.size());
	for (size_t t = 0; t < inTriangles.size(); ++t)
	{
		outTriangles[t].mMaterialIndex = inTriangles[t].mMaterialIndex;
		for (int v = 0; v < 3; ++v)
			outTriangles[t].mV[v] = inVertices[inTriangles[t].mIdx[v]];
	}
}

JPH_NAMESPACE_END
