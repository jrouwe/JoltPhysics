// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "HairUpdateRootsBindings.h"
#include "HairCommon.h"

JPH_SHADER_FUNCTION_BEGIN(void, main, cHairPerStrandBatch, 1, 1)
	JPH_SHADER_PARAM_THREAD_ID(tid)
JPH_SHADER_FUNCTION_END
{
	// Check if this is a valid strand
	uint strand_idx = tid.x;
	if (strand_idx >= cNumStrands)
		return;

	float inv_fraction = 1.0f - cIterationFraction;

	JPH_HairPosition pos = gPositions[strand_idx];
	JPH_HairPosition target_pos = gTargetPositions[strand_idx];
	pos.mPosition = pos.mPosition * inv_fraction + target_pos.mPosition * cIterationFraction;
	pos.mRotation = normalize(pos.mRotation * inv_fraction + target_pos.mRotation * cIterationFraction);
	gPositions[strand_idx] = pos;

	JPH_HairGlobalPoseTransform transf = gGlobalPoseTransforms[strand_idx];
	JPH_HairGlobalPoseTransform target_transf = gTargetGlobalPoseTransforms[strand_idx];
	transf.mPosition = transf.mPosition * inv_fraction + target_transf.mPosition * cIterationFraction;
	transf.mRotation = normalize(transf.mRotation * inv_fraction + target_transf.mRotation * cIterationFraction);
	gGlobalPoseTransforms[strand_idx] = transf;
}
