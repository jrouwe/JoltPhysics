// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#ifdef JPH_USE_CPU_COMPUTE

#include <Jolt/Shaders/HairWrapper.h>

#define JPH_SHADER_NAME HairTeleport
#include <Jolt/Compute/CPU/WrapShaderBegin.h>
#include "HairTeleport.hlsl"
#include <Jolt/Compute/CPU/WrapShaderBindings.h>
#include "HairTeleportBindings.h"
#include <Jolt/Compute/CPU/WrapShaderEnd.h>

#define JPH_SHADER_NAME HairApplyDeltaTransform
#include <Jolt/Compute/CPU/WrapShaderBegin.h>
#include "HairApplyDeltaTransform.hlsl"
#include <Jolt/Compute/CPU/WrapShaderBindings.h>
#include "HairApplyDeltaTransformBindings.h"
#include <Jolt/Compute/CPU/WrapShaderEnd.h>

#define JPH_SHADER_NAME HairSkinVertices
#include <Jolt/Compute/CPU/WrapShaderBegin.h>
#include "HairSkinVertices.hlsl"
#include <Jolt/Compute/CPU/WrapShaderBindings.h>
#include "HairSkinVerticesBindings.h"
#include <Jolt/Compute/CPU/WrapShaderEnd.h>

#define JPH_SHADER_NAME HairSkinRoots
#include <Jolt/Compute/CPU/WrapShaderBegin.h>
#include "HairSkinRoots.hlsl"
#include <Jolt/Compute/CPU/WrapShaderBindings.h>
#include "HairSkinRootsBindings.h"
#include <Jolt/Compute/CPU/WrapShaderEnd.h>

#define JPH_SHADER_NAME HairApplyGlobalPose
#include <Jolt/Compute/CPU/WrapShaderBegin.h>
#include "HairApplyGlobalPose.hlsl"
#include <Jolt/Compute/CPU/WrapShaderBindings.h>
#include "HairApplyGlobalPoseBindings.h"
#include <Jolt/Compute/CPU/WrapShaderEnd.h>

#define JPH_SHADER_NAME HairCalculateCollisionPlanes
#include <Jolt/Compute/CPU/WrapShaderBegin.h>
#include "HairCalculateCollisionPlanes.hlsl"
#include <Jolt/Compute/CPU/WrapShaderBindings.h>
#include "HairCalculateCollisionPlanesBindings.h"
#include <Jolt/Compute/CPU/WrapShaderEnd.h>

#define JPH_SHADER_NAME HairGridClear
#include <Jolt/Compute/CPU/WrapShaderBegin.h>
#include "HairGridClear.hlsl"
#include <Jolt/Compute/CPU/WrapShaderBindings.h>
#include "HairGridClearBindings.h"
#include <Jolt/Compute/CPU/WrapShaderEnd.h>

#define JPH_SHADER_NAME HairGridAccumulate
#include <Jolt/Compute/CPU/WrapShaderBegin.h>
#include "HairGridAccumulate.hlsl"
#include <Jolt/Compute/CPU/WrapShaderBindings.h>
#include "HairGridAccumulateBindings.h"
#include <Jolt/Compute/CPU/WrapShaderEnd.h>

#define JPH_SHADER_NAME HairGridNormalize
#include <Jolt/Compute/CPU/WrapShaderBegin.h>
#include "HairGridNormalize.hlsl"
#include <Jolt/Compute/CPU/WrapShaderBindings.h>
#include "HairGridNormalizeBindings.h"
#include <Jolt/Compute/CPU/WrapShaderEnd.h>

#define JPH_SHADER_NAME HairIntegrate
#include <Jolt/Compute/CPU/WrapShaderBegin.h>
#include "HairIntegrate.hlsl"
#include <Jolt/Compute/CPU/WrapShaderBindings.h>
#include "HairIntegrateBindings.h"
#include <Jolt/Compute/CPU/WrapShaderEnd.h>

#define JPH_SHADER_NAME HairUpdateRoots
#include <Jolt/Compute/CPU/WrapShaderBegin.h>
#include "HairUpdateRoots.hlsl"
#include <Jolt/Compute/CPU/WrapShaderBindings.h>
#include "HairUpdateRootsBindings.h"
#include <Jolt/Compute/CPU/WrapShaderEnd.h>

#define JPH_SHADER_NAME HairUpdateStrands
#include <Jolt/Compute/CPU/WrapShaderBegin.h>
#include "HairUpdateStrands.hlsl"
#include <Jolt/Compute/CPU/WrapShaderBindings.h>
#include "HairUpdateStrandsBindings.h"
#include <Jolt/Compute/CPU/WrapShaderEnd.h>

#define JPH_SHADER_NAME HairUpdateVelocity
#include <Jolt/Compute/CPU/WrapShaderBegin.h>
#include "HairUpdateVelocity.hlsl"
#include <Jolt/Compute/CPU/WrapShaderBindings.h>
#include "HairUpdateVelocityBindings.h"
#include <Jolt/Compute/CPU/WrapShaderEnd.h>

#define JPH_SHADER_NAME HairUpdateVelocityIntegrate
#include <Jolt/Compute/CPU/WrapShaderBegin.h>
#include "HairUpdateVelocityIntegrate.hlsl"
#include <Jolt/Compute/CPU/WrapShaderBindings.h>
#include "HairUpdateVelocityIntegrateBindings.h"
#include <Jolt/Compute/CPU/WrapShaderEnd.h>

#define JPH_SHADER_NAME HairCalculateRenderPositions
#include <Jolt/Compute/CPU/WrapShaderBegin.h>
#include "HairCalculateRenderPositions.hlsl"
#include <Jolt/Compute/CPU/WrapShaderBindings.h>
#include "HairCalculateRenderPositionsBindings.h"
#include <Jolt/Compute/CPU/WrapShaderEnd.h>

JPH_NAMESPACE_BEGIN

void JPH_EXPORT HairRegisterShaders(ComputeSystemCPU *inComputeSystem)
{
	JPH_REGISTER_SHADER(inComputeSystem, HairTeleport);
	JPH_REGISTER_SHADER(inComputeSystem, HairApplyDeltaTransform);
	JPH_REGISTER_SHADER(inComputeSystem, HairSkinVertices);
	JPH_REGISTER_SHADER(inComputeSystem, HairSkinRoots);
	JPH_REGISTER_SHADER(inComputeSystem, HairApplyGlobalPose);
	JPH_REGISTER_SHADER(inComputeSystem, HairCalculateCollisionPlanes);
	JPH_REGISTER_SHADER(inComputeSystem, HairGridClear);
	JPH_REGISTER_SHADER(inComputeSystem, HairGridAccumulate);
	JPH_REGISTER_SHADER(inComputeSystem, HairGridNormalize);
	JPH_REGISTER_SHADER(inComputeSystem, HairIntegrate);
	JPH_REGISTER_SHADER(inComputeSystem, HairUpdateRoots);
	JPH_REGISTER_SHADER(inComputeSystem, HairUpdateStrands);
	JPH_REGISTER_SHADER(inComputeSystem, HairUpdateVelocity);
	JPH_REGISTER_SHADER(inComputeSystem, HairUpdateVelocityIntegrate);
	JPH_REGISTER_SHADER(inComputeSystem, HairCalculateRenderPositions);
}

JPH_NAMESPACE_END

#endif // JPH_USE_CPU_COMPUTE
