// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "HairSkinRootsBindings.h"
#include "HairCommon.h"

JPH_SHADER_FUNCTION_BEGIN(void, main, cHairPerStrandBatch, 1, 1)
	JPH_SHADER_PARAM_THREAD_ID(tid)
JPH_SHADER_FUNCTION_END
{
	// Check if this is a valid strand
	uint strand_idx = tid.x;
	if (strand_idx >= cNumStrands)
		return;

	JPH_HairSkinPoint sp = gSkinPoints[strand_idx];

	// Get the vertices of the attached triangle
	uint tri_idx = sp.mTriangleIndex * 3;
	float3 v0 = JPH_Mat44Mul3x3Vec3(cScalpToHead, gScalpVertices[gScalpTriangles[tri_idx + 0]]);
	float3 v1 = JPH_Mat44Mul3x3Vec3(cScalpToHead, gScalpVertices[gScalpTriangles[tri_idx + 1]]);
	float3 v2 = JPH_Mat44Mul3x3Vec3(cScalpToHead, gScalpVertices[gScalpTriangles[tri_idx + 2]]);

	JPH_HairPosition root;

	// Set the position of the root
	root.mPosition = sp.mU * v0 + sp.mV * v1 + (1.0f - sp.mU - sp.mV) * v2 + cScalpToHead[3].xyz;

	// Get tangent vector
	float3 tangent = normalize(v1 - v0);

	// Get normal of the triangle
	float3 normal = normalize(cross(tangent, v2 - v0));

	// Calculate basis for the triangle
	float3 binormal = cross(tangent, normal);
	JPH_Quat triangle_basis = JPH_QuatFromMat33(normal, binormal, tangent);

	// Calculate the new Bishop frame of the root
	root.mRotation = JPH_QuatMulQuat(triangle_basis, JPH_QuatDecompress(sp.mToBishop));

	gPositions[strand_idx] = root;

	// Calculate the transform that transforms the stored global pose to the space of the skinned root of the strand
	JPH_HairGlobalPoseTransform transform;
	transform.mRotation = JPH_QuatMulQuat(root.mRotation, JPH_QuatConjugate(JPH_QuatDecompress(gInitialBishops[strand_idx])));
	transform.mPosition = root.mPosition - JPH_QuatMulVec3(transform.mRotation, gInitialPositions[strand_idx]);
	gGlobalPoseTransforms[strand_idx] = transform;
}
