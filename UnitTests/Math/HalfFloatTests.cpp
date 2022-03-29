// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include <Jolt/Math/HalfFloat.h>
#include <Jolt/Core/FPException.h>

TEST_SUITE("HalfFloatTests")
{
	// Helper function to construct a float with a specific bit pattern
	static inline float ReinterpretAsFloat(uint32 inValue)
	{
		static_assert(sizeof(float) == sizeof(uint32));
		union IntToFloat
		{
			uint32	i;
			float	f;
		};
		IntToFloat i_to_f;
		i_to_f.i = inValue;
		return i_to_f.f;
	}

#if defined(JPH_USE_F16C) || defined(JPH_USE_NEON)
	TEST_CASE("TestHalfFloatToFloat")
	{
		// Check all half float values, 4 at a time, skip NaN's and INF
		for (uint32 v = 0; v < 0x7c00; v += 2)
		{
			// Test value, next value and negative variants of both
			UVec4 half_float(v | ((v + 1) << 16), (v | 0x8000) | (((v + 1) | 0x8000) << 16), 0, 0);

			// Compare hardware intrinsic version with fallback version
			Vec4 flt1 = HalfFloatConversion::ToFloat(half_float);
			Vec4 flt2 = HalfFloatConversion::ToFloatFallback(half_float);

			UVec4 flt1_as_int = flt1.ReinterpretAsInt();
			UVec4 flt2_as_int = flt2.ReinterpretAsInt();
			if (flt1_as_int != flt2_as_int)
				CHECK(false); // Not using CHECK(flt1_as_int == flt2_as_int) macros as that makes the test very slow
		}
	}

	// Helper function to compare the intrinsics version with the fallback version
	static inline void CheckFloatToHalfFloat(uint32 inValue, uint32 inSign)
	{
		const float fvalue = ReinterpretAsFloat(inValue + inSign * 0x80000000U);

		HalfFloat hf1 = HalfFloatConversion::FromFloat<HalfFloatConversion::ROUND_TO_NEAREST>(fvalue);
		HalfFloat hf2 = HalfFloatConversion::FromFloatFallback<HalfFloatConversion::ROUND_TO_NEAREST>(fvalue);
		bool result = (hf1 == hf2);
		if (!result)
			CHECK(false); // Not using CHECK(hf1 == hf2) macros as that makes the test very slow

		hf1 = HalfFloatConversion::FromFloat<HalfFloatConversion::ROUND_TO_POS_INF>(fvalue);
		hf2 = HalfFloatConversion::FromFloatFallback<HalfFloatConversion::ROUND_TO_POS_INF>(fvalue);
		result = (hf1 == hf2);
		if (!result)
			CHECK(false);

		hf1 = HalfFloatConversion::FromFloat<HalfFloatConversion::ROUND_TO_NEG_INF>(fvalue);
		hf2 = HalfFloatConversion::FromFloatFallback<HalfFloatConversion::ROUND_TO_NEG_INF>(fvalue);
		result = (hf1 == hf2);
		if (!result)
			CHECK(false);
	}

	TEST_CASE("TestFloatToHalfFloat")
	{
		for (uint32 sign = 0; sign < 2; ++sign)
		{
			// Zero and smallest possible float
			for (uint32 value = 0; value < 2; value++)
				CheckFloatToHalfFloat(value, sign);

			// Floats that are large enough to become a denormalized half float, incrementing by smallest increment that can make a difference
			for (uint32 value = (HalfFloatConversion::FLOAT_EXPONENT_BIAS - HalfFloatConversion::HALF_FLT_EXPONENT_BIAS - HalfFloatConversion::HALF_FLT_MANTISSA_BITS) << HalfFloatConversion::FLOAT_EXPONENT_POS; value < HalfFloatConversion::FLOAT_EXPONENT_MASK << HalfFloatConversion::FLOAT_EXPONENT_POS; value += 1 << (HalfFloatConversion::FLOAT_MANTISSA_BITS - HalfFloatConversion::HALF_FLT_MANTISSA_BITS - 2))
				CheckFloatToHalfFloat(value, sign);

			// INF
			CheckFloatToHalfFloat(0x7f800000U, sign);

			// Nan
			CheckFloatToHalfFloat(0x7fc00000U, sign);
		}
	}
#endif

	TEST_CASE("TestHalfFloatINF")
	{
		// Float -> half float
		CHECK(HalfFloatConversion::FromFloatFallback<HalfFloatConversion::ROUND_TO_NEAREST>(ReinterpretAsFloat(0x7f800000U)) == HALF_FLT_INF);
		CHECK(HalfFloatConversion::FromFloatFallback<HalfFloatConversion::ROUND_TO_NEAREST>(ReinterpretAsFloat(0xff800000U)) == HALF_FLT_INF_NEGATIVE);

		// Half float -> float
		UVec4 half_float(uint32(HALF_FLT_INF) | (uint32(HALF_FLT_INF_NEGATIVE) << 16), 0, 0, 0);
		UVec4 flt = HalfFloatConversion::ToFloatFallback(half_float).ReinterpretAsInt();
		CHECK(flt == UVec4(0x7f800000U, 0xff800000U, 0, 0));
	}

	TEST_CASE("TestHalfFloatNaN")
	{
		// Float -> half float
		CHECK(HalfFloatConversion::FromFloatFallback<HalfFloatConversion::ROUND_TO_NEAREST>(ReinterpretAsFloat(0x7fc00000U)) == HALF_FLT_NANQ);
		CHECK(HalfFloatConversion::FromFloatFallback<HalfFloatConversion::ROUND_TO_NEAREST>(ReinterpretAsFloat(0xffc00000U)) == HALF_FLT_NANQ_NEGATIVE);

		// Half float -> float
		UVec4 half_float(uint32(HALF_FLT_NANQ) | (uint32(HALF_FLT_NANQ_NEGATIVE) << 16), 0, 0, 0);
		UVec4 flt = HalfFloatConversion::ToFloatFallback(half_float).ReinterpretAsInt();
		CHECK(flt == UVec4(0x7fc00000U, 0xffc00000U, 0, 0));
	}
}
