// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

JPH_Plane JPH_PlaneFromPointAndNormal(float3 inPoint, float3 inNormal)
{
	return JPH_Plane(inNormal, -dot(inNormal, inPoint));
}

float3 JPH_PlaneGetNormal(JPH_Plane inPlane)
{
	return inPlane.xyz;
}

float JPH_PlaneSignedDistance(JPH_Plane inPlane, float3 inPoint)
{
	return dot(inPoint, inPlane.xyz) + inPlane.w;
}
