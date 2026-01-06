// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "TestComputeBindings.h"

JPH_SHADER_FUNCTION_BEGIN(void, main, cTestComputeGroupSize, 1, 1)
	JPH_SHADER_PARAM_THREAD_ID(tid)
JPH_SHADER_FUNCTION_END
{
	// Ensure that we do not write out of bounds
	if (tid.x >= cNumElements)
		return;

	if (cUIntValue2 == 0)
	{
		// First write, uses optional data and tests that the packing of float3/uint3's works
		gData[tid.x] = gOptionalData[tid.x] + int(cFloat3Value2.y) + gUploadData[0];
	}
	else
	{
		// Read-modify-write gData
		gData[tid.x] = (gData[tid.x] + cUIntValue) * cUIntValue2;
	}
}
