// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "HairStructs.h"

// Overridable output type
#ifndef JPH_SHADER_BIND_RENDER_POSITIONS
	#define JPH_SHADER_BIND_RENDER_POSITIONS(name)	JPH_SHADER_BIND_RW_BUFFER(JPH_float3, name)
#endif

JPH_SHADER_BIND_BEGIN(JPH_HairCalculateRenderPositions)
	JPH_SHADER_BIND_BUFFER(JPH_HairSVertexInfluence, gSVertexInfluences)
	JPH_SHADER_BIND_BUFFER(JPH_HairPosition, gPositions)
	JPH_SHADER_BIND_RENDER_POSITIONS(gRenderPositions)
JPH_SHADER_BIND_END(JPH_HairCalculateRenderPositions)
