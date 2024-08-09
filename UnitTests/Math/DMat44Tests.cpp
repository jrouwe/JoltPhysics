// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2022 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include <Jolt/Math/DMat44.h>

TEST_SUITE("DMat44Tests")
{
	TEST_CASE("TestDMat44Zero")
	{
		DMat44 zero = DMat44::sZero();

		CHECK(zero.GetAxisX() == Vec3::sZero());
		CHECK(zero.GetAxisY() == Vec3::sZero());
		CHECK(zero.GetAxisZ() == Vec3::sZero());
		CHECK(zero.GetTranslation() == DVec3::sZero());
	}

	TEST_CASE("TestDMat44Identity")
	{
		DMat44 identity = DMat44::sIdentity();

		CHECK(identity == DMat44(Vec4(1, 0, 0, 0), Vec4(0, 1, 0, 0), Vec4(0, 0, 1, 0), DVec3(0, 0, 0)));
	}

	TEST_CASE("TestDMat44Construct")
	{
		DMat44 mat(Vec4(1, 2, 3, 4), Vec4(5, 6, 7, 8), Vec4(9, 10, 11, 12), DVec3(13, 14, 15));

		CHECK(mat.GetColumn4(0) == Vec4(1, 2, 3, 4));
		CHECK(mat.GetColumn4(1) == Vec4(5, 6, 7, 8));
		CHECK(mat.GetColumn4(2) == Vec4(9, 10, 11, 12));
		CHECK(mat.GetTranslation() == DVec3(13, 14, 15));

		DMat44 mat2(mat);

		CHECK(mat2.GetColumn4(0) == Vec4(1, 2, 3, 4));
		CHECK(mat2.GetColumn4(1) == Vec4(5, 6, 7, 8));
		CHECK(mat2.GetColumn4(2) == Vec4(9, 10, 11, 12));
		CHECK(mat2.GetTranslation() == DVec3(13, 14, 15));
	}

	TEST_CASE("TestDMat44Scale")
	{
		CHECK(DMat44::sScale(Vec3(2, 3, 4)) == DMat44(Vec4(2, 0, 0, 0), Vec4(0, 3, 0, 0), Vec4(0, 0, 4, 0), DVec3(0, 0, 0)));
	}

	TEST_CASE("TestDMat44Rotation")
	{
		DMat44 mat(Vec4(1, 2, 3, 4), Vec4(5, 6, 7, 8), Vec4(9, 10, 11, 12), DVec3(13, 14, 15));
		CHECK(mat.GetRotation() == Mat44(Vec4(1, 2, 3, 4), Vec4(5, 6, 7, 8), Vec4(9, 10, 11, 12), Vec4(0, 0, 0, 1)));
	}

	TEST_CASE("TestMat44SetRotation")
	{
		DMat44 mat(Vec4(1, 2, 3, 4), Vec4(5, 6, 7, 8), Vec4(9, 10, 11, 12), DVec3(13, 14, 15));
		Mat44 mat2(Vec4(17, 18, 19, 20), Vec4(21, 22, 23, 24), Vec4(25, 26, 27, 28), Vec4(29, 30, 31, 32));

		mat.SetRotation(mat2);
		CHECK(mat == DMat44(Vec4(17, 18, 19, 20), Vec4(21, 22, 23, 24), Vec4(25, 26, 27, 28), DVec3(13, 14, 15)));
	}

	TEST_CASE("TestDMat44MultiplyMat44")
	{
		DMat44 mat(Vec4(1, 2, 3, 0), Vec4(5, 6, 7, 0), Vec4(9, 10, 11, 0), DVec3(13, 14, 15));
		Mat44 mat2(Vec4(17, 18, 19, 0), Vec4(21, 22, 23, 0), Vec4(25, 26, 27, 0), Vec4(29, 30, 31, 1));

		DMat44 result = mat * mat2;
		CHECK(result == DMat44(Vec4(278, 332, 386, 0), Vec4(338, 404, 470, 0), Vec4(398, 476, 554, 0), DVec3(471, 562, 653)));
	}

	TEST_CASE("TestDMat44MultiplyDMat44")
	{
		DMat44 mat(Vec4(1, 2, 3, 0), Vec4(5, 6, 7, 0), Vec4(9, 10, 11, 0), DVec3(13, 14, 15));
		DMat44 mat2(Vec4(17, 18, 19, 0), Vec4(21, 22, 23, 0), Vec4(25, 26, 27, 0), DVec3(29, 30, 31));

		DMat44 result = mat * mat2;
		CHECK(result == DMat44(Vec4(278, 332, 386, 0), Vec4(338, 404, 470, 0), Vec4(398, 476, 554, 0), DVec3(471, 562, 653)));
	}

	TEST_CASE("TestDMat44MultiplyVec3")
	{
		DMat44 mat(Vec4(1, 2, 3, 4), Vec4(5, 6, 7, 8), Vec4(9, 10, 11, 12), DVec3(13, 14, 15));
		Vec3 vec(17, 18, 19);

		DVec3 result = mat * DVec3(vec);
		CHECK(result == DVec3(291, 346, 401));

		DVec3 result2 = mat * vec;
		CHECK(result2 == DVec3(291, 346, 401));

		Vec3 result3 = mat.Multiply3x3(vec);
		CHECK(result3 == Vec3(278, 332, 386));

		Vec3 result4 = mat.Multiply3x3Transposed(vec);
		CHECK(result4 == Vec3(110, 326, 542));
	}

	TEST_CASE("TestDMat44Inversed")
	{
		DMat44 mat(Vec4(1, 16, 2, 0), Vec4(2, 8, 4, 0), Vec4(8, 4, 1, 0), DVec3(4, 2, 8));
		DMat44 inverse = mat.Inversed();
		DMat44 identity = mat * inverse;
		CHECK_APPROX_EQUAL(identity, DMat44::sIdentity());
	}

	TEST_CASE("TestDMat44InverseRotateTranslate")
	{
		Quat rot = Quat::sRotation(Vec3(0, 1, 0), 0.2f * JPH_PI);
		DVec3 pos(2, 3, 4);

		DMat44 m1 = DMat44::sRotationTranslation(rot, pos).Inversed();
		DMat44 m2 = DMat44::sInverseRotationTranslation(rot, pos);

		CHECK_APPROX_EQUAL(m1, m2);
	}

	TEST_CASE("TestDMat44InversedRotationTranslation")
	{
		Quat rot = Quat::sRotation(Vec3(0, 1, 0), 0.2f * JPH_PI);
		DVec3 pos(2, 3, 4);

		DMat44 m1 = DMat44::sRotationTranslation(rot, pos).InversedRotationTranslation();
		DMat44 m2 = DMat44::sInverseRotationTranslation(rot, pos);

		CHECK_APPROX_EQUAL(m1, m2);
	}

	TEST_CASE("TestDMat44PrePostScaled")
	{
		DMat44 m(Vec4(2, 3, 4, 0), Vec4(5, 6, 7, 0), Vec4(8, 9, 10, 0), DVec3(11, 12, 13));
		Vec3 v(14, 15, 16);

		CHECK(m.PreScaled(v) == m * DMat44::sScale(v));
		CHECK(m.PostScaled(v) == DMat44::sScale(v) * m);
	}

	TEST_CASE("TestDMat44PrePostTranslated")
	{
		DMat44 m(Vec4(2, 3, 4, 0), Vec4(5, 6, 7, 0), Vec4(8, 9, 10, 0), DVec3(11, 12, 13));
		Vec3 v(14, 15, 16);

		CHECK_APPROX_EQUAL(m.PreTranslated(v), m * DMat44::sTranslation(DVec3(v)));
		CHECK_APPROX_EQUAL(m.PostTranslated(v), DMat44::sTranslation(DVec3(v)) * m);
	}

	TEST_CASE("TestDMat44Decompose")
	{
		// Create a rotation/translation matrix
		Quat rot = Quat::sRotation(Vec3(1, 1, 1).Normalized(), 0.2f * JPH_PI);
		DVec3 pos(2, 3, 4);
		DMat44 rotation_translation = DMat44::sRotationTranslation(rot, pos);

		// Scale the matrix
		Vec3 scale(2, 1, 3);
		DMat44 m1 = rotation_translation * DMat44::sScale(scale);

		// Decompose scale
		Vec3 scale_out;
		DMat44 m2 = m1.Decompose(scale_out);

		// Check individual components
		CHECK_APPROX_EQUAL(rotation_translation, m2);
		CHECK_APPROX_EQUAL(scale, scale_out);
	}
}
