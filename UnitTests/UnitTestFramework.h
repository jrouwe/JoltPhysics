// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>
#include <Jolt/Core/Atomics.h>
#include <Jolt/Math/DVec3.h>
#include <Jolt/Math/Float2.h>

// Disable common warnings
JPH_SUPPRESS_WARNINGS
JPH_CLANG_SUPPRESS_WARNING("-Wheader-hygiene")
#ifdef JPH_DOUBLE_PRECISION
JPH_CLANG_SUPPRESS_WARNING("-Wdouble-promotion")
#endif // JPH_DOUBLE_PRECISION

JPH_SUPPRESS_WARNINGS_STD_BEGIN
#include "doctest.h"
JPH_SUPPRESS_WARNINGS_STD_END

using namespace JPH;
using namespace JPH::literals;
using namespace std;

inline void CHECK_APPROX_EQUAL(float inLHS, float inRHS, float inTolerance = 1.0e-6f)
{
	CHECK(abs(inRHS - inLHS) <= inTolerance);
}

inline void CHECK_APPROX_EQUAL(double inLHS, double inRHS, double inTolerance = 1.0e-6)
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

inline void CHECK_APPROX_EQUAL(DMat44Arg inLHS, DMat44Arg inRHS, float inTolerance = 1.0e-6f)
{
	CHECK(inLHS.IsClose(inRHS, inTolerance * inTolerance));
}

inline void CHECK_APPROX_EQUAL(QuatArg inLHS, QuatArg inRHS, float inTolerance = 1.0e-6f)
{
	bool close = inLHS.IsClose(inRHS, inTolerance * inTolerance) || inLHS.IsClose(-inRHS, inTolerance * inTolerance);
	CHECK(close);
}

inline void CHECK_APPROX_EQUAL(DVec3Arg inLHS, DVec3Arg inRHS, double inTolerance = 1.0e-6)
{
	CHECK(inLHS.IsClose(inRHS, inTolerance * inTolerance));
}

inline void CHECK_APPROX_EQUAL(const Float2 &inLHS, const Float2 &inRHS, float inTolerance = 1.0e-6f)
{
	Float2 diff(inLHS.x - inRHS.x, inLHS.y - inRHS.y);
	CHECK(Square(diff.x) + Square(diff.y) < inTolerance * inTolerance);
}

// Define the exact random number generator we want to use across platforms for consistency (default_random_engine's implementation is platform specific)
using UnitTestRandom = mt19937;

#ifdef JPH_ENABLE_ASSERTS

// Stack based object that tests for an assert
class ExpectAssert
{
public:
	/// Expect inCount asserts
	explicit					ExpectAssert(int inCount)
	{
		CHECK(sCount == 0);
		sCount = inCount;

		mPrevAssertFailed = AssertFailed;
		AssertFailed = [](const char*, const char*, const char*, uint)
		{
			--sCount;
			return false;
		};
	}

	/// Verifies that the expected number of asserts were triggered
								~ExpectAssert()
	{
		AssertFailed = mPrevAssertFailed;
		CHECK(sCount == 0);
	}

private:
	// Keeps track of number of asserts that are expected
	inline static atomic<int>	sCount { 0 };

	// Previous assert function
	AssertFailedFunction		mPrevAssertFailed;
};

#endif // JPH_ENABLE_ASSERTS
