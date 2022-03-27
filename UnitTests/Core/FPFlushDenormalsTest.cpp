// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include <Jolt/Core/FPFlushDenormals.h>

// Implemented as a global so the compiler can't optimize it to a constant
extern volatile float TestFltMin;
volatile float TestFltMin = FLT_MIN;

TEST_SUITE("FlushDenormalsTests")
{
	TEST_CASE("TestFlushDenormals")
	{
		// By default flush denormals should be off
		{
			float value = TestFltMin * 0.1f;
			CHECK(value > 0.0f);
		}

		// Turn flush denormal on
		{
			FPFlushDenormals flush_denormals;

			float value = TestFltMin * 0.1f;
			CHECK(value == 0.0f);
		}

		// Check if state was properly restored
		{
			float value = TestFltMin * 0.1f;
			CHECK(value > 0.0f);
		}
	}
}
