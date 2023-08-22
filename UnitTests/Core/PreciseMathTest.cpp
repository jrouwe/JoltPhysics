// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include <atomic>

// Implemented as a global atomic so the compiler can't optimize it to a constant
extern atomic<float> One, OneTenth, Ten;
atomic<float> One = 1.0f, OneTenth = 0.1f /* Actually: 0.100000001f */, Ten = 10.0f;

JPH_PRECISE_MATH_ON

TEST_SUITE("PreciseMathTest")
{
	TEST_CASE("CheckNoFMA")
	{
		// Should not use the FMA instruction and end up being zero
		// If FMA is active, a * b will not be rounded to 1.0f so the result will be a small positive number
		float a = OneTenth, b = Ten, c = One;
		float result = a * b - c;
		CHECK(result == 0.0f);

		// Compiler should not optimize these variables away
		One = OneTenth = Ten = 2.0f;
	}
}

JPH_PRECISE_MATH_OFF
