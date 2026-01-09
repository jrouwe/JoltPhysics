// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "HairTeleportBindings.h"
#include "HairCommon.h"

JPH_SHADER_FUNCTION_BEGIN(void, main, cHairPerVertexBatch, 1, 1)
	JPH_SHADER_PARAM_THREAD_ID(tid)
JPH_SHADER_FUNCTION_END
{
	// Check if this is a valid vertex
	uint vtx = tid.x;
	if (vtx >= cNumVertices)
		return;

	// Initialize position based on the initial vertex data
	JPH_HairPosition pos;
	pos.mPosition = gInitialPositions[vtx];
	pos.mRotation = JPH_QuatDecompress(gInitialBishops[vtx]);
	gPositions[vtx] = pos;

	// Initialize velocity to zero
	JPH_HairVelocity vel;
	vel.mVelocity = float3(0, 0, 0);
	vel.mAngularVelocity = float3(0, 0, 0);
	gVelocities[vtx] = vel;
}
