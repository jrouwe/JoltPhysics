// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

JPH_NAMESPACE_BEGIN

// Note that this file exists because std::sin etc. are not platform independent and will lead to non-deterministic simulation

/// Sine of x
JPH_INLINE float Sin(float inX)
{
	Vec4 s, c;
	Vec4::sReplicate(inX).SinCos(s, c);
	return s.GetX();
}

/// Cosine of x
JPH_INLINE float Cos(float inX)
{
	Vec4 s, c;
	Vec4::sReplicate(inX).SinCos(s, c);
	return c.GetX();
}

/// Tangent of x
JPH_INLINE float Tan(float inX)
{
	return Vec4::sReplicate(inX).Tan().GetX();
}

/// Arc sine of x
JPH_INLINE float ASin(float inX)
{
	return asin(inX);
}

/// Arc cosine of x
JPH_INLINE float ACos(float inX)
{
	return acos(inX);
}

/// Arc tangent of x
JPH_INLINE float ATan(float inX)
{
	return atan(inX);
}

/// Arc tangent of y / x
JPH_INLINE float ATan2(float inY, float inX)
{
	return atan2(inY, inX);
}

JPH_NAMESPACE_END
