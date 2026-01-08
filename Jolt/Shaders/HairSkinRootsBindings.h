// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "HairStructs.h"

// Overridable input types
#ifndef JPH_SHADER_BIND_SCALP_VERTICES
	#define JPH_SHADER_BIND_SCALP_VERTICES(name)	JPH_SHADER_BIND_BUFFER(JPH_float3, name)
#endif
#ifndef JPH_SHADER_BIND_SCALP_TRIANGLES
	#define JPH_SHADER_BIND_SCALP_TRIANGLES(name)	JPH_SHADER_BIND_BUFFER(JPH_uint, name)
#endif

JPH_SHADER_BIND_BEGIN(JPH_HairSkinRoots)
	JPH_SHADER_BIND_BUFFER(JPH_HairSkinPoint, gSkinPoints)
	JPH_SHADER_BIND_SCALP_VERTICES(gScalpVertices)
	JPH_SHADER_BIND_SCALP_TRIANGLES(gScalpTriangles)
	JPH_SHADER_BIND_BUFFER(JPH_float3, gInitialPositions)
	JPH_SHADER_BIND_BUFFER(JPH_uint, gInitialBishops)
	JPH_SHADER_BIND_RW_BUFFER(JPH_HairPosition, gPositions)
	JPH_SHADER_BIND_RW_BUFFER(JPH_HairGlobalPoseTransform, gGlobalPoseTransforms)
JPH_SHADER_BIND_END(JPH_HairSkinRoots)
