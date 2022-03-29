// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>
#include <Jolt/Math/DVec3.h>
#include <Jolt/Math/Float2.h>

// Disable common warnings
JPH_SUPPRESS_WARNINGS
JPH_CLANG_SUPPRESS_WARNING("-Wheader-hygiene")

#include "doctest.h"

using namespace JPH;

inline void CHECK_APPROX_EQUAL(float inLHS, float inRHS, float inTolerance = 1.0e-6f)
{
	CHECK(abs(inRHS - inLHS) <= inTolerance);
}

inline void CHECK_APPROX_EQUAL(Vec3Arg inLHS, Vec3Arg inRHS, float inTolerance = 1.0e-6f)
{
	CHECK(inLHS.IsClose(inRHS, inTolerance * inTolerance));
}

inline void CHECK_APPROX_EQUAL(Vec4Arg inLHS, Vec4Arg inRHS, float inTolerance = 1.0e-6f)
{
	CHECK(inLHS.IsClose(inRHS, inTolerance * inTolerance));
}

inline void CHECK_APPROX_EQUAL(Mat44Arg inLHS, Mat44Arg inRHS, float inTolerance = 1.0e-6f)
{
	CHECK(inLHS.IsClose(inRHS, inTolerance * inTolerance));
}

inline void CHECK_APPROX_EQUAL(QuatArg inLHS, QuatArg inRHS, float inTolerance = 1.0e-6f)
{
	bool close = inLHS.IsClose(inRHS, inTolerance * inTolerance) || inLHS.IsClose(-inRHS, inTolerance * inTolerance);
	CHECK(close);
}

#ifdef JPH_USE_AVX2

inline void CHECK_APPROX_EQUAL(DVec3Arg inLHS, DVec3Arg inRHS, double inTolerance = 1.0e-6)
{
	CHECK(inLHS.IsClose(inRHS, inTolerance * inTolerance));
}

#endif // JPH_USE_AVX2

inline void CHECK_APPROX_EQUAL(const Float2 &inLHS, const Float2 &inRHS, float inTolerance = 1.0e-6f)
{
	Float2 diff(inLHS.x - inRHS.x, inLHS.y - inRHS.y);
	CHECK(Square(diff.x) + Square(diff.y) < inTolerance * inTolerance);
}

// Define the exact random number generator we want to use across platforms for consistency (default_random_engine's implementation is platform specific)
using UnitTestRandom = mt19937;
