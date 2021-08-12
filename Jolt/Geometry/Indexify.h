// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Geometry/Triangle.h>
#include <Geometry/IndexedTriangle.h>

namespace JPH {

/// Take a list of triangles and get the unique set of vertices and use them to create indexed triangles.
/// Vertices that are less than inVertexWeldDistance apart will be combined to a single vertex.
void Indexify(const TriangleList &inTriangles, VertexList &outVertices, IndexedTriangleList &outTriangles, float inVertexWeldDistance = 1.0e-4f);

/// Take a list of indexed triangles and unpack them
void Deindexify(const VertexList &inVertices, const IndexedTriangleList &inTriangles, TriangleList &outTriangles);

} // JPH
