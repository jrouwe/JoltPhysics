// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "HairStructs.h"

JPH_SHADER_BIND_BEGIN(JPH_HairCalculateCollisionPlanes)
	JPH_SHADER_BIND_BUFFER(JPH_HairPosition, gPositions)
	JPH_SHADER_BIND_BUFFER(JPH_Plane, gShapePlanes)
	JPH_SHADER_BIND_BUFFER(JPH_float3, gShapeVertices)
	JPH_SHADER_BIND_BUFFER(JPH_uint, gShapeIndices)
	JPH_SHADER_BIND_RW_BUFFER(JPH_HairCollisionPlane, gCollisionPlanes)
JPH_SHADER_BIND_END(JPH_HairCalculateCollisionPlanes)
