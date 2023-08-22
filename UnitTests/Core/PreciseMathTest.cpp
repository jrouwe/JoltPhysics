// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include <atomic>

// Only when actually using FMA and optimizations are enabled
#if defined(NDEBUG) && defined(JPH_USE_FMADD) && (defined(JPH_COMPILER_MSVC) || (defined(JPH_COMPILER_CLANG) && __clang_major__ >= 14))

// Implemented as a global atomic so the compiler can't optimize it to a constant
extern atomic<float> One, OneTenth, Ten;
atomic<float> One = 1.0f, OneTenth = 0.1f /* Actually: 0.100000001f */, Ten = 10.0f;

TEST_SUITE("NormalMathTest")
{
	TEST_CASE("NormalFMA")
	{
		// Should use the FMA instruction and end up not being zero because a * b is not rounded as an intermediate step
		float a = OneTenth, b = Ten, c = One;
		float result = a * b - c;
		CHECK(result != 0.0f);

		// Compiler should not optimize these variables away
		One = OneTenth = Ten = 2.0f;
	}
}

#endif // defined(NDEBUG) && defined(JPH_USE_FMADD) && (defined(JPH_COMPILER_MSVC) || (defined(JPH_COMPILER_CLANG) && __clang_major__ >= 14))

JPH_PRECISE_MATH_ON

extern atomic<float> One2, OneTenth2, Ten2;
atomic<float> One2 = 1.0f, OneTenth2 = 0.1f /* Actually: 0.100000001f */, Ten2 = 10.0f;

TEST_SUITE("PreciseMathTest")
{
	TEST_CASE("PreciseFMA")
	{
		// Should not use the FMA instruction and end up being zero
		float a = OneTenth2, b = Ten2, c = One2;
		float result = a * b - c;
		CHECK(result == 0.0f);

		// Compiler should not optimize these variables away
		One2 = OneTenth2 = Ten2 = 2.0f;
	}
}

JPH_PRECISE_MATH_OFF
