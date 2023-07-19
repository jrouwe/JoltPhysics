// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include <Jolt/Physics/Collision/Shape/SubShapeID.h>

TEST_SUITE("SubShapeIDTest")
{
	struct SSPair
	{
		uint32		mValue;
		uint		mNumBits;
	};

	using SSPairs = Array<SSPair>;

	// Helper function that pushes sub shape ID's on the creator and checks that they come out again
	static void TestPushPop(const SSPairs &inPairs)
	{
		// Push all id's on the creator
		SubShapeIDCreator creator;
		int total_bits = 0;
		for (const SSPair &p : inPairs)
		{
			creator = creator.PushID(p.mValue, p.mNumBits);
			total_bits += p.mNumBits;
		}
		CHECK(creator.GetNumBitsWritten() == total_bits);

		// Now pop all parts
		SubShapeID id = creator.GetID();
		for (const SSPair &p : inPairs)
		{
			// There should be data (note there is a possibility of a false positive if the bit pattern is all 1's)
			CHECK(!id.IsEmpty());

			// Pop the part
			SubShapeID remainder;
			uint32 value = id.PopID(p.mNumBits, remainder);

			// Check value
			CHECK(value == p.mValue);

			// Continue with the remainder
			id = remainder;
		}

		CHECK(id.IsEmpty());
	}

	TEST_CASE("SubShapeIDTest")
	{
		// Test storing some values
		TestPushPop({ { 0b110101010, 9 }, { 0b0101010101, 10 }, { 0b10110101010, 11 } });

		// Test storing some values with a different pattern
		TestPushPop({ { 0b001010101, 9 }, { 0b1010101010, 10 }, { 0b01001010101, 11 } });

		// Test storing up to 32 bits
		TestPushPop({ { 0b10, 2 }, { 0b1110101010, 10 }, { 0b0101010101, 10 }, { 0b1010101010, 10 } });

		// Test storing up to 32 bits with a different pattern
		TestPushPop({ { 0b0001010101, 10 }, { 0b1010101010, 10 }, { 0b0101010101, 10 }, { 0b01, 2 } });

		// Test storing 0 bits
		TestPushPop({ { 0b10, 2 }, { 0b1110101010, 10 }, { 0, 0 }, { 0b0101010101, 10 }, { 0, 0 }, { 0b1010101010, 10 } });

		// Test 32 bits at once
		TestPushPop({ { 0b10101010101010101010101010101010, 32 } });

		// Test 32 bits at once with a different pattern
		TestPushPop({ { 0b01010101010101010101010101010101, 32 } });
	}
}
