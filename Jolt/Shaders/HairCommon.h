// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "ShaderMath.h"
#include "ShaderMat44.h"
#include "ShaderQuat.h"
#include "ShaderPlane.h"
#include "ShaderVec3.h"

// The density and velocity fields are stored in fixed point while accumulating, this constant converts from float to fixed point.
JPH_SHADER_CONSTANT(int, cFloatToFixed, 1 << 10)
JPH_SHADER_CONSTANT(float, cFixedToFloat, 1.0f / float(cFloatToFixed))

bool IsVertexFixed(JPH_SHADER_BUFFER(JPH_uint) inVertexFixed, uint inVertexIndex)
{
	return (inVertexFixed[inVertexIndex >> 5] & (1u << (inVertexIndex & 31))) != 0;
}

float GetVertexInvMass(JPH_SHADER_BUFFER(JPH_uint) inVertexFixed, uint inVertexIndex)
{
	return IsVertexFixed(inVertexFixed, inVertexIndex)? 0.0f : 1.0f;
}

float GetVertexStrandFraction(JPH_SHADER_BUFFER(JPH_uint) inStrandFractions, uint inVertexIndex)
{
	return ((inStrandFractions[inVertexIndex >> 2] >> ((inVertexIndex & 3) << 3)) & 0xff) * (1.0f / 255.0f);
}

uint GetStrandVertexCount(JPH_SHADER_BUFFER(JPH_uint) inStrandVertexCounts, uint inStrandIndex)
{
	return (inStrandVertexCounts[inStrandIndex >> 2] >> ((inStrandIndex & 3) << 3)) & 0xff;
}

uint GetStrandMaterialIndex(JPH_SHADER_BUFFER(JPH_uint) inStrandMaterialIndex, uint inStrandIndex)
{
	return (inStrandMaterialIndex[inStrandIndex >> 2] >> ((inStrandIndex & 3) << 3)) & 0xff;
}

float GradientSamplerSample(float4 inSampler, float inStrandFraction)
{
	return min(inSampler.w, max(inSampler.z, inSampler.y + inStrandFraction * inSampler.x));
}

void GridPositionToIndexAndFraction(float3 inPosition, JPH_OUT(uint3) outIndex, JPH_OUT(float3) outFraction)
{
	// Get position in grid space
	float3 grid_pos = min(max(inPosition - cGridOffset, float3(0, 0, 0)) * cGridScale, cGridSizeMin1);
	outIndex = min(uint3(grid_pos), cGridSizeMin2);
	outFraction = grid_pos - float3(outIndex);
}

uint GridIndexToBufferIndex(uint3 inIndex)
{
	return inIndex.x + inIndex.y * cGridStride.y + inIndex.z * cGridStride.z;
}
