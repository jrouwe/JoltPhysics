// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "HairCalculateRenderPositionsBindings.h"
#include "HairCommon.h"
#include "HairCalculateRenderPositions.h"

JPH_SHADER_FUNCTION_BEGIN(void, main, cHairPerRenderVertexBatch, 1, 1)
	JPH_SHADER_PARAM_THREAD_ID(tid)
JPH_SHADER_FUNCTION_END
{
	// Check if this is a valid vertex
	uint vtx = tid.x;
	if (vtx >= cNumRenderVertices)
		return;

	float3 out_position = SkinRenderVertex(vtx);

	// Copy the vertex position to the output buffer
	gRenderPositions[vtx] = out_position;
}
