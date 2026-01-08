// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "HairApplyGlobalPoseBindings.h"
#include "HairCommon.h"
#include "HairApplyGlobalPose.h"

JPH_SHADER_FUNCTION_BEGIN(void, main, cHairPerVertexBatch, 1, 1)
	JPH_SHADER_PARAM_THREAD_ID(tid)
JPH_SHADER_FUNCTION_END
{
	// Check if this is a valid vertex
	uint vtx = tid.x + cNumStrands; // Skip the root of each strand, it's fixed
	if (vtx >= cNumVertices)
		return;
	if (IsVertexFixed(gVerticesFixed, vtx))
		return;

	// Load the material
	uint strand_idx = vtx % cNumStrands;
	JPH_HairMaterial material = gMaterials[GetStrandMaterialIndex(gStrandMaterialIndex, strand_idx)];

	// Load the vertex
	float strand_fraction = GetVertexStrandFraction(gStrandFractions, vtx);
	float3 initial_pos = gInitialPositions[vtx];
	float4 initial_bishop = JPH_QuatDecompress(gInitialBishops[vtx]);
	JPH_HairGlobalPoseTransform global_pose_transform = gGlobalPoseTransforms[strand_idx];

	// Only apply global pose
	JPH_HairPosition pos;
	pos.mPosition = float3(0, 0, 0);
	pos.mRotation = float4(0, 0, 0, 0);
	ApplyGlobalPose(pos, initial_pos, initial_bishop, global_pose_transform, material, strand_fraction);

	// Write back vertex
	gPositions[vtx] = pos;
}
