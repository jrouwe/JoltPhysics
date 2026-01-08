// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "HairApplyDeltaTransformBindings.h"
#include "HairCommon.h"

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

	// Transform the position so that it stays in the same place in world space (if influence is 1)
	float influence = GradientSamplerSample(material.mWorldTransformInfluence, strand_fraction);
	pos.mPosition += influence * (JPH_Mat44Mul3x4Vec3(cDeltaTransform, pos.mPosition) - pos.mPosition);

	// Linear interpolate the rotation based on the influence
	pos.mRotation = normalize(JPH_QuatMulQuat(influence * cDeltaTransformQuat + float4(0, 0, 0, 1.0f - influence), pos.mRotation));

	// Transform velocities
	vel.mVelocity = JPH_Mat44Mul3x3Vec3(cDeltaTransform, vel.mVelocity);
	vel.mAngularVelocity = JPH_Mat44Mul3x3Vec3(cDeltaTransform, vel.mAngularVelocity);

	// Write back vertex
	gPositions[vtx] = pos;
	gVelocities[vtx] = vel;
}
