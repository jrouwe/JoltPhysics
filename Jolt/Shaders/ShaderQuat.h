// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

inline float3 JPH_QuatMulVec3(JPH_Quat inLHS, float3 inRHS)
{
	float3 v_xyz = inLHS.xyz;
	float3 v_yzx = inLHS.yzx;
	float3 q_cross_p = (inRHS.yzx * v_xyz - v_yzx * inRHS).yzx;
	float3 q_cross_q_cross_p = (q_cross_p.yzx * v_xyz - v_yzx * q_cross_p).yzx;
	float3 v = inLHS.w * q_cross_p + q_cross_q_cross_p;
	return inRHS + (v + v);
}

inline JPH_Quat JPH_QuatMulQuat(JPH_Quat inLHS, JPH_Quat inRHS)
{
	float x = inLHS.w * inRHS.x + inLHS.x * inRHS.w + inLHS.y * inRHS.z - inLHS.z * inRHS.y;
	float y = inLHS.w * inRHS.y - inLHS.x * inRHS.z + inLHS.y * inRHS.w + inLHS.z * inRHS.x;
	float z = inLHS.w * inRHS.z + inLHS.x * inRHS.y - inLHS.y * inRHS.x + inLHS.z * inRHS.w;
	float w = inLHS.w * inRHS.w - inLHS.x * inRHS.x - inLHS.y * inRHS.y - inLHS.z * inRHS.z;
	return JPH_Quat(x, y, z, w);
}

inline JPH_Quat JPH_QuatImaginaryMulQuat(float3 inLHS, JPH_Quat inRHS)
{
	float x = +inLHS.x * inRHS.w + inLHS.y * inRHS.z - inLHS.z * inRHS.y;
	float y = -inLHS.x * inRHS.z + inLHS.y * inRHS.w + inLHS.z * inRHS.x;
	float z = +inLHS.x * inRHS.y - inLHS.y * inRHS.x + inLHS.z * inRHS.w;
	float w = -inLHS.x * inRHS.x - inLHS.y * inRHS.y - inLHS.z * inRHS.z;
	return JPH_Quat(x, y, z, w);
}

inline float3 JPH_QuatRotateAxisZ(JPH_Quat inRotation)
{
	return (inRotation.z + inRotation.z) * inRotation.xyz + (inRotation.w + inRotation.w) * float3(inRotation.y, -inRotation.x, inRotation.w) - float3(0, 0, 1);
}

inline JPH_Quat JPH_QuatConjugate(JPH_Quat inRotation)
{
	return JPH_Quat(-inRotation.x, -inRotation.y, -inRotation.z, inRotation.w);
}

inline JPH_Quat JPH_QuatDecompress(uint inValue)
{
	const float cOneOverSqrt2 = 0.70710678f;
	const uint cNumBits = 9;
	const uint cMask = (1u << cNumBits) - 1;
	const uint cMaxValue = cMask - 1; // Need odd number of buckets to quantize to or else we can't encode 0
	const float cScale = 2.0f * cOneOverSqrt2 / float(cMaxValue);

	// Restore two components
	float3 v3 = float3(float(inValue & cMask), float((inValue >> cNumBits) & cMask), float((inValue >> (2 * cNumBits)) & cMask)) * cScale - float3(cOneOverSqrt2, cOneOverSqrt2, cOneOverSqrt2);

	// Restore the highest component
	float4 v = float4(v3, sqrt(max(1.0f - dot(v3, v3), 0.0f)));

	// Extract sign
	if ((inValue & 0x80000000u) != 0)
		v = -v;

	// Swizzle the components in place
	uint max_element = (inValue >> 29) & 3;
	v = max_element == 0? v.wxyz : (max_element == 1? v.xwyz : (max_element == 2? v.xywz : v));

	return v;
}

inline JPH_Quat JPH_QuatFromMat33(float3 inCol0, float3 inCol1, float3 inCol2)
{
	float tr = inCol0.x + inCol1.y + inCol2.z;
	if (tr >= 0.0f)
	{
		float s = sqrt(tr + 1.0f);
		float is = 0.5f / s;
		return JPH_Quat(
			(inCol1.z - inCol2.y) * is,
			(inCol2.x - inCol0.z) * is,
			(inCol0.y - inCol1.x) * is,
			0.5f * s);
	}
	else
	{
		if (inCol0.x > inCol1.y && inCol0.x > inCol2.z)
		{
			float s = sqrt(inCol0.x - (inCol1.y + inCol2.z) + 1);
			float is = 0.5f / s;
			return JPH_Quat(
				0.5f * s,
				(inCol1.x + inCol0.y) * is,
				(inCol0.z + inCol2.x) * is,
				(inCol1.z - inCol2.y) * is);
		}
		else if (inCol1.y > inCol2.z)
		{
			float s = sqrt(inCol1.y - (inCol2.z + inCol0.x) + 1);
			float is = 0.5f / s;
			return JPH_Quat(
				(inCol1.x + inCol0.y) * is,
				0.5f * s,
				(inCol2.y + inCol1.z) * is,
				(inCol2.x - inCol0.z) * is);
		}
		else
		{
			float s = sqrt(inCol2.z - (inCol0.x + inCol1.y) + 1);
			float is = 0.5f / s;
			return JPH_Quat(
				(inCol0.z + inCol2.x) * is,
				(inCol2.y + inCol1.z) * is,
				0.5f * s,
				(inCol0.y - inCol1.x) * is);
		}
	}
}
