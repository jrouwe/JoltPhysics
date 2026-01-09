// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "HairStructs.h"

JPH_SHADER_BIND_BEGIN(JPH_HairIntegrate)
	JPH_SHADER_BIND_BUFFER(JPH_uint, gVerticesFixed)
	JPH_SHADER_BIND_BUFFER(JPH_uint, gStrandFractions)
	JPH_SHADER_BIND_BUFFER(JPH_float, gNeutralDensity)
	JPH_SHADER_BIND_BUFFER(JPH_float4, gVelocityAndDensity)
	JPH_SHADER_BIND_BUFFER(JPH_uint, gStrandMaterialIndex)
	JPH_SHADER_BIND_BUFFER(JPH_HairMaterial, gMaterials)
	JPH_SHADER_BIND_BUFFER(JPH_HairVelocity, gVelocities)
	JPH_SHADER_BIND_RW_BUFFER(JPH_HairPosition, gPositions)
	JPH_SHADER_BIND_RW_BUFFER(JPH_HairPosition, gPreviousPositions)
JPH_SHADER_BIND_END(JPH_HairIntegrate)
