// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#ifdef __cplusplus
	#pragma once
#endif

#include "ShaderCore.h"

static const int cTestComputeGroupSize = 64;

JPH_SHADER_CONSTANTS_BEGIN(TestComputeContext, gContext)
	JPH_SHADER_CONSTANTS_MEMBER(JPH_float3,		Float3Value)
	JPH_SHADER_CONSTANTS_MEMBER(JPH_uint,		UIntValue)		// Test that this value packs correctly with the float3 preceding it
	JPH_SHADER_CONSTANTS_MEMBER(JPH_float3,		Float3Value2)
	JPH_SHADER_CONSTANTS_MEMBER(JPH_uint,		UIntValue2)
	JPH_SHADER_CONSTANTS_MEMBER(JPH_uint,		NumElements)
JPH_SHADER_CONSTANTS_END
