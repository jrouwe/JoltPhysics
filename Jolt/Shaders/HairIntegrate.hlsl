// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "HairIntegrateBindings.h"
#include "HairCommon.h"
#include "HairIntegrate.h"

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
	JPH_HairPosition pos = gPositions[vtx];
	JPH_HairVelocity vel = gVelocities[vtx];

	// Update previous position
	gPreviousPositions[vtx] = pos;

	ApplyGrid(pos, vel, material, strand_fraction);
	Integrate(pos, vel, material, strand_fraction);
	gPositions[vtx] = pos;
}
