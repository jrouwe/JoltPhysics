// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

inline float3 JPH_Vec3DecompressUnit(uint inValue)
{
	const float cOneOverSqrt2 = 0.70710678f;
	const uint cNumBits = 14;
	const uint cMask = (1u << cNumBits) - 1;
	const uint cMaxValue = cMask - 1; // Need odd number of buckets to quantize to or else we can't encode 0
	const float cScale = 2.0f * cOneOverSqrt2 / float(cMaxValue);

	// Restore two components
	float2 v2 = float2(float(inValue & cMask), float((inValue >> cNumBits) & cMask)) * cScale - float2(cOneOverSqrt2, cOneOverSqrt2);

	// Restore the highest component
	float3 v = float3(v2, sqrt(max(1.0f - dot(v2, v2), 0.0f)));

	// Extract sign
	if ((inValue & 0x80000000u) != 0)
		v = -v;

	// Swizzle the components in place
	uint max_element = (inValue >> 29) & 3;
	v = max_element == 0? v.zxy : (max_element == 1? v.xzy : v);

	return v;
}
