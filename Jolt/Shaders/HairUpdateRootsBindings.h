// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "HairStructs.h"

JPH_SHADER_BIND_BEGIN(JPH_HairUpdateRoots)
	JPH_SHADER_BIND_BUFFER(JPH_HairPosition, gTargetPositions)
	JPH_SHADER_BIND_BUFFER(JPH_HairGlobalPoseTransform, gTargetGlobalPoseTransforms)
	JPH_SHADER_BIND_RW_BUFFER(JPH_HairPosition, gPositions)
	JPH_SHADER_BIND_RW_BUFFER(JPH_HairGlobalPoseTransform, gGlobalPoseTransforms)
JPH_SHADER_BIND_END(JPH_HairUpdateRoots)
