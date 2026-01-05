// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

// Calculate inValue^2
inline float JPH_Square(float inValue)
{
	return inValue * inValue;
}

// Get the closest point on a line segment defined by inA + x * inAB for x e [0, 1] to the origin
inline float3 JPH_GetClosestPointOnLine(float3 inA, float3 inAB)
{
	float v = clamp(-dot(inA, inAB) / dot(inAB, inAB), 0.0f, 1.0f);
	return inA + v * inAB;
}
