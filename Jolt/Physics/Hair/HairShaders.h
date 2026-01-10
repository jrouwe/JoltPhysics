// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Core/Reference.h>
#include <Jolt/Compute/ComputeSystem.h>

JPH_NAMESPACE_BEGIN

/// This class loads the shaders used by the hair system. This can be shared among all hair instances.
class JPH_EXPORT HairShaders : public RefTarget<HairShaders>
{
public:
	/// Loads all shaders
	/// Note that if you want to run the sim on CPU you need call HairRegisterShaders first.
	void				Init(ComputeSystem *inComputeSystem);

	Ref<ComputeShader>	mTeleportCS;
	Ref<ComputeShader>	mApplyDeltaTransformCS;
	Ref<ComputeShader>	mSkinVerticesCS;
	Ref<ComputeShader>	mSkinRootsCS;
	Ref<ComputeShader>	mApplyGlobalPoseCS;
	Ref<ComputeShader>	mCalculateCollisionPlanesCS;
	Ref<ComputeShader>	mGridClearCS;
	Ref<ComputeShader>	mGridAccumulateCS;
	Ref<ComputeShader>	mGridNormalizeCS;
	Ref<ComputeShader>	mIntegrateCS;
	Ref<ComputeShader>	mUpdateRootsCS;
	Ref<ComputeShader>	mUpdateStrandsCS;
	Ref<ComputeShader>	mUpdateVelocityCS;
	Ref<ComputeShader>	mUpdateVelocityIntegrateCS;
	Ref<ComputeShader>	mCalculateRenderPositionsCS;
};

JPH_NAMESPACE_END
