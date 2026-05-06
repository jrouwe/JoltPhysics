// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include <Jolt/Math/Mat44.h>
#include <Jolt/Math/FindRoot.h>

TEST_SUITE("Mat44Tests")
{
	TEST_CASE("TestCountTrailingZeros")
	{
		CHECK(CountTrailingZeros(0) == 32);
		for (int i = 0; i < 32; ++i)
			CHECK(CountTrailingZeros(1U << i) == i);
	}

	TEST_CASE("TestCountLeadingZeros")
	{
		CHECK(CountLeadingZeros(0) == 32);
		for (int i = 0; i < 32; ++i)
			CHECK(CountLeadingZeros(1U << i) == 31 - i);
	}

	TEST_CASE("TestCountBits")
	{
		CHECK(CountBits(0) == 0);
		CHECK(CountBits(0b10000000000000000000000000000000) == 1);
		CHECK(CountBits(0b00000000000000000000000000000001) == 1);
		CHECK(CountBits(0b10000000000000001000000000000000) == 2);
		CHECK(CountBits(0b00000000000000010000000000000001) == 2);
		CHECK(CountBits(0b10000000100000001000000010000000) == 4);
		CHECK(CountBits(0b00000001000000010000000100000001) == 4);
		CHECK(CountBits(0b10001000100010001000100010001000) == 8);
		CHECK(CountBits(0b00010001000100010001000100010001) == 8);
		CHECK(CountBits(0b10101010101010101010101010101010) == 16);
		CHECK(CountBits(0b01010101010101010101010101010101) == 16);
		CHECK(CountBits(0b11111111111111111111111111111111) == 32);
	}

	TEST_CASE("TestNextPowerOf2")
	{
		CHECK(GetNextPowerOf2(0) == 1);

		for (int i = 0; i < 31; ++i)
		{
			uint32 pow = uint32(1) << i;
			if (pow > 2)
				CHECK(GetNextPowerOf2(pow - 1) == pow);
			CHECK(GetNextPowerOf2(pow) == pow);
			CHECK(GetNextPowerOf2(pow + 1) == pow << 1);
		}

		CHECK(GetNextPowerOf2(0x8000000U - 1) == 0x8000000U);
		CHECK(GetNextPowerOf2(0x8000000U) == 0x8000000U);
	}

	TEST_CASE("TestCenterAngleAroundZero")
	{
		for (int i = 0; i < 10; i += 2)
		{
			CHECK_APPROX_EQUAL(CenterAngleAroundZero(i * JPH_PI), 0, 1.0e-5f);
			CHECK_APPROX_EQUAL(CenterAngleAroundZero((0.5f + i) * JPH_PI), 0.5f * JPH_PI, 1.0e-5f);
			CHECK_APPROX_EQUAL(CenterAngleAroundZero((1.5f + i) * JPH_PI), -0.5f * JPH_PI, 1.0e-5f);
			CHECK_APPROX_EQUAL(CenterAngleAroundZero(-(0.5f + i) * JPH_PI), -0.5f * JPH_PI, 1.0e-5f);
			CHECK_APPROX_EQUAL(CenterAngleAroundZero(-(1.5f + i) * JPH_PI), 0.5f * JPH_PI, 1.0e-5f);
			CHECK_APPROX_EQUAL(CenterAngleAroundZero(-(0.99f + i) * JPH_PI), -0.99f * JPH_PI, 1.0e-5f);
			CHECK_APPROX_EQUAL(CenterAngleAroundZero((0.99f + i) * JPH_PI), 0.99f * JPH_PI, 1.0e-5f);
		}
	}

	TEST_CASE("TestIsPowerOf2")
	{
		for (int i = 0; i < 63; ++i)
			CHECK(IsPowerOf2(uint64(1) << 1));
		CHECK(!IsPowerOf2(-2));
		CHECK(!IsPowerOf2(0));
		CHECK(!IsPowerOf2(3));
		CHECK(!IsPowerOf2(5));
		CHECK(!IsPowerOf2(15));
		CHECK(!IsPowerOf2(17));
		CHECK(!IsPowerOf2(65535));
		CHECK(!IsPowerOf2(65537));
	}

	TEST_CASE("TestFindRoot")
	{
		{
			// Quadratic, 2 solutions
			float x1, x2;
			CHECK(FindRoot(2.0f, 8.0f, 6.0f, x1, x2) == 2);
			CHECK(x1 == -3.0f);
			CHECK(x2 == -1.0f);
		}

		{
			// Quadratic without b term, 2 solutions
			float x1, x2;
			CHECK(FindRoot(2.0f, 0.0f, -8.0f, x1, x2) == 2);
			CHECK(x1 == -2.0f);
			CHECK(x2 == 2.0f);
		}

		{
			// Quadratic without solution
			float x1, x2;
			CHECK(FindRoot(2.0f, 1.0f, 8.0f, x1, x2) == 0);
		}

		{
			// Linear
			float x1, x2;
			CHECK(FindRoot(0.0f, 8.0f, 4.0f, x1, x2) == 1);
			CHECK(x1 == -0.5f);
			CHECK(x2 == -0.5f);
		}

		{
			// Constant
			float x1, x2;
			CHECK(FindRoot(0.0f, 0.0f, 0.0f, x1, x2) == 1);
			CHECK(x1 == 0.0f);
			CHECK(x2 == 0.0f);
		}

		{
			// Constant, no solution
			float x1, x2;
			CHECK(FindRoot(0.0f, 0.0f, 1.0f, x1, x2) == 0);
		}
	}

	JPH_SUPPRESS_WARNING_PUSH
	JPH_MSVC_SUPPRESS_WARNING(4746) // volatile access of 'X' is subject to / volatile : <iso | ms> setting; consider using __iso_volatile_load / store intrinsic functions

	TEST_CASE("TestDifferenceOfProducts")
	{
		// Mark 'volatile' to try to prevent the compiler from doing the calculation at compile time
		volatile float a = 33962.035f, b = -30438.8f, c = 41563.4f, d = -24871.969f;
		float result = DifferenceOfProducts(a, b, c, d);
		double expected = double(a) * double(b) - double(c) * double(d);
		CHECK(expected == -75.165603637695312);
	#ifdef JPH_USE_FMADD
		CHECK(result == float(expected));
	#else
		CHECK(result == -128.0f); // The products are in the order of 10^9, so the subtraction causes a large loss of precision and we get a very different result. This is expected when fused multiply add instructions are not available.
	#endif
	}

	JPH_SUPPRESS_WARNING_POP
}
