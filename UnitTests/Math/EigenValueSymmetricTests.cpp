// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include <Jolt/Math/EigenValueSymmetric.h>
#include <Jolt/Math/Matrix.h>

TEST_SUITE("EigenValueSymmetricTests")
{
	TEST_CASE("TestEigenValueSymmetric")
	{
		default_random_engine random;
		uniform_real_distribution<float> angle_distribution(0, 2.0f * JPH_PI);
		uniform_real_distribution<float> scale_distribution(0.1f, 10.0f);

		for (int i = 0; i < 1000; ++i)
		{
			// Random scale vector
			Vec3 scale(scale_distribution(random), scale_distribution(random), scale_distribution(random));

			// Random rotation matrix
			Mat44 rotation = Mat44::sRotation(Vec3::sRandom(random), angle_distribution(random));

			// Construct a symmetric tensor from this rotation and scale
			Mat44 tensor4 = rotation.Multiply3x3(Mat44::sScale(scale)).Multiply3x3RightTransposed(rotation);

			// Get the eigenvalues and eigenvectors
			Matrix<3, 3> tensor;
			Matrix<3, 3> eigen_vec = Matrix<3, 3>::sIdentity();
			Vector<3> eigen_val;
			tensor.CopyPart(tensor4, 0, 0, 3, 3, 0, 0);
			CHECK(EigenValueSymmetric(tensor, eigen_vec, eigen_val));

			for (int c = 0; c < 3; ++c)
			{
				// Check that we found a valid eigenvalue
				bool found = false;
				for (int c2 = 0; c2 < 3; ++c2)
					if (abs(scale[c2] - eigen_val[c]) < 1.0e-5f)
					{
						found = true;
						break;
					}
				CHECK(found);

				// Check if the eigenvector is normalized
				CHECK(eigen_vec.GetColumn(c).IsNormalized());

				// Check if matrix * eigen_vector = eigen_value * eigen_vector
				Vector mat_eigvec = tensor * eigen_vec.GetColumn(c);
				Vector eigval_eigvec = eigen_val[c] * eigen_vec.GetColumn(c);
				CHECK(mat_eigvec.IsClose(eigval_eigvec, Square(1.0e-5f)));
			}
		}
	}
}
