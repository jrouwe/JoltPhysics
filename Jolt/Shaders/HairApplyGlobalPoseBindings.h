// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "HairStructs.h"

JPH_SHADER_BIND_BEGIN(JPH_HairApplyGlobalPose)
	JPH_SHADER_BIND_BUFFER(JPH_uint, gVerticesFixed)
	JPH_SHADER_BIND_BUFFER(JPH_uint, gStrandFractions)
	JPH_SHADER_BIND_BUFFER(JPH_float3, gInitialPositions)
	JPH_SHADER_BIND_BUFFER(JPH_uint, gInitialBishops)
	JPH_SHADER_BIND_BUFFER(JPH_uint, gStrandMaterialIndex)
	JPH_SHADER_BIND_BUFFER(JPH_HairMaterial, gMaterials)
	JPH_SHADER_BIND_BUFFER(JPH_HairGlobalPoseTransform, gGlobalPoseTransforms)
	JPH_SHADER_BIND_RW_BUFFER(JPH_HairPosition, gPositions)
JPH_SHADER_BIND_END(JPH_HairApplyGlobalPose)
