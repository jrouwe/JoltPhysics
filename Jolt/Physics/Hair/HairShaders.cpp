// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Hair/HairShaders.h>
#include <Jolt/Shaders/HairStructs.h>

JPH_NAMESPACE_BEGIN

void HairShaders::Init(ComputeSystem *inComputeSystem)
{
	auto get = [](const ComputeShaderResult &inResult) { return inResult.IsValid()? inResult.Get() : nullptr; };

	mTeleportCS = get(inComputeSystem->CreateComputeShader("HairTeleport", cHairPerVertexBatch));
	mApplyDeltaTransformCS = get(inComputeSystem->CreateComputeShader("HairApplyDeltaTransform", cHairPerVertexBatch));
	mSkinVerticesCS = get(inComputeSystem->CreateComputeShader("HairSkinVertices", cHairPerVertexBatch));
	mSkinRootsCS = get(inComputeSystem->CreateComputeShader("HairSkinRoots", cHairPerStrandBatch));
	mApplyGlobalPoseCS = get(inComputeSystem->CreateComputeShader("HairApplyGlobalPose", cHairPerVertexBatch));
	mCalculateCollisionPlanesCS = get(inComputeSystem->CreateComputeShader("HairCalculateCollisionPlanes", cHairPerVertexBatch));
	mGridClearCS = get(inComputeSystem->CreateComputeShader("HairGridClear", cHairPerGridCellBatch));
	mGridAccumulateCS = get(inComputeSystem->CreateComputeShader("HairGridAccumulate", cHairPerVertexBatch));
	mGridNormalizeCS = get(inComputeSystem->CreateComputeShader("HairGridNormalize", cHairPerGridCellBatch));
	mIntegrateCS = get(inComputeSystem->CreateComputeShader("HairIntegrate", cHairPerVertexBatch));
	mUpdateRootsCS = get(inComputeSystem->CreateComputeShader("HairUpdateRoots", cHairPerStrandBatch));
	mUpdateStrandsCS = get(inComputeSystem->CreateComputeShader("HairUpdateStrands", cHairPerStrandBatch));
	mUpdateVelocityCS = get(inComputeSystem->CreateComputeShader("HairUpdateVelocity", cHairPerVertexBatch));
	mUpdateVelocityIntegrateCS = get(inComputeSystem->CreateComputeShader("HairUpdateVelocityIntegrate", cHairPerVertexBatch));
	mCalculateRenderPositionsCS = get(inComputeSystem->CreateComputeShader("HairCalculateRenderPositions", cHairPerRenderVertexBatch));
}

JPH_NAMESPACE_END
