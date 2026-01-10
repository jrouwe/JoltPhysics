// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "HairGridNormalizeBindings.h"
#include "HairCommon.h"

JPH_SHADER_FUNCTION_BEGIN(void, main, cHairPerGridCellBatch, 1, 1)
	JPH_SHADER_PARAM_THREAD_ID(tid)
JPH_SHADER_FUNCTION_END
{
	uint index = tid.x;
	if (index >= cNumGridPoints)
		return;

	// Convert from fixed point back to float and divide velocity by density to get average velocity
	float4 v = (float4)gVelocityAndDensity[index] * cFixedToFloat;
	float density = v.w;
	if (density > 1.0e-12f)
	{
		v.x /= density;
		v.y /= density;
		v.z /= density;
	}
	gVelocityAndDensity[index] = asint(v);
}
