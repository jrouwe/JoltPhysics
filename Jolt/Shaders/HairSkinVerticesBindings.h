// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "HairStructs.h"

JPH_SHADER_BIND_BEGIN(JPH_HairSkinVertices)
	JPH_SHADER_BIND_BUFFER(JPH_float3, gScalpVertices)
	JPH_SHADER_BIND_BUFFER(JPH_HairSkinWeight, gScalpSkinWeights)
	JPH_SHADER_BIND_BUFFER(JPH_Mat44, gScalpJointMatrices)
	JPH_SHADER_BIND_RW_BUFFER(JPH_float3, gScalpVerticesOut)
JPH_SHADER_BIND_END(JPH_HairSkinVertices)
