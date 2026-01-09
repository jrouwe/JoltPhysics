// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "HairUpdateStrandsBindings.h"
#include "HairCommon.h"

void ApplyLRA(float3 inX0, float inMaxDist, JPH_IN_OUT(JPH_HairPosition) ioV0)
{
	float3 delta = ioV0.mPosition - inX0;
	float delta_len_sq = dot(delta, delta);
	if (delta_len_sq > JPH_Square(inMaxDist))
		ioV0.mPosition = inX0 + delta * inMaxDist / sqrt(delta_len_sq);
}

void ApplyStretchShear(JPH_IN_OUT(JPH_HairPosition) ioV0, JPH_IN_OUT(JPH_HairPosition) ioV1, float inLength01, float inInvMass0, float inInvMass1, JPH_IN(JPH_HairMaterial) inMaterial)
{
	// Inertia of a thin rod of length L ~ m * L^2, we take the maximum mass of the two vertices
	float rod_inv_mass = min(inInvMass0, inInvMass1) / inMaterial.mInertiaMultiplier; // / L^2, which we'll apply later

	// Equation 37 from "Position and Orientation Based Cosserat Rods" - Kugelstadt and Schoemer - SIGGRAPH 2016
	float denom = inInvMass0 + inInvMass1 + 4.0f * rod_inv_mass /* / L^2 * L^2 cancels */ + inMaterial.mStretchComplianceInvDeltaTimeSq;
	if (denom >= 1.0e-12f)
	{
		float3 x0 = ioV0.mPosition;
		float3 x1 = ioV1.mPosition;
		JPH_Quat rotation1 = ioV0.mRotation;
		float3 d3 = JPH_QuatRotateAxisZ(rotation1);
		float3 delta = (x1 - x0 - d3 * inLength01) / denom;
		ioV0.mPosition = x0 + inInvMass0 * delta;
		ioV1.mPosition = x1 - inInvMass1 * delta;
		// q * e3_bar = q * (0, 0, -1, 0) = [-qy, qx, -qw, qz]
		JPH_Quat q_e3_bar = JPH_Quat(-rotation1.y, rotation1.x, -rotation1.w, rotation1.z);
		rotation1 += (2.0f * rod_inv_mass / inLength01 /* / L^2 * L => / L */) * JPH_QuatImaginaryMulQuat(delta, q_e3_bar);
		ioV0.mRotation = normalize(rotation1);
	}
}

void ApplyBendTwist(JPH_IN_OUT(JPH_HairPosition) ioV0, JPH_IN_OUT(JPH_HairPosition) ioV1, JPH_Quat inOmega0, float inLength01, float inLength12, float inStrandFraction1, float inInvMass0, float inInvMass1, float inInvMass2, JPH_IN(JPH_HairMaterial) inMaterial)
{
	// Inertia of a thin rod of length L ~ m * L^2, we take the maximum mass of the two vertices
	float rod_inv_mass = min(inInvMass0, inInvMass1) / (inMaterial.mInertiaMultiplier * JPH_Square(inLength01));
	float rod2_inv_mass = min(inInvMass1, inInvMass2) / (inMaterial.mInertiaMultiplier * JPH_Square(inLength12));

	// Calculate multiplier for the bend compliance based on strand fraction
	float fraction = inStrandFraction1 * 3.0f;
	uint idx = uint(fraction);
	fraction = fraction - float(idx);
	float multiplier = inMaterial.mBendComplianceMultiplier[idx] * (1.0f - fraction) + inMaterial.mBendComplianceMultiplier[idx + 1] * fraction;

	// Equation 40 from "Position and Orientation Based Cosserat Rods" - Kugelstadt and Schoemer - SIGGRAPH 2016
	float denom = rod_inv_mass + rod2_inv_mass + inMaterial.mBendComplianceInvDeltaTimeSq * multiplier;
	if (denom >= 1.0e-12f)
	{
		JPH_Quat rotation1 = ioV0.mRotation;
		JPH_Quat rotation2 = ioV1.mRotation;
		JPH_Quat omega = JPH_QuatMulQuat(JPH_QuatConjugate(rotation1), rotation2);
		JPH_Quat omega_min_omega0 = omega - inOmega0;
		JPH_Quat omega_plus_omega0 = omega + inOmega0;
		// Take the shortest of the two rotations
		JPH_Quat delta_omega = dot(omega_plus_omega0, omega_plus_omega0) < dot(omega_min_omega0, omega_min_omega0) ? omega_plus_omega0 : omega_min_omega0;
		delta_omega /= denom;
		delta_omega.w = 0; // Scalar part needs to be zero because the real part of the Darboux vector doesn't vanish, see text between eq. 39 and 40.
		JPH_Quat delta_rod2 = rod2_inv_mass * JPH_QuatMulQuat(rotation1, delta_omega);
		rotation1 += rod_inv_mass * JPH_QuatMulQuat(rotation2, delta_omega);
		rotation2 -= delta_rod2;
		ioV0.mRotation = normalize(rotation1);
		ioV1.mRotation = normalize(rotation2);
	}
}

JPH_SHADER_FUNCTION_BEGIN(void, main, cHairPerStrandBatch, 1, 1)
	JPH_SHADER_PARAM_THREAD_ID(tid)
JPH_SHADER_FUNCTION_END
{
	// Check if this is a valid strand
	uint strand_idx = tid.x;
	if (strand_idx >= cNumStrands)
		return;
	uint strand_vtx_count = GetStrandVertexCount(gStrandVertexCounts, strand_idx);

	// Load the material
	JPH_HairMaterial material = gMaterials[GetStrandMaterialIndex(gStrandMaterialIndex, strand_idx)];

	// Load the first two vertices
	uint vtx_idx_to_load = strand_idx;
	float inv_mass0 = GetVertexInvMass(gVerticesFixed, vtx_idx_to_load);
	float length0 = gInitialLengths[vtx_idx_to_load];
	JPH_HairPosition v0 = gPositions[vtx_idx_to_load];

	// LRA: Vertex everything is attached to
	float3 x0 = gInitialPositions[vtx_idx_to_load];

	// LRA: Tracks the distance from the first vertex
	float max_dist = length0;

	vtx_idx_to_load += cNumStrands;
	float inv_mass1 = GetVertexInvMass(gVerticesFixed, vtx_idx_to_load);
	float strand_fraction1 = GetVertexStrandFraction(gStrandFractions, vtx_idx_to_load);
	float length1 = gInitialLengths[vtx_idx_to_load];
	JPH_HairPosition v1 = gPositions[vtx_idx_to_load];

	// Process 2nd vertex
	if (material.mEnableLRA && inv_mass1 > 0.0f)
		ApplyLRA(x0, max_dist, v1);
	max_dist += length1;

	uint vtx_idx_to_retire = strand_idx;
	for (uint vtx = 2; vtx < strand_vtx_count; ++vtx)
	{
		// Get the initial rotation difference from the middle vertex
		JPH_Quat omega0 = JPH_QuatDecompress(gOmega0s[vtx_idx_to_load]);

		// Load the next vertex
		vtx_idx_to_load += cNumStrands;
		float inv_mass2 = GetVertexInvMass(gVerticesFixed, vtx_idx_to_load);
		float strand_fraction2 = GetVertexStrandFraction(gStrandFractions, vtx_idx_to_load);
		float length2 = gInitialLengths[vtx_idx_to_load];
		JPH_HairPosition v2 = gPositions[vtx_idx_to_load];

		// Process newly added vertex
		if (material.mEnableLRA && inv_mass2 > 0.0f)
			ApplyLRA(x0, max_dist, v2);
		max_dist += length2;

		// Stitched mode as per Strand-based Hair System - Pedersen - SIGGRAPH 2022
		ApplyStretchShear(v1, v2, length1, inv_mass1, inv_mass2, material);
		ApplyStretchShear(v0, v1, length0, inv_mass0, inv_mass1, material);
		ApplyBendTwist(v0, v1, omega0, length0, length1, strand_fraction1, inv_mass0, inv_mass1, inv_mass2, material);

		// Retire vertex
		gPositions[vtx_idx_to_retire] = v0;
		vtx_idx_to_retire += cNumStrands;

		// Shift the vertices
		inv_mass0 = inv_mass1;
		inv_mass1 = inv_mass2;
		strand_fraction1 = strand_fraction2;
		length0 = length1;
		length1 = length2;
		v0 = v1;
		v1 = v2;
	}

	// Retire 2nd to last vertex
	gPositions[vtx_idx_to_retire] = v0;
	vtx_idx_to_retire += cNumStrands;

	// Cannot calculate rotation for last vertex, take the 2nd last
	v1.mRotation = v0.mRotation;

	// Retire last vertex
	gPositions[vtx_idx_to_retire] = v1;
}
