// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include <Jolt/Math/Mat44.h>

TEST_SUITE("Mat44Tests")
{
	TEST_CASE("TestMat44Zero")
	{
		Mat44 zero = Mat44::sZero();

		for (int row = 0; row < 4; ++row)
			for (int col = 0; col < 4; ++col)
				CHECK(zero(row, col) == 0.0f);
	}

	TEST_CASE("TestMat44NaN")
	{
		Mat44 nan = Mat44::sNaN();

		for (int row = 0; row < 4; ++row)
			for (int col = 0; col < 4; ++col)
				CHECK(isnan(nan(row, col)));
	}

	TEST_CASE("TestMat44Identity")
	{
		Mat44 identity = Mat44::sIdentity();

		for (int row = 0; row < 4; ++row)
			for (int col = 0; col < 4; ++col)
				if (row != col)
					CHECK(identity(row, col) == 0.0f);
				else
					CHECK(identity(row, col) == 1.0f);
	}

	TEST_CASE("TestMat44Construct")
	{
		Mat44 mat(Vec4(1, 2, 3, 4), Vec4(5, 6, 7, 8), Vec4(9, 10, 11, 12), Vec4(13, 14, 15, 16));

		CHECK(mat(0, 0) == 1.0f);
		CHECK(mat(1, 0) == 2.0f);
		CHECK(mat(2, 0) == 3.0f);
		CHECK(mat(3, 0) == 4.0f);

		CHECK(mat(0, 1) == 5.0f);
		CHECK(mat(1, 1) == 6.0f);
		CHECK(mat(2, 1) == 7.0f);
		CHECK(mat(3, 1) == 8.0f);

		CHECK(mat(0, 2) == 9.0f);
		CHECK(mat(1, 2) == 10.0f);
		CHECK(mat(2, 2) == 11.0f);
		CHECK(mat(3, 2) == 12.0f);

		CHECK(mat(0, 3) == 13.0f);
		CHECK(mat(1, 3) == 14.0f);
		CHECK(mat(2, 3) == 15.0f);
		CHECK(mat(3, 3) == 16.0f);

		Mat44 mat2(mat);

		CHECK(mat2(0, 0) == 1.0f);
		CHECK(mat2(1, 0) == 2.0f);
		CHECK(mat2(2, 0) == 3.0f);
		CHECK(mat2(3, 0) == 4.0f);

		CHECK(mat2(0, 1) == 5.0f);
		CHECK(mat2(1, 1) == 6.0f);
		CHECK(mat2(2, 1) == 7.0f);
		CHECK(mat2(3, 1) == 8.0f);

		CHECK(mat2(0, 2) == 9.0f);
		CHECK(mat2(1, 2) == 10.0f);
		CHECK(mat2(2, 2) == 11.0f);
		CHECK(mat2(3, 2) == 12.0f);

		CHECK(mat2(0, 3) == 13.0f);
		CHECK(mat2(1, 3) == 14.0f);
		CHECK(mat2(2, 3) == 15.0f);
		CHECK(mat2(3, 3) == 16.0f);

		// Check equal
		CHECK(mat == mat2);
		CHECK(!(mat != mat2));

		// Make unequal
		mat(3, 3) = 1;

		// Check non-equal
		CHECK(!(mat == mat2));
		CHECK(mat != mat2);
	}

	TEST_CASE("TestMat44IsClose")
	{
		Mat44 mat = Mat44::sIdentity();
		Mat44 mat2(mat);

		CHECK(mat.IsClose(mat2, Square(0.1f)));

		mat2(0, 1) = 0.09f;
		CHECK(mat.IsClose(mat2, Square(0.1f)));

		mat2(0, 1) = 0.11f;
		CHECK(!mat.IsClose(mat2, Square(0.1f)));
	}

	TEST_CASE("TestMat44Translation")
	{
		CHECK(Mat44::sTranslation(Vec3(2, 3, 4)) == Mat44(Vec4(1, 0, 0, 0), Vec4(0, 1, 0, 0), Vec4(0, 0, 1, 0), Vec4(2, 3, 4, 1)));
	}

	TEST_CASE("TestMat44Scale")
	{
		CHECK(Mat44::sScale(2) == Mat44(Vec4(2, 0, 0, 0), Vec4(0, 2, 0, 0), Vec4(0, 0, 2, 0), Vec4(0, 0, 0, 1)));
		CHECK(Mat44::sScale(Vec3(2, 3, 4)) == Mat44(Vec4(2, 0, 0, 0), Vec4(0, 3, 0, 0), Vec4(0, 0, 4, 0), Vec4(0, 0, 0, 1)));
	}

	TEST_CASE("TestMat44Rotation")
	{
		Mat44 mat(Vec4(1, 2, 3, 0), Vec4(5, 6, 7, 0), Vec4(9, 10, 11, 0), Vec4(13, 14, 15, 16));
		CHECK(mat.GetRotation() == Mat44(Vec4(1, 2, 3, 0), Vec4(5, 6, 7, 0), Vec4(9, 10, 11, 0), Vec4(0, 0, 0, 1)));
	}

	TEST_CASE("TestMat44SetRotation")
	{
		Mat44 mat(Vec4(1, 2, 3, 4), Vec4(5, 6, 7, 8), Vec4(9, 10, 11, 12), Vec4(13, 14, 15, 16));
		Mat44 mat2(Vec4(17, 18, 19, 20), Vec4(21, 22, 23, 24), Vec4(25, 26, 27, 28), Vec4(29, 30, 31, 32));

		mat.SetRotation(mat2);
		CHECK(mat == Mat44(Vec4(17, 18, 19, 20), Vec4(21, 22, 23, 24), Vec4(25, 26, 27, 28), Vec4(13, 14, 15, 16)));
	}

	TEST_CASE("TestMat44RotationSafe")
	{
		Mat44 mat(Vec4(1, 2, 3, 4), Vec4(5, 6, 7, 8), Vec4(9, 10, 11, 12), Vec4(13, 14, 15, 16));
		CHECK(mat.GetRotationSafe() == Mat44(Vec4(1, 2, 3, 0), Vec4(5, 6, 7, 0), Vec4(9, 10, 11, 0), Vec4(0, 0, 0, 1)));
	}

	TEST_CASE("TestMat44LoadStore")
	{
		Mat44 mat(Vec4(1, 2, 3, 4), Vec4(5, 6, 7, 8), Vec4(9, 10, 11, 12), Vec4(13, 14, 15, 16));

		Float4 storage[4];
		mat.StoreFloat4x4(storage);

		CHECK(storage[0].x == 1.0f);
		CHECK(storage[0].y == 2.0f);
		CHECK(storage[0].z == 3.0f);
		CHECK(storage[0].w == 4.0f);

		CHECK(storage[1].x == 5.0f);
		CHECK(storage[1].y == 6.0f);
		CHECK(storage[1].z == 7.0f);
		CHECK(storage[1].w == 8.0f);

		CHECK(storage[2].x == 9.0f);
		CHECK(storage[2].y == 10.0f);
		CHECK(storage[2].z == 11.0f);
		CHECK(storage[2].w == 12.0f);

		CHECK(storage[3].x == 13.0f);
		CHECK(storage[3].y == 14.0f);
		CHECK(storage[3].z == 15.0f);
		CHECK(storage[3].w == 16.0f);

		Mat44 mat2 = Mat44::sLoadFloat4x4(storage);
		CHECK(mat2 == mat);
	}

	TEST_CASE("TestMat44LoadAligned")
	{
		alignas(16) float values[16] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
		Mat44 mat = Mat44::sLoadFloat4x4Aligned((const Float4 *)values);
		CHECK(mat == Mat44(Vec4(1, 2, 3, 4), Vec4(5, 6, 7, 8), Vec4(9, 10, 11, 12), Vec4(13, 14, 15, 16)));
	}

	TEST_CASE("TestMat44MultiplyMat44")
	{
		Mat44 mat(Vec4(1, 2, 3, 4), Vec4(5, 6, 7, 8), Vec4(9, 10, 11, 12), Vec4(13, 14, 15, 16));
		Mat44 mat2(Vec4(17, 18, 19, 20), Vec4(21, 22, 23, 24), Vec4(25, 26, 27, 28), Vec4(29, 30, 31, 32));

		Mat44 result = mat * mat2;
		CHECK(result == Mat44(Vec4(538, 612, 686, 760), Vec4(650, 740, 830, 920), Vec4(762, 868, 974, 1080), Vec4(874, 996, 1118, 1240)));
	}

	TEST_CASE("TestMat44Add")
	{
		Mat44 mat(Vec4(1, 2, 3, 4), Vec4(5, 6, 7, 8), Vec4(9, 10, 11, 12), Vec4(13, 14, 15, 16));
		Mat44 mat2(Vec4(17, 18, 19, 20), Vec4(21, 22, 23, 24), Vec4(25, 26, 27, 28), Vec4(29, 30, 31, 32));

		Mat44 result = mat + mat2;
		CHECK(result == Mat44(Vec4(18, 20, 22, 24), Vec4(26, 28, 30, 32), Vec4(34, 36, 38, 40), Vec4(42, 44, 46, 48)));

		mat += mat2;
		CHECK(mat == Mat44(Vec4(18, 20, 22, 24), Vec4(26, 28, 30, 32), Vec4(34, 36, 38, 40), Vec4(42, 44, 46, 48)));
	}

	TEST_CASE("TestMat44Sub")
	{
		Mat44 mat(Vec4(1, 2, 3, 4), Vec4(5, 6, 7, 8), Vec4(9, 10, 11, 12), Vec4(13, 14, 15, 16));
		Mat44 mat2(Vec4(32, 31, 30, 29), Vec4(28, 27, 26, 25), Vec4(24, 23, 22, 21), Vec4(20, 19, 18, 17));

		Mat44 result = mat - mat2;
		CHECK(result == Mat44(Vec4(-31, -29, -27, -25), Vec4(-23, -21, -19, -17), Vec4(-15, -13, -11, -9), Vec4(-7, -5, -3, -1)));
	}

	TEST_CASE("TestMat44Negate")
	{
		Mat44 mat(Vec4(1, 2, 3, 4), Vec4(5, 6, 7, 8), Vec4(9, 10, 11, 12), Vec4(13, 14, 15, 16));

		Mat44 result = -mat;
		CHECK(result == Mat44(Vec4(-1, -2, -3, -4), Vec4(-5, -6, -7, -8), Vec4(-9, -10, -11, -12), Vec4(-13, -14, -15, -16)));
	}

	TEST_CASE("TestMat44MultiplyVec3")
	{
		Mat44 mat(Vec4(1, 2, 3, 4), Vec4(5, 6, 7, 8), Vec4(9, 10, 11, 12), Vec4(13, 14, 15, 16));
		Vec3 vec(17, 18, 19);

		Vec3 result = mat * vec;
		CHECK(result == Vec3(291, 346, 401));

		result = mat.Multiply3x3(vec);
		CHECK(result == Vec3(278, 332, 386));

		result = mat.Multiply3x3Transposed(vec);
		CHECK(result == Vec3(110, 326, 542));
	}

	TEST_CASE("TestMat44MultiplyVec4")
	{
		Mat44 mat(Vec4(1, 2, 3, 4), Vec4(5, 6, 7, 8), Vec4(9, 10, 11, 12), Vec4(13, 14, 15, 16));
		Vec4 vec(17, 18, 19, 20);

		Vec4 result = mat * vec;
		CHECK(result == Vec4(538, 612, 686, 760));
	}

	TEST_CASE("TestMat44Scale")
	{
		Mat44 mat(Vec4(1, 2, 3, 4), Vec4(5, 6, 7, 8), Vec4(9, 10, 11, 12), Vec4(13, 14, 15, 16));
		Mat44 result = mat * 2.0f;
		CHECK(result == Mat44(Vec4(2, 4, 6, 8), Vec4(10, 12, 14, 16), Vec4(18, 20, 22, 24), Vec4(26, 28, 30, 32)));
		CHECK(result != mat);
		result *= 0.5f;
		CHECK(result == mat);
	}

	TEST_CASE("TestMat44Transposed")
	{
		Mat44 mat(Vec4(1, 2, 3, 4), Vec4(5, 6, 7, 8), Vec4(9, 10, 11, 12), Vec4(13, 14, 15, 16));
		Mat44 result = mat.Transposed();
		CHECK(result == Mat44(Vec4(1, 5, 9, 13), Vec4(2, 6, 10, 14), Vec4(3, 7, 11, 15), Vec4(4, 8, 12, 16)));
	}

	TEST_CASE("TestMat44Transposed3x3")
	{
		Mat44 mat(Vec4(1, 2, 3, 4), Vec4(5, 6, 7, 8), Vec4(9, 10, 11, 12), Vec4(13, 14, 15, 16));
		Mat44 result = mat.Transposed3x3();
		CHECK(result == Mat44(Vec4(1, 5, 9, 0), Vec4(2, 6, 10, 0), Vec4(3, 7, 11, 0), Vec4(0, 0, 0, 1)));
	}

	TEST_CASE("TestMat44Multiply3x3")
	{
		Mat44 mat1(Vec4(1, 2, 3, 0), Vec4(4, 5, 6, 0), Vec4(7, 8, 9, 0), Vec4(10, 11, 12, 1));
		Mat44 mat2(Vec4(13, 14, 15, 0), Vec4(16, 17, 18, 0), Vec4(19, 20, 21, 0), Vec4(22, 23, 24, 1));
		Mat44 result = mat1.Multiply3x3(mat2);
		CHECK(result.GetColumn4(3) == Vec4(0, 0, 0, 1));
		Mat44 result2 = mat1.GetRotationSafe() * mat2.GetRotationSafe();
		CHECK(result == result2);
	}

	TEST_CASE("TestMat44Multiply3x3LeftTransposed")
	{
		Mat44 mat1(Vec4(1, 2, 3, 4), Vec4(5, 6, 7, 8), Vec4(9, 10, 11, 12), Vec4(13, 14, 15, 16));
		Mat44 mat2(Vec4(17, 18, 19, 20), Vec4(21, 22, 23, 24), Vec4(25, 26, 27, 28), Vec4(29, 30, 31, 32));
		Mat44 result = mat1.Multiply3x3LeftTransposed(mat2);
		CHECK(result.GetColumn4(3) == Vec4(0, 0, 0, 1));
		Mat44 result2 = mat1.GetRotationSafe().Transposed() * mat2.GetRotationSafe();
		CHECK(result == result2);
	}

	TEST_CASE("TestMat44Multiply3x3RightTransposed")
	{
		Mat44 mat1(Vec4(1, 2, 3, 0), Vec4(4, 5, 6, 0), Vec4(7, 8, 9, 0), Vec4(10, 11, 12, 1));
		Mat44 mat2(Vec4(13, 14, 15, 0), Vec4(16, 17, 18, 0), Vec4(19, 20, 21, 0), Vec4(22, 23, 24, 1));
		Mat44 result = mat1.Multiply3x3RightTransposed(mat2);
		CHECK(result.GetColumn4(3) == Vec4(0, 0, 0, 1));
		Mat44 result2 = mat1.GetRotationSafe() * mat2.GetRotationSafe().Transposed();
		CHECK(result == result2);
	}

	TEST_CASE("TestMat44Inversed")
	{
		Mat44 mat(Vec4(0, 2, 0, 8), Vec4(4, 0, 16, 0), Vec4(0, 16, 0, 4), Vec4(8, 0, 2, 0));
		Mat44 inverse = mat.Inversed();
		Mat44 identity = mat * inverse;
		CHECK(identity == Mat44::sIdentity());
	}

	TEST_CASE("TestMat44Inversed3x3")
	{
		Mat44 mat(Vec4(1, 2, 0, 0), Vec4(4, 0, 8, 0), Vec4(0, 16, 0, 0), Vec4(1, 2, 3, 1));
		Mat44 inverse = mat.Inversed3x3();
		CHECK(inverse.GetColumn4(3) == Vec4(0, 0, 0, 1));
		Mat44 identity = mat.Multiply3x3(inverse);
		CHECK(identity == Mat44::sIdentity());
	}

	TEST_CASE("TestMat44SetInversed3x3")
	{
		Mat44 mat(Vec4(1, 2, 0, 0), Vec4(4, 0, 8, 0), Vec4(0, 16, 0, 0), Vec4(1, 2, 3, 1));

		// First test succeeding inverse
		Mat44 inverse;
		CHECK(inverse.SetInversed3x3(mat));
		CHECK(inverse.GetColumn4(3) == Vec4(0, 0, 0, 1));
		Mat44 identity = mat.Multiply3x3(inverse);
		CHECK(identity == Mat44::sIdentity());

		// Now make singular
		mat(0, 0) = 0.0f;
		CHECK(!inverse.SetInversed3x3(mat));
	}

	TEST_CASE("TestMat44GetDeterminant3x3")
	{
		Mat44 mat(Vec4(1, 2, 0, 0), Vec4(4, 0, 8, 0), Vec4(0, 16, 0, 0), Vec4(1, 2, 3, 1));
		CHECK(mat.GetDeterminant3x3() == -128);
	}

	TEST_CASE("TestMat44Adjointed3x3")
	{
		Mat44 mat(Vec4(1, 2, 3, 0), Vec4(5, 6, 7, 0), Vec4(9, 10, 11, 0), Vec4(13, 14, 15, 16));
		Mat44 result = mat.Adjointed3x3();
		CHECK(result == Mat44(Vec4(-4, 8, -4, 0), Vec4(8, -16, 8, 0), Vec4(-4, 8, -4, 0), Vec4(0, 0, 0, 1)));
	}

	TEST_CASE("TestMat44RotationXYZ")
	{
		Mat44 rot = Mat44::sRotationX(0.5f * JPH_PI);
		Vec3 v = rot * Vec3(1, 0, 0);
		CHECK(v == Vec3(1, 0, 0));
		v = rot * Vec3(0, 1, 0);
		CHECK_APPROX_EQUAL(v, Vec3(0, 0, 1));
		v = rot * Vec3(0, 0, 1);
		CHECK_APPROX_EQUAL(v, Vec3(0, -1, 0));

		rot = Mat44::sRotationY(0.5f * JPH_PI);
		v = rot * Vec3(1, 0, 0);
		CHECK_APPROX_EQUAL(v, Vec3(0, 0, -1));
		v = rot * Vec3(0, 1, 0);
		CHECK(v == Vec3(0, 1, 0));
		v = rot * Vec3(0, 0, 1);
		CHECK_APPROX_EQUAL(v, Vec3(1, 0, 0));

		rot = Mat44::sRotationZ(0.5f * JPH_PI);
		v = rot * Vec3(1, 0, 0);
		CHECK_APPROX_EQUAL(v, Vec3(0, 1, 0));
		v = rot * Vec3(0, 1, 0);
		CHECK_APPROX_EQUAL(v, Vec3(-1, 0, 0));
		v = rot * Vec3(0, 0, 1);
		CHECK(v == Vec3(0, 0, 1));
	}

	TEST_CASE("TestMat44RotationAxisAngle")
	{
		Mat44 r1 = Mat44::sRotationX(0.1f * JPH_PI);
		Mat44 r2 = Mat44::sRotation(Vec3(1, 0, 0), 0.1f * JPH_PI);
		CHECK_APPROX_EQUAL(r1, r2);

		r1 = Mat44::sRotationY(0.2f * JPH_PI);
		r2 = Mat44::sRotation(Vec3(0, 1, 0), 0.2f * JPH_PI);
		CHECK_APPROX_EQUAL(r1, r2);

		r1 = Mat44::sRotationZ(0.3f * JPH_PI);
		r2 = Mat44::sRotation(Vec3(0, 0, 1), 0.3f * JPH_PI);
		CHECK_APPROX_EQUAL(r1, r2);
	}

	TEST_CASE("TestMat44CrossProduct")
	{
		Vec3 v1(1, 2, 3);
		Vec3 v2(4, 5, 6);
		Vec3 v3 = v1.Cross(v2);
		Vec3 v4 = Mat44::sCrossProduct(v1) * v2;
		CHECK(v3 == v4);
	}

	TEST_CASE("TestMat44OuterProduct")
	{
		Vec3 v1(1, 2, 3);
		Vec3 v2(4, 5, 6);
		CHECK(Mat44::sOuterProduct(v1, v2) == Mat44(Vec4(1 * 4, 2 * 4, 3 * 4, 0), Vec4(1 * 5, 2 * 5, 3 * 5, 0), Vec4(1 * 6, 2 * 6, 3 * 6, 0), Vec4(0, 0, 0, 1)));
	}

	TEST_CASE("TestMat44QuatLeftMultiply")
	{
		Quat p(2, 3, 4, 1);
		Quat q(6, 7, 8, 5);

		Quat r1 = p * q;
		Quat r2 = Quat(Mat44::sQuatLeftMultiply(p) * q.GetXYZW());
		CHECK(r1 == r2);
	}

	TEST_CASE("TestMat44QuatRightMultiply")
	{
		Quat p(2, 3, 4, 1);
		Quat q(6, 7, 8, 5);

		Quat r1 = q * p;
		Quat r2 = Quat(Mat44::sQuatRightMultiply(p) * q.GetXYZW());
		CHECK(r1 == r2);
	}

	TEST_CASE("TestMat44InverseRotateTranslate")
	{
		Quat rot = Quat::sRotation(Vec3(0, 1, 0), 0.2f * JPH_PI);
		Vec3 pos(2, 3, 4);

		Mat44 m1 = Mat44::sRotationTranslation(rot, pos).Inversed();
		Mat44 m2 = Mat44::sInverseRotationTranslation(rot, pos);

		CHECK_APPROX_EQUAL(m1, m2);
	}

	TEST_CASE("TestMat44InversedRotationTranslation")
	{
		Quat rot = Quat::sRotation(Vec3(0, 1, 0), 0.2f * JPH_PI);
		Vec3 pos(2, 3, 4);

		Mat44 m1 = Mat44::sRotationTranslation(rot, pos).InversedRotationTranslation();
		Mat44 m2 = Mat44::sInverseRotationTranslation(rot, pos);

		CHECK_APPROX_EQUAL(m1, m2);
	}

	TEST_CASE("TestMat44Decompose")
	{
		Mat44 rotation = Mat44::sRotationX(0.1f * JPH_PI) * Mat44::sRotationZ(0.2f * JPH_PI);
		Vec3 scale = Vec3(-1, 2, 3);
		Mat44 mat = rotation * Mat44::sScale(scale);
		CHECK(mat.GetDeterminant3x3() < 0); // Left handed

		Vec3 new_scale;
		Mat44 new_rotation = mat.Decompose(new_scale);
		CHECK(new_rotation.GetDeterminant3x3() > 0); // Right handed

		Mat44 mat2 = new_rotation * Mat44::sScale(new_scale);
		CHECK(mat.IsClose(mat2));
	}

	TEST_CASE("TestMat44PrePostScaled")
	{
		Mat44 m(Vec4(2, 3, 4, 0), Vec4(5, 6, 7, 0), Vec4(8, 9, 10, 0), Vec4(11, 12, 13, 1));
		Vec3 v(14, 15, 16);

		CHECK(m.PreScaled(v) == m * Mat44::sScale(v));
		CHECK(m.PostScaled(v) == Mat44::sScale(v) * m);
	}

	TEST_CASE("TestMat44PrePostTranslated")
	{
		Mat44 m(Vec4(2, 3, 4, 0), Vec4(5, 6, 7, 0), Vec4(8, 9, 10, 0), Vec4(11, 12, 13, 1));
		Vec3 v(14, 15, 16);

		CHECK(m.PreTranslated(v) == m * Mat44::sTranslation(v));
		CHECK(m.PostTranslated(v) == Mat44::sTranslation(v) * m);
	}

	TEST_CASE("TestMat44Decompose")
	{
		// Create a rotation/translation matrix
		Quat rot = Quat::sRotation(Vec3(1, 1, 1).Normalized(), 0.2f * JPH_PI);
		Vec3 pos(2, 3, 4);
		Mat44 rotation_translation = Mat44::sRotationTranslation(rot, pos);

		// Scale the matrix
		Vec3 scale(2, 1, 3);
		Mat44 m1 = rotation_translation * Mat44::sScale(scale);

		// Decompose scale
		Vec3 scale_out;
		Mat44 m2 = m1.Decompose(scale_out);

		// Check individual components
		CHECK_APPROX_EQUAL(rotation_translation, m2);
		CHECK_APPROX_EQUAL(scale, scale_out);
	}

	TEST_CASE("TestMat44DecomposeSkewed")
	{
		// Create a rotation/translation matrix
		Quat rot = Quat::sRotation(Vec3(1, 1, 1).Normalized(), 0.2f * JPH_PI);
		Vec3 pos(2, 3, 4);
		Mat44 rotation_translation = Mat44::sRotationTranslation(rot, pos);

		// Skew the matrix by applying a non-uniform scale
		Mat44 skewed_rotation_translation = Mat44::sScale(Vec3(1.0f, 0.99f, 0.98f)) * rotation_translation;
		float val = skewed_rotation_translation.GetAxisX().Cross(skewed_rotation_translation.GetAxisY()).Dot(skewed_rotation_translation.GetAxisZ());
		CHECK(abs(val - 1.0f) > 0.01f); // Check not matrix is no longer perpendicular

		// Scale the matrix
		Vec3 scale(2, 1, 3);
		Mat44 m1 = skewed_rotation_translation * Mat44::sScale(scale);

		// Decompose scale
		Vec3 scale_out;
		Mat44 m2 = m1.Decompose(scale_out);

		// Check individual components
		CHECK_APPROX_EQUAL(m2.GetAxisX(), skewed_rotation_translation.GetAxisX().Normalized()); // Check X axis didn't change
		CHECK_APPROX_EQUAL(m2.GetAxisY(), skewed_rotation_translation.GetAxisY().Normalized(), 0.003f); // Y axis may move a bit
		CHECK_APPROX_EQUAL(m2.GetAxisZ(), skewed_rotation_translation.GetAxisZ().Normalized(), 0.02f); // Z axis may move a bit
		CHECK_APPROX_EQUAL(m2.GetAxisX().Cross(m2.GetAxisY()).Dot(m2.GetAxisZ()), 1.0f); // Check perpendicular
		CHECK_APPROX_EQUAL(scale, scale_out, 0.05f); // Scale may change a bit
	}
}
