// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "ShaderCore.h"

JPH_SHADER_CONSTANT(int, cTestCompute2GroupSize, 1)

JPH_SHADER_STRUCT_BEGIN(TestCompute2Input)
	JPH_SHADER_STRUCT_MEMBER(JPH_Mat44,			Mat44Value)
	JPH_SHADER_STRUCT_MEMBER(JPH_float3,		Mat44MulValue)
	JPH_SHADER_STRUCT_MEMBER(JPH_uint,			CompressedVec3)
	JPH_SHADER_STRUCT_MEMBER(JPH_uint,			CompressedQuat)
JPH_SHADER_STRUCT_END(TestComputeContext)

JPH_SHADER_STRUCT_BEGIN(TestCompute2Output)
	JPH_SHADER_STRUCT_MEMBER(JPH_float3,		Mul3x4Output)
	JPH_SHADER_STRUCT_MEMBER(JPH_float3,		Mul3x3Output)
	JPH_SHADER_STRUCT_MEMBER(JPH_float3,		DecompressedVec3)
	JPH_SHADER_STRUCT_MEMBER(JPH_Quat,			DecompressedQuat)
JPH_SHADER_STRUCT_END(TestCompute2Output)

JPH_SHADER_BIND_BEGIN(JPH_TestCompute2)
	JPH_SHADER_BIND_BUFFER(TestCompute2Input, gInput)
	JPH_SHADER_BIND_RW_BUFFER(TestCompute2Output, gOutput)
JPH_SHADER_BIND_END(JPH_TestCompute2)
