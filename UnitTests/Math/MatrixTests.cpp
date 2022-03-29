// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include <Jolt/Math/Matrix.h>

TEST_SUITE("MatrixTests")
{
	TEST_CASE("TestMatrixZero")
	{
		Matrix<3, 5> m = Matrix<3, 5>::sZero();

		for (uint r = 0; r < 3; ++r)
			for (uint c = 0; c < 5; ++c)
				CHECK(m(r, c) == 0.0f);
	}

	TEST_CASE("TestMatrixIdentity")
	{
		Matrix<3, 5> m = Matrix<3, 5>::sIdentity();

		for (uint r = 0; r < 3; ++r)
			for (uint c = 0; c < 5; ++c)
				CHECK(m(r, c) == (r == c? 1.0f : 0.0f));
	}

	TEST_CASE("TestMatrixMultiply")
	{
		Matrix<3, 5> m1 = Matrix<3, 5>::sZero();
		Matrix<5, 4> m2 = Matrix<5, 4>::sZero();

		for (uint r = 0; r < 3; ++r)
			for (uint c = 0; c < 5; ++c)
				m1(r, c) = float(r * 5 + c + 1);

		for (uint r = 0; r < 5; ++r)
			for (uint c = 0; c < 4; ++c)
				m2(r, c) = float(r * 4 + c + 1);

		Matrix<3, 4> m3 = m1 * m2;
			
		CHECK(m3(0, 0) == 175.0f);
		CHECK(m3(1, 0) == 400.0f);
		CHECK(m3(2, 0) == 625.0f);
		CHECK(m3(0, 1) == 190.0f);
		CHECK(m3(1, 1) == 440.0f);
		CHECK(m3(2, 1) == 690.0f);
		CHECK(m3(0, 2) == 205.0f);
		CHECK(m3(1, 2) == 480.0f);
		CHECK(m3(2, 2) == 755.0f);
		CHECK(m3(0, 3) == 220.0f);
		CHECK(m3(1, 3) == 520.0f);
		CHECK(m3(2, 3) == 820.0f);
	}

	TEST_CASE("TestMatrixInversed")
	{
		Matrix<4, 4> mat = Matrix<4, 4>::sZero();
		mat(1, 0) = 4;
		mat(3, 0) = 8;
		mat(0, 1) = 2;
		mat(2, 1) = 16;
		mat(1, 2) = 16;
		mat(3, 2) = 4;
		mat(0, 3) = 8;
		mat(2, 3) = 2;
		Matrix<4, 4> inverse;
		CHECK(inverse.SetInversed(mat));
		Matrix<4, 4> identity = mat * inverse;
		CHECK(identity == Matrix<4, 4>::sIdentity());
	}

	TEST_CASE("TestMatrix22Inversed")
	{
		// SetInverse is specialized for 2x2 matrices
		Matrix<2, 2> mat;
		mat(0, 0) = 1;
		mat(0, 1) = 2;
		mat(1, 0) = 3;
		mat(1, 1) = 4;
		Matrix<2, 2> inverse;
		CHECK(inverse.SetInversed(mat));
		Matrix<2, 2> identity = mat * inverse;
		CHECK(identity == Matrix<2, 2>::sIdentity());
	}
}
