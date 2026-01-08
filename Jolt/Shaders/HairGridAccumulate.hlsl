// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "HairGridAccumulateBindings.h"
#include "HairCommon.h"

void AtomicAddVelocityAndDensity(uint inIndex, int4 inValue)
{
	JPH_AtomicAdd(gVelocityAndDensity[inIndex].x, inValue.x);
	JPH_AtomicAdd(gVelocityAndDensity[inIndex].y, inValue.y);
	JPH_AtomicAdd(gVelocityAndDensity[inIndex].z, inValue.z);
	JPH_AtomicAdd(gVelocityAndDensity[inIndex].w, inValue.w);
}

JPH_SHADER_FUNCTION_BEGIN(void, main, cHairPerVertexBatch, 1, 1)
	JPH_SHADER_PARAM_THREAD_ID(tid)
JPH_SHADER_FUNCTION_END
{
	// Check that we are processing a valid vertex
	uint vtx = tid.x + cNumStrands; // Skip the root of each strand, it's fixed
	if (vtx >= cNumVertices)
		return;
	if (IsVertexFixed(gVerticesFixed, vtx))
		return;

	// Convert position to grid index and fraction
	uint3 index;
	float3 ma;
	GridPositionToIndexAndFraction(gPositions[vtx].mPosition, index, ma);
	float3 a = float3(1, 1, 1) - ma;

	// Get velocity
	float4 velocity_and_density = float4(gVelocities[vtx].mVelocity, 1) * cFloatToFixed;

	// Calculate contribution of density and velocity for each cell
	uint3 stride = cGridStride;
	uint adr_000 = GridIndexToBufferIndex(index);
	uint adr_100 = adr_000 + 1;
	uint adr_010 = adr_000 + stride.y;
	uint adr_110 = adr_010 + 1;
	AtomicAddVelocityAndDensity(adr_000,            (int4)round( a.x *  a.y *  a.z * velocity_and_density));
	AtomicAddVelocityAndDensity(adr_100,            (int4)round(ma.x *  a.y *  a.z * velocity_and_density));
	AtomicAddVelocityAndDensity(adr_010,            (int4)round( a.x * ma.y *  a.z * velocity_and_density));
	AtomicAddVelocityAndDensity(adr_110,            (int4)round(ma.x * ma.y *  a.z * velocity_and_density));
	AtomicAddVelocityAndDensity(adr_000 + stride.z, (int4)round( a.x *  a.y * ma.z * velocity_and_density));
	AtomicAddVelocityAndDensity(adr_100 + stride.z, (int4)round(ma.x *  a.y * ma.z * velocity_and_density));
	AtomicAddVelocityAndDensity(adr_010 + stride.z, (int4)round( a.x * ma.y * ma.z * velocity_and_density));
	AtomicAddVelocityAndDensity(adr_110 + stride.z, (int4)round(ma.x * ma.y * ma.z * velocity_and_density));
}
