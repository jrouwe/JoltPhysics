// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Geometry/Triangle.h>
#include <Jolt/Geometry/IndexedTriangle.h>

JPH_NAMESPACE_BEGIN

/// Take a list of triangles and get the unique set of vertices and use them to create indexed triangles.
/// Vertices that are less than inVertexWeldDistance apart will be combined to a single vertex.
void Indexify(const TriangleList &inTriangles, VertexList &outVertices, IndexedTriangleList &outTriangles, float inVertexWeldDistance = 1.0e-4f);

/// Take a list of indexed triangles and unpack them
void Deindexify(const VertexList &inVertices, const IndexedTriangleList &inTriangles, TriangleList &outTriangles);

JPH_NAMESPACE_END
