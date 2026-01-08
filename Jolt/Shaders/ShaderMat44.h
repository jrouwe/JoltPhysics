// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

inline float3 JPH_Mat44Mul3x4Vec3(JPH_Mat44 inLHS, float3 inRHS)
{
	return inLHS[0].xyz * inRHS.x + inLHS[1].xyz * inRHS.y + inLHS[2].xyz * inRHS.z + inLHS[3].xyz;
}

inline float3 JPH_Mat44Mul3x3Vec3(JPH_Mat44 inLHS, float3 inRHS)
{
	return inLHS[0].xyz * inRHS.x + inLHS[1].xyz * inRHS.y + inLHS[2].xyz * inRHS.z;
}
