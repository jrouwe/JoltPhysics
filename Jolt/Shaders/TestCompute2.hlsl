// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "TestCompute2Bindings.h"
#include "ShaderMat44.h"
#include "ShaderVec3.h"
#include "ShaderQuat.h"

JPH_SHADER_FUNCTION_BEGIN(void, main, cTestCompute2GroupSize, 1, 1)
	JPH_SHADER_PARAM_THREAD_ID(tid)
JPH_SHADER_FUNCTION_END
{
	TestCompute2Input input = gInput[tid.x];
	TestCompute2Output output;

	output.mMul3x4Output = JPH_Mat44Mul3x4Vec3(input.mMat44Value, input.mMat44MulValue);
	output.mMul3x3Output = JPH_Mat44Mul3x3Vec3(input.mMat44Value, input.mMat44MulValue);

	output.mDecompressedVec3 = JPH_Vec3DecompressUnit(input.mCompressedVec3);

	output.mDecompressedQuat = JPH_QuatDecompress(input.mCompressedQuat);

	gOutput[tid.x] = output;
}
