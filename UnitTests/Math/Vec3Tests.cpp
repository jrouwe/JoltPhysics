// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"

TEST_SUITE("Vec3Tests")
{
	TEST_CASE("TestVec3ConstructComponents")
	{
		Vec3 v(1, 2, 3);

		// Test component access
		CHECK(v.GetX() == 1);
		CHECK(v.GetY() == 2);
		CHECK(v.GetZ() == 3);

		// Test component access by [] operators
		CHECK(v[0] == 1);
		CHECK(v[1] == 2);
		CHECK(v[2] == 3);

		// Test == and != operators
		CHECK(v == Vec3(1, 2, 3));
		CHECK(v != Vec3(1, 2, 4));

		// Set the components
		v.SetComponent(0, 4);
		v.SetComponent(1, 5);
		v.SetComponent(2, 6);
		CHECK(v == Vec3(4, 5, 6));
	}

	TEST_CASE("TestVec3LoadStoreFloat3")
	{
		float f4[] = { 1, 2, 3, 4 }; // Extra element since we read one too many in sLoadFloat3Unsafe
		Float3 &f3 = *(Float3 *)f4;
		CHECK(Vec3(f3) == Vec3(1, 2, 3));
		CHECK(Vec3::sLoadFloat3Unsafe(f3) == Vec3(1, 2, 3));

		Float3 f3_out;
		Vec3(1, 2, 3).StoreFloat3(&f3_out);
		CHECK(f3 == f3_out);
	}

	TEST_CASE("TestVec3ConstructVec4")
	{
		Vec4 v4(1, 2, 3, 4);
		CHECK(Vec3(v4) == Vec3(1, 2, 3));
	}

	TEST_CASE("TestVec3Zero")
	{
		Vec3 v = Vec3::sZero();

		CHECK(v.GetX() == 0);
		CHECK(v.GetY() == 0);
		CHECK(v.GetZ() == 0);
	}

	TEST_CASE("TestVec3NaN")
	{
		Vec3 v = Vec3::sNaN();

		CHECK(isnan(v.GetX()));
		CHECK(isnan(v.GetY()));
		CHECK(isnan(v.GetZ()));
		CHECK(v.IsNaN());

		v.SetComponent(0, 0);
		CHECK(v.IsNaN());
		v.SetComponent(1, 0);
		CHECK(v.IsNaN());
		v.SetComponent(2, 0);
		CHECK(!v.IsNaN());
	}

	TEST_CASE("TestVec3Replicate")
	{
		CHECK(Vec3::sReplicate(2) == Vec3(2, 2, 2));
	}

	TEST_CASE("TestVec3MinMax")
	{
		Vec3 v1(1, 5, 3);
		Vec3 v2(4, 2, 6);

		CHECK(Vec3::sMin(v1, v2) == Vec3(1, 2, 3));
		CHECK(Vec3::sMax(v1, v2) == Vec3(4, 5, 6));

		CHECK(v1.ReduceMin() == 1);
		CHECK(v1.ReduceMax() == 5);
		CHECK(v2.ReduceMin() == 2);
		CHECK(v2.ReduceMax() == 6);

		CHECK(v1.GetLowestComponentIndex() == 0);
		CHECK(v1.GetHighestComponentIndex() == 1);
		CHECK(v2.GetLowestComponentIndex() == 1);
		CHECK(v2.GetHighestComponentIndex() == 2);
	}

	TEST_CASE("TestVec3Clamp")
	{
		Vec3 v1(1, 2, 3);
		Vec3 v2(4, 5, 6);
		Vec3 v(-1, 3, 7);

		CHECK(Vec3::sClamp(v, v1, v2) == Vec3(1, 3, 6));
	}

	TEST_CASE("TestVec3Comparisons")
	{
		CHECK(Vec3::sEquals(Vec3(1, 2, 3), Vec3(1, 4, 3)) == UVec4(0xffffffffU, 0, 0xffffffffU, 0xffffffffU)); // W is always Z for comparisons
		CHECK(Vec3::sLess(Vec3(1, 2, 4), Vec3(1, 4, 3)) == UVec4(0, 0xffffffffU, 0, 0));
		CHECK(Vec3::sLessOrEqual(Vec3(1, 2, 4), Vec3(1, 4, 3)) == UVec4(0xffffffffU, 0xffffffffU, 0, 0));
		CHECK(Vec3::sGreater(Vec3(1, 2, 4), Vec3(1, 4, 3)) == UVec4(0, 0, 0xffffffffU, 0xffffffffU));
		CHECK(Vec3::sGreaterOrEqual(Vec3(1, 2, 4), Vec3(1, 4, 3)) == UVec4(0xffffffffU, 0, 0xffffffffU, 0xffffffffU));
	}

	TEST_CASE("TestVec3FMA")
	{
		CHECK(Vec3::sFusedMultiplyAdd(Vec3(1, 2, 3), Vec3(4, 5, 6), Vec3(7, 8, 9)) == Vec3(1 * 4 + 7, 2 * 5 + 8, 3 * 6 + 9));
	}

	TEST_CASE("TestVec3Select")
	{
		CHECK(Vec3::sSelect(Vec3(1, 2, 3), Vec3(4, 5, 6), UVec4(0x80000000U, 0, 0x80000000U, 0)) == Vec3(4, 2, 6));
		CHECK(Vec3::sSelect(Vec3(1, 2, 3), Vec3(4, 5, 6), UVec4(0, 0x80000000U, 0, 0x80000000U)) == Vec3(1, 5, 3));
	}

	TEST_CASE("TestVec3BitOps")
	{
		// Test all bit permutations
		Vec3 v1(UVec4(0b0011, 0b00110, 0b001100, 0).ReinterpretAsFloat());
		Vec3 v2(UVec4(0b0101, 0b01010, 0b010100, 0).ReinterpretAsFloat());

		CHECK(Vec3::sOr(v1, v2) == Vec3(UVec4(0b0111, 0b01110, 0b011100, 0).ReinterpretAsFloat()));
		CHECK(Vec3::sXor(v1, v2) == Vec3(UVec4(0b0110, 0b01100, 0b011000, 0).ReinterpretAsFloat()));
		CHECK(Vec3::sAnd(v1, v2) == Vec3(UVec4(0b0001, 0b00010, 0b000100, 0).ReinterpretAsFloat()));
	}

	TEST_CASE("TestVec3Close")
	{
		CHECK(Vec3(1, 2, 3).IsClose(Vec3(1.001f, 2.001f, 3.001f), 1.0e-4f));
		CHECK(!Vec3(1, 2, 3).IsClose(Vec3(1.001f, 2.001f, 3.001f), 1.0e-6f));

		CHECK(Vec3(1.001f, 0, 0).IsNormalized(1.0e-2f));
		CHECK(!Vec3(0, 1.001f, 0).IsNormalized(1.0e-4f));

		CHECK(Vec3(-1.0e-7f, 1.0e-7f, 1.0e-8f).IsNearZero());
		CHECK(!Vec3(-1.0e-7f, 1.0e-7f, -1.0e-5f).IsNearZero());
	}

	TEST_CASE("TestVec3Operators")
	{
		CHECK(-Vec3(1, 2, 3) == Vec3(-1, -2, -3));

		CHECK(Vec3(1, 2, 3) + Vec3(4, 5, 6) == Vec3(5, 7, 9));
		CHECK(Vec3(1, 2, 3) - Vec3(6, 5, 4) == Vec3(-5, -3, -1));

		CHECK(Vec3(1, 2, 3) * Vec3(4, 5, 6) == Vec3(4, 10, 18));
		CHECK(Vec3(1, 2, 3) * 2 == Vec3(2, 4, 6));
		CHECK(4 * Vec3(1, 2, 3) == Vec3(4, 8, 12));

		CHECK(Vec3(1, 2, 3) / 2 == Vec3(0.5f, 1.0f, 1.5f));
		CHECK(Vec3(1, 2, 3) / Vec3(2, 8, 24) == Vec3(0.5f, 0.25f, 0.125f));

		Vec3 v = Vec3(1, 2, 3);
		v *= Vec3(4, 5, 6);
		CHECK(v == Vec3(4, 10, 18));
		v *= 2;
		CHECK(v == Vec3(8, 20, 36));
		v /= 2;
		CHECK(v == Vec3(4, 10, 18));
		v += Vec3(1, 2, 3);
		CHECK(v == Vec3(5, 12, 21));

		CHECK(Vec3(2, 4, 8).Reciprocal() == Vec3(0.5f, 0.25f, 0.125f));
	}

	TEST_CASE("TestVec3Swizzle")	
	{
		Vec3 v(1, 2, 3);

		CHECK(v.SplatX() == Vec4::sReplicate(1));
		CHECK(v.SplatY() == Vec4::sReplicate(2));
		CHECK(v.SplatZ() == Vec4::sReplicate(3));

		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_X, SWIZZLE_X>() == Vec3(1, 1, 1));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_X, SWIZZLE_Y>() == Vec3(1, 1, 2));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_X, SWIZZLE_Z>() == Vec3(1, 1, 3));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Y, SWIZZLE_X>() == Vec3(1, 2, 1));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Y, SWIZZLE_Y>() == Vec3(1, 2, 2));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Y, SWIZZLE_Z>() == Vec3(1, 2, 3));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Z, SWIZZLE_X>() == Vec3(1, 3, 1));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Z, SWIZZLE_Y>() == Vec3(1, 3, 2));
		CHECK(v.Swizzle<SWIZZLE_X, SWIZZLE_Z, SWIZZLE_Z>() == Vec3(1, 3, 3));

		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_X, SWIZZLE_X>() == Vec3(2, 1, 1));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_X, SWIZZLE_Y>() == Vec3(2, 1, 2));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_X, SWIZZLE_Z>() == Vec3(2, 1, 3));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_X>() == Vec3(2, 2, 1));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_Y>() == Vec3(2, 2, 2));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_Z>() == Vec3(2, 2, 3));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_X>() == Vec3(2, 3, 1));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_Y>() == Vec3(2, 3, 2));
		CHECK(v.Swizzle<SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_Z>() == Vec3(2, 3, 3));

		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_X, SWIZZLE_X>() == Vec3(3, 1, 1));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_X, SWIZZLE_Y>() == Vec3(3, 1, 2));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_X, SWIZZLE_Z>() == Vec3(3, 1, 3));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_X>() == Vec3(3, 2, 1));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_Y>() == Vec3(3, 2, 2));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_Z>() == Vec3(3, 2, 3));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_X>() == Vec3(3, 3, 1));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_Y>() == Vec3(3, 3, 2));
		CHECK(v.Swizzle<SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_Z>() == Vec3(3, 3, 3));
	}

	TEST_CASE("TestVec3Abs")
	{
		CHECK(Vec3(1, -2, 3).Abs() == Vec3(1, 2, 3));
		CHECK(Vec3(-1, 2, -3).Abs() == Vec3(1, 2, 3));
	}

	TEST_CASE("TestVec3Dot")
	{
		CHECK(Vec3(1, 2, 3).Dot(Vec3(4, 5, 6)) == float(1 * 4 + 2 * 5 + 3 * 6));
		CHECK(Vec3(1, 2, 3).DotV(Vec3(4, 5, 6)) == Vec3::sReplicate(1 * 4 + 2 * 5 + 3 * 6));
		CHECK(Vec3(1, 2, 3).DotV4(Vec3(4, 5, 6)) == Vec4::sReplicate(1 * 4 + 2 * 5 + 3 * 6));
	}
		
	TEST_CASE("TestVec3Length")
	{
		CHECK(Vec3(1, 2, 3).LengthSq() == float(1 + 4 + 9));
		CHECK(Vec3(1, 2, 3).Length() == sqrt(float(1 + 4 + 9)));
	}

	TEST_CASE("TestVec3Sqrt")
	{
		CHECK_APPROX_EQUAL(Vec3(13, 15, 17).Sqrt(), Vec3(sqrt(13.0f), sqrt(15.0f), sqrt(17.0f)));
	}

	TEST_CASE("TestVec3Cross")
	{
		CHECK(Vec3(1, 0, 0).Cross(Vec3(0, 1, 0)) == Vec3(0, 0, 1));
		CHECK(Vec3(0, 1, 0).Cross(Vec3(1, 0, 0)) == Vec3(0, 0, -1));
		CHECK(Vec3(0, 1, 0).Cross(Vec3(0, 0, 1)) == Vec3(1, 0, 0));
		CHECK(Vec3(0, 0, 1).Cross(Vec3(0, 1, 0)) == Vec3(-1, 0, 0));
		CHECK(Vec3(0, 0, 1).Cross(Vec3(1, 0, 0)) == Vec3(0, 1, 0));
		CHECK(Vec3(1, 0, 0).Cross(Vec3(0, 0, 1)) == Vec3(0, -1, 0));
	}

	TEST_CASE("TestVec3Normalize")
	{
		CHECK(Vec3(3, 2, 1).Normalized() == Vec3(3, 2, 1) / sqrt(9.0f + 4.0f + 1.0f));
		CHECK(Vec3(3, 2, 1).NormalizedOr(Vec3(1, 2, 3)) == Vec3(3, 2, 1) / sqrt(9.0f + 4.0f + 1.0f));
		CHECK(Vec3::sZero().NormalizedOr(Vec3(1, 2, 3)) == Vec3(1, 2, 3));
	}

	TEST_CASE("TestVec3Cast")
	{
		CHECK(Vec3(1, 2, 3).ToInt() == UVec4(1, 2, 3, 3));
		CHECK(Vec3(1, 2, 3).ReinterpretAsInt() == UVec4(0x3f800000U, 0x40000000U, 0x40400000U, 0x40400000U));
	}

	TEST_CASE("TestVec3NormalizedPerpendicular")
	{
		UnitTestRandom random;
		uniform_real_distribution<float> one_to_ten(1.0f, 10.0f);
		for (int i = 0; i < 100; ++i)
		{
			Vec3 v = Vec3::sRandom(random);
			CHECK(v.IsNormalized());
			v *= one_to_ten(random);

			Vec3 p = v.GetNormalizedPerpendicular();

			CHECK(p.IsNormalized());
			CHECK(abs(v.Dot(p)) < 1.0e-6f);
		}
	}

	TEST_CASE("TestVec3Sign")
	{
		CHECK(Vec3(1.2345f, -6.7891f, 0).GetSign() == Vec3(1, -1, 1));
		CHECK(Vec3(0, 2.3456f, -7.8912f).GetSign() == Vec3(1, 1, -1));
	}

#ifdef JPH_FLOATING_POINT_EXCEPTIONS_ENABLED
	TEST_CASE("TestVec3SyncW")
	{
		{
			// Check that W equals Z
			Vec3 v(1, 2, 3);
			CHECK(Vec4(v) == Vec4(1, 2, 3, 3));
		}

		{
			// Check that setting individual components syncs W and Z
			Vec3 v;
			v.SetComponent(2, 3);
			v.SetComponent(1, 2);
			v.SetComponent(0, 1);
			CHECK(v == Vec3(1, 2, 3));
			CHECK(Vec4(v) == Vec4(1, 2, 3, 3));
		}

		{
			// Check that W and Z are still synced after a simple addition
			CHECK(Vec4(Vec3(1, 2, 3) + Vec3(4, 5, 6)) == Vec4(5, 7, 9, 9));
		}

		{
			// Test that casting a Vec4 to Vec3 syncs W and Z
			CHECK(Vec4(Vec3(Vec4(1, 2, 3, 4))) == Vec4(1, 2, 3, 3));
		}

		{
			// Test that loading from Float3 syncs W and Z
			CHECK(Vec4(Vec3(Float3(1, 2, 3))) == Vec4(1, 2, 3, 3));
		}

		{
			// Test that loading unsafe from Float3 syncs W and Z
			Float4 v(1, 2, 3, 4);
			CHECK(Vec4(Vec3::sLoadFloat3Unsafe(*(Float3 *)&v)) == Vec4(1, 2, 3, 3));
		}

		{
			// Test swizzle syncs W and Z
			CHECK(Vec4(Vec3(1, 2, 3).Swizzle<SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_X>()) == Vec4(3, 2, 1, 1));
		}

		{
			// Test cross product syncs W and Z
			CHECK(Vec4(Vec3(1, 0, 0).Cross(Vec3(0, 1, 0))) == Vec4(0, 0, 1, 1));
			CHECK(Vec4(Vec3(0, 1, 0).Cross(Vec3(0, 0, 1))) == Vec4(1, 0, 0, 0));
		}
	}
#endif // JPH_FLOATING_POINT_EXCEPTIONS_ENABLED
}
