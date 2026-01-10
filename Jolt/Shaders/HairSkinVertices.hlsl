// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "HairSkinVerticesBindings.h"
#include "HairCommon.h"

JPH_SHADER_FUNCTION_BEGIN(void, main, cHairPerVertexBatch, 1, 1)
	JPH_SHADER_PARAM_THREAD_ID(tid)
JPH_SHADER_FUNCTION_END
{
	// Check if this is a valid vertex
	uint vtx = tid.x;
		if (vtx >= cNumSkinVertices)
			return;

	// Skin the vertex
	float3 v = float3(0, 0, 0);
	for (uint w = vtx * cNumSkinWeightsPerVertex, w_end = w + cNumSkinWeightsPerVertex; w < w_end; ++w)
	{
		JPH_HairSkinWeight sw = gScalpSkinWeights[w];
		if (sw.mWeight > 0.0f)
			v += sw.mWeight * JPH_Mat44Mul3x4Vec3(gScalpJointMatrices[sw.mJointIdx], gScalpVertices[vtx]);
	}
	gScalpVerticesOut[vtx] = v;
}
