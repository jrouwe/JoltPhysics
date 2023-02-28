// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2022 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Utils/ShapeCreator.h>

namespace ShapeCreator {

ShapeRefC CreateTorusMesh(float inTorusRadius, float inTubeRadius, uint inTorusSegments, uint inTubeSegments)
{
	uint cNumVertices = inTorusSegments * inTubeSegments;

	// Create torus
	MeshShapeSettings mesh;
	mesh.mTriangleVertices.reserve(cNumVertices);
	mesh.mIndexedTriangles.reserve(cNumVertices * 2);
	for (uint torus_segment = 0; torus_segment < inTorusSegments; ++torus_segment)
	{
		Mat44 rotation = Mat44::sRotation(Vec3::sAxisY(), float(torus_segment) * 2.0f * JPH_PI / inTorusSegments);
		for (uint tube_segment = 0; tube_segment < inTubeSegments; ++tube_segment)
		{
			// Create vertices
			float tube_angle = float(tube_segment) * 2.0f * JPH_PI / inTubeSegments;
			Vec3 pos = rotation * Vec3(inTorusRadius + inTubeRadius * Sin(tube_angle), inTubeRadius * Cos(tube_angle), 0);
			Float3 v;
			pos.StoreFloat3(&v);
			mesh.mTriangleVertices.push_back(v);

			// Create indices
			uint start_idx = torus_segment * inTubeSegments + tube_segment;
			mesh.mIndexedTriangles.emplace_back(start_idx, (start_idx + 1) % cNumVertices, (start_idx + inTubeSegments) % cNumVertices);
			mesh.mIndexedTriangles.emplace_back((start_idx + 1) % cNumVertices, (start_idx + inTubeSegments + 1) % cNumVertices, (start_idx + inTubeSegments) % cNumVertices);
		}
	}
	return mesh.Create().Get();
}

};
