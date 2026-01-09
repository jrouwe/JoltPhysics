// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

float3 SkinRenderVertex(uint inVertexIndex)
{
	// Calculating resulting render position
	float3 out_position = float3(0, 0, 0);
	for (uint idx = inVertexIndex * cHairNumSVertexInfluences, idx_end = idx + cHairNumSVertexInfluences; idx < idx_end; ++idx)
	{
		JPH_HairSVertexInfluence inf = gSVertexInfluences[idx];
		JPH_HairPosition sim_vtx = gPositions[inf.mVertexIndex];
		out_position += inf.mWeight * (sim_vtx.mPosition + JPH_QuatMulVec3(sim_vtx.mRotation, inf.mRelativePosition));
	}
	return out_position;
}
