// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "HairGridClearBindings.h"
#include "HairCommon.h"

JPH_SHADER_FUNCTION_BEGIN(void, main, cHairPerGridCellBatch, 1, 1)
	JPH_SHADER_PARAM_THREAD_ID(tid)
JPH_SHADER_FUNCTION_END
{
	uint index = tid.x;
	if (index >= cNumGridPoints)
		return;

	gVelocityAndDensity[index] = int4(0, 0, 0, 0);
}
