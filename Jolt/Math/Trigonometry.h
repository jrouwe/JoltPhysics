// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

JPH_NAMESPACE_BEGIN

// Note that this file exists because std::sin etc. are not platform independent and will lead to non-deterministic simulation

/// Sine of x (input in radians)
JPH_INLINE float Sin(float inX)
{
	Vec4 s, c;
	Vec4::sReplicate(inX).SinCos(s, c);
	return s.GetX();
}

/// Cosine of x (input in radians)
JPH_INLINE float Cos(float inX)
{
	Vec4 s, c;
	Vec4::sReplicate(inX).SinCos(s, c);
	return c.GetX();
}

/// Tangent of x (input in radians)
JPH_INLINE float Tan(float inX)
{
	return Vec4::sReplicate(inX).Tan().GetX();
}

/// Arc sine of x (returns value in the range [-PI / 2, PI / 2])
/// Note that all input values will be clamped to the range [-1, 1] and this function will not return NaNs like std::asin
JPH_INLINE float ASin(float inX)
{
	return Vec4::sReplicate(inX).ASin().GetX();
}

/// Arc cosine of x (returns value in the range [0, PI])
/// Note that all input values will be clamped to the range [-1, 1] and this function will not return NaNs like std::acos
JPH_INLINE float ACos(float inX)
{
	return Vec4::sReplicate(inX).ACos().GetX();
}

/// Arc tangent of x (returns value in the range [-PI / 2, PI / 2])
JPH_INLINE float ATan(float inX)
{
	return Vec4::sReplicate(inX).ATan().GetX();
}

/// Arc tangent of y / x using the signs of the arguments to determine the correct quadrant (returns value in the range [-PI, PI])
JPH_INLINE float ATan2(float inY, float inX)
{
	return Vec4::sATan2(Vec4::sReplicate(inY), Vec4::sReplicate(inX)).GetX();
}

JPH_NAMESPACE_END
