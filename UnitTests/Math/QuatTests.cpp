// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include <Jolt/Math/Mat44.h>
#include <Jolt/Math/Quat.h>
#include <Jolt/Core/StringTools.h>
#include <random>

TEST_SUITE("QuatTests")
{
	TEST_CASE("TestQuatSetXYZW")
	{
		Quat q(0, 0, 0, 0);
		CHECK(q == Quat(0, 0, 0, 0));
		q.SetX(1);
		q.SetY(2);
		q.SetZ(3);
		q.SetW(4);
		CHECK(q == Quat(1, 2, 3, 4));

		q.Set(4, 3, 2, 1);
		CHECK(q == Quat(4, 3, 2, 1));
	}

	TEST_CASE("TestQuatEqual")
	{
		CHECK(Quat(1, 2, 3, 4) == Quat(1, 2, 3, 4));
		CHECK(Quat(1, 2, 3, 4) != Quat(0, 2, 3, 4));
		CHECK(Quat(1, 2, 3, 4) != Quat(1, 0, 3, 4));
		CHECK(Quat(1, 2, 3, 4) != Quat(1, 2, 0, 4));
		CHECK(Quat(1, 2, 3, 4) != Quat(1, 2, 3, 0));
	}

	TEST_CASE("TestQuatZero")
	{
		Quat zero = Quat::sZero();
		CHECK(zero == Quat(0, 0, 0, 0));
	}

	TEST_CASE("TestQuatIdentity")
	{
		Quat identity = Quat::sIdentity();

		CHECK_APPROX_EQUAL(identity.GetX(), 0.0f);
		CHECK_APPROX_EQUAL(identity.GetY(), 0.0f);
		CHECK_APPROX_EQUAL(identity.GetZ(), 0.0f);
		CHECK_APPROX_EQUAL(identity.GetW(), 1.0f);
	}

	TEST_CASE("TestQuatIsNaN")
	{
		CHECK(Quat(numeric_limits<float>::quiet_NaN(), 0, 0, 0).IsNaN());
		CHECK(Quat(0, numeric_limits<float>::quiet_NaN(), 0, 0).IsNaN());
		CHECK(Quat(0, 0, numeric_limits<float>::quiet_NaN(), 0).IsNaN());
		CHECK(Quat(0, 0, 0, numeric_limits<float>::quiet_NaN()).IsNaN());
	}

	TEST_CASE("TestQuatOperators")
	{
		CHECK(-Quat(1, 2, 3, 4) == Quat(-1, -2, -3, -4));
		CHECK(Quat(1, 2, 3, 4) + Quat(5, 6, 7, 8) == Quat(6, 8, 10, 12));
		CHECK(Quat(5, 6, 7, 8) - Quat(4, 3, 2, 1) == Quat(1, 3, 5, 7));
		CHECK(Quat(1, 2, 3, 4) * 5.0f == Quat(5, 10, 15, 20));
		CHECK(5.0f * Quat(1, 2, 3, 4) == Quat(5, 10, 15, 20));
		CHECK(Quat(2, 4, 6, 8) / 2.0f == Quat(1, 2, 3, 4));

		Quat v(1, 2, 3, 4);
		v += Quat(5, 6, 7, 8);
		CHECK(v == Quat(6, 8, 10, 12));
		v -= Quat(4, 3, 2, 1);
		CHECK(v == Quat(2, 5, 8, 11));
		v *= 2.0f;
		CHECK(v == Quat(4, 10, 16, 22));
		v /= 2.0f;
		CHECK(v == Quat(2, 5, 8, 11));
	}

	TEST_CASE("TestQuatPerpendicular")
	{
		Quat q1(1, 2, 3, 4);
		CHECK(q1.GetPerpendicular().Dot(q1) == 0.0f);

		Quat q2(-5, 4, -3, 2);
		CHECK(q2.GetPerpendicular().Dot(q2) == 0.0f);
	}

	TEST_CASE("TestQuatNormalized")
	{
		CHECK(Quat(1, 0, 0, 0).IsNormalized());
		CHECK(Quat(-0.7071067f, 0.7071067f, 0, 0).IsNormalized());
		CHECK(Quat(0.5773502f, -0.5773502f, 0.5773502f, 0).IsNormalized());
		CHECK(Quat(0.5f, -0.5f, 0.5f, -0.5f).IsNormalized());
		CHECK(!Quat(2, 0, 0, 0).IsNormalized());
		CHECK(!Quat(0, 2, 0, 0).IsNormalized());
		CHECK(!Quat(0, 0, 2, 0).IsNormalized());
		CHECK(!Quat(0, 0, 0, 2).IsNormalized());
	}

	TEST_CASE("TestQuatConvertMatrix")
	{
		UnitTestRandom random;
		uniform_real_distribution<float> zero_to_two_pi(0.0f, 2.0f * JPH_PI);
		for (int i = 0; i < 1000; ++i)
		{
			Vec3 axis = Vec3::sRandom(random);
			float angle = zero_to_two_pi(random);

			Mat44 m1 = Mat44::sRotation(axis, angle);
			Quat q1 = m1.GetQuaternion();
			Quat q2 = Quat::sRotation(axis, angle);
			CHECK_APPROX_EQUAL(q1, q2);
			Mat44 m2 = Mat44::sRotation(q2);
			CHECK_APPROX_EQUAL(m1, m2);
		}
	}

	TEST_CASE("TestQuatMultiplyVec3")
	{
		UnitTestRandom random;
		uniform_real_distribution<float> zero_to_two_pi(0.0f, 2.0f * JPH_PI);
		for (int i = 0; i < 1000; ++i)
		{
			Vec3 axis = Vec3::sRandom(random);
			float angle = zero_to_two_pi(random);
			Mat44 m1 = Mat44::sRotation(axis, angle);
			Quat q1 = Quat::sRotation(axis, angle);

			Vec3 rv = 10.0f * Vec3::sRandom(random);
			Vec3 r1 = m1 * rv;
			Vec3 r2 = q1 * rv;
			CHECK_APPROX_EQUAL(r1, r2, 1.0e-5f);
		}
	}

	TEST_CASE("TestQuatRotateAxisXYZ")
	{
		UnitTestRandom random;
		uniform_real_distribution<float> zero_to_two_pi(0.0f, 2.0f * JPH_PI);
		for (int i = 0; i < 1000; ++i)
		{
			Vec3 axis = Vec3::sRandom(random);
			float angle = zero_to_two_pi(random);
			Quat q1 = Quat::sRotation(axis, angle);

			Vec3 r1 = q1 * Vec3::sAxisX();
			Vec3 r2 = q1.RotateAxisX();
			CHECK_APPROX_EQUAL(r1, r2, 1.0e-5f);

			r1 = q1 * Vec3::sAxisY();
			r2 = q1.RotateAxisY();
			CHECK_APPROX_EQUAL(r1, r2, 1.0e-5f);

			r1 = q1 * Vec3::sAxisZ();
			r2 = q1.RotateAxisZ();
			CHECK_APPROX_EQUAL(r1, r2, 1.0e-5f);
		}
	}

	TEST_CASE("TestQuatMultiplyQuat")
	{
		{
			// We use a right handed system, so test that: i * j = k
			Quat r1 = Quat(1, 0, 0, 0) * Quat(0, 1, 0, 0);
			Quat r2 = Quat(0, 0, 1, 0);
			CHECK(r1.IsClose(r2));
		}

		{
			// Test: j * i = -k
			Quat r1 = Quat(0, 1, 0, 0) * Quat(1, 0, 0, 0);
			Quat r2 = Quat(0, 0, -1, 0);
			CHECK(r1.IsClose(r2));
		}

		{
			// Test predefined multiplication
			Quat r1 = Quat(2, 3, 4, 1) * Quat(6, 7, 8, 5);
			Quat r2 = Quat(12, 30, 24, -60);
			CHECK(r1.IsClose(r2));
		}

		// Compare random matrix multiplications with quaternion multiplications
		UnitTestRandom random;
		uniform_real_distribution<float> zero_to_two_pi(0.0f, 2.0f * JPH_PI);
		for (int i = 0; i < 1000; ++i)
		{
			Vec3 axis1 = Vec3::sRandom(random);
			float angle1 = zero_to_two_pi(random);
			Quat q1 = Quat::sRotation(axis1, angle1);
			Mat44 m1 = Mat44::sRotation(axis1, angle1);

			Vec3 axis2 = Vec3::sRandom(random);
			float angle2 = zero_to_two_pi(random);
			Quat q2 = Quat::sRotation(axis2, angle2);
			Mat44 m2 = Mat44::sRotation(axis2, angle2);

			Quat r1 = q1 * q2;
			Quat r2 = (m1 * m2).GetQuaternion();

			CHECK_APPROX_EQUAL(r1, r2);
		}
	}

	TEST_CASE("TestQuatRotationAxisAngle")
	{
		Mat44 r1 = Mat44::sRotation(Vec3(1, 0, 0), 0.1f * JPH_PI);
		Mat44 r2 = Mat44::sRotation(Quat::sRotation(Vec3(1, 0, 0), 0.1f * JPH_PI));
		CHECK_APPROX_EQUAL(r1, r2);

		r1 = Mat44::sRotation(Vec3(0, 1, 0), 0.2f * JPH_PI);
		r2 = Mat44::sRotation(Quat::sRotation(Vec3(0, 1, 0), 0.2f * JPH_PI));
		CHECK_APPROX_EQUAL(r1, r2);

		r1 = Mat44::sRotation(Vec3(0, 0, 1), 0.3f * JPH_PI);
		r2 = Mat44::sRotation(Quat::sRotation(Vec3(0, 0, 1), 0.3f * JPH_PI));
		CHECK_APPROX_EQUAL(r1, r2);
	}

	TEST_CASE("TestQuatGetAxisAngle")
	{
		// Test identity rotation
		{
			Vec3 axis;
			float angle;
			Quat::sIdentity().GetAxisAngle(axis, angle);
			CHECK_APPROX_EQUAL(Vec3::sZero(), axis);
			CHECK_APPROX_EQUAL(0.0f, angle);
		}

		{
			Vec3 axis;
			float angle;
			(-Quat::sIdentity()).GetAxisAngle(axis, angle);
			CHECK_APPROX_EQUAL(Vec3::sZero(), axis);
			CHECK_APPROX_EQUAL(0.0f, angle);
		}

		// Test positive rotation
		Quat q1 = Quat::sRotation(Vec3(0, 1, 0), 0.2f * JPH_PI);

		{
			Vec3 axis;
			float angle;
			q1.GetAxisAngle(axis, angle);
			CHECK_APPROX_EQUAL(Vec3(0, 1, 0), axis);
			CHECK_APPROX_EQUAL(0.2f * JPH_PI, angle, 1.0e-5f);
		}

		{
			Vec3 axis;
			float angle;
			(-q1).GetAxisAngle(axis, angle);
			CHECK_APPROX_EQUAL(Vec3(0, 1, 0), axis);
			CHECK_APPROX_EQUAL(0.2f * JPH_PI, angle, 1.0e-5f);
		}

		// Test negative rotation
		Quat q2 = Quat::sRotation(Vec3(0, 1, 0), -0.2f * JPH_PI);

		{
			Vec3 axis;
			float angle;
			q2.GetAxisAngle(axis, angle);
			CHECK_APPROX_EQUAL(Vec3(0, -1, 0), axis);
			CHECK_APPROX_EQUAL(0.2f * JPH_PI, angle, 1.0e-5f);
		}

		{
			Vec3 axis;
			float angle;
			(-q2).GetAxisAngle(axis, angle);
			CHECK_APPROX_EQUAL(Vec3(0, -1, 0), axis);
			CHECK_APPROX_EQUAL(0.2f * JPH_PI, angle, 1.0e-5f);
		}

		// Test keeping range between [0, PI]
		Quat q3 = Quat::sRotation(Vec3(0, 1, 0), 1.1f * JPH_PI);

		{
			Vec3 axis;
			float angle;
			q3.GetAxisAngle(axis, angle);
			CHECK_APPROX_EQUAL(Vec3(0, -1, 0), axis);
			CHECK_APPROX_EQUAL(0.9f * JPH_PI, angle, 1.0e-5f);
		}

		{
			Vec3 axis;
			float angle;
			(-q3).GetAxisAngle(axis, angle);
			CHECK_APPROX_EQUAL(Vec3(0, -1, 0), axis);
			CHECK_APPROX_EQUAL(0.9f * JPH_PI, angle, 1.0e-5f);
		}
	}

	TEST_CASE("TestQuatInverse")
	{
		UnitTestRandom random;
		uniform_real_distribution<float> zero_to_two_pi(0.0f, 2.0f * JPH_PI);
		for (int i = 0; i < 1000; ++i)
		{
			Vec3 axis = Vec3::sRandom(random);
			float angle = zero_to_two_pi(random);

			Quat q1 = Quat::sRotation(axis, angle);
			Quat q2 = q1.Inversed();

			CHECK_APPROX_EQUAL(Quat::sIdentity(), q1 * q2);
		}
	}

	TEST_CASE("TestQuatConjugate")
	{
		CHECK(Quat(1, 2, 3, 4).Conjugated() == Quat(-1, -2, -3, 4));
		CHECK(Quat(-1, -2, -3, -4).Conjugated() == Quat(1, 2, 3, -4));
	}

	TEST_CASE("TestQuatEnsureWPositive")
	{
		CHECK(Quat(1, -2, 3, -4).EnsureWPositive() == Quat(-1, 2, -3, 4));
		CHECK(Quat(-4, 5, -6, 7).EnsureWPositive() == Quat(-4, 5, -6, 7));
		CHECK(Quat(1, 2, 3, 0).EnsureWPositive() == Quat(1, 2, 3, 0));
	}

	TEST_CASE("TestQuatStoreFloat3")
	{
		Float3 q1;
		Quat(0.7071067f, 0, 0, -0.7071067f).StoreFloat3(&q1);
		CHECK(q1 == Float3(-0.7071067f, 0, 0));

		Float3 q2;
		Quat(0, 0.7071067f, 0, 0.7071067f).StoreFloat3(&q2);
		CHECK(q2 == Float3(0, 0.7071067f, 0));

		Float3 q3;
		Quat(0, 0, 1, 0).StoreFloat3(&q3);
		CHECK(q3 == Float3(0, 0, 1));
	}

	TEST_CASE("TestQuatGetTwistAxis")
	{
		Quat q1 = Quat::sRotation(Vec3::sAxisX(), DegreesToRadians(-10.0f));
		Quat q2 = Quat::sRotation(Vec3::sAxisY(), DegreesToRadians(20.0f));
		Quat q = q1 * q2;

		Quat twist1 = q.GetTwist(Vec3::sAxisX());
		CHECK_APPROX_EQUAL(twist1, q1);
		Quat swing1 = twist1.Inversed() * q;
		CHECK_APPROX_EQUAL(swing1, q2);
		Quat twist2 = swing1.GetTwist(Vec3::sAxisY());
		CHECK_APPROX_EQUAL(twist2, q2);
		Quat swing2 = twist2.Inversed() * swing1;
		CHECK_APPROX_EQUAL(swing2, Quat::sIdentity());

		CHECK(Quat::sZero().GetTwist(Vec3::sAxisX()) == Quat::sIdentity());
	}

	TEST_CASE("TestQuatGetRotationAngle")
	{
		Quat q1 = Quat::sRotation(Vec3::sAxisX(), DegreesToRadians(-10.0f));
		Quat q2 = Quat::sRotation(Vec3::sAxisY(), DegreesToRadians(20.0f));
		Quat q3 = Quat::sRotation(Vec3::sAxisZ(), DegreesToRadians(-95.0f));

		float a = q1.GetRotationAngle(Vec3::sAxisX());
		CHECK_APPROX_EQUAL(a, DegreesToRadians(-10.0f), 1.0e-5f);

		a = q2.GetRotationAngle(Vec3::sAxisY());
		CHECK_APPROX_EQUAL(a, DegreesToRadians(20.0f), 1.0e-5f);

		a = q3.GetRotationAngle(Vec3::sAxisZ());
		CHECK_APPROX_EQUAL(a, DegreesToRadians(-95.0f), 1.0e-5f);

		a = (q1 * q2).GetRotationAngle(Vec3::sAxisX());
		CHECK_APPROX_EQUAL(a, DegreesToRadians(-10.0f), 1.0e-5f);

		a = (q3 * q1).GetRotationAngle(Vec3::sAxisX());
		CHECK_APPROX_EQUAL(a, DegreesToRadians(-10.0f), 1.0e-5f);
	}

	TEST_CASE("TestQuatGetEulerAngles")
	{
		Vec3 input(DegreesToRadians(-10.0f), DegreesToRadians(20.0f), DegreesToRadians(-95.0f));

		Quat qx = Quat::sRotation(Vec3::sAxisX(), input.GetX());
		Quat qy = Quat::sRotation(Vec3::sAxisY(), input.GetY());
		Quat qz = Quat::sRotation(Vec3::sAxisZ(), input.GetZ());
		Quat q = qz * qy * qx;

		Quat q2 = Quat::sEulerAngles(input);
		CHECK_APPROX_EQUAL(q, q2);

		Vec3 angles = q2.GetEulerAngles();
		CHECK_APPROX_EQUAL(angles, input);
	}

	TEST_CASE("TestQuatRotationFromTo")
	{
		{
			// Parallel vectors
			Vec3 v1(10, 0, 0);
			Vec3 v2(20, 0, 0);
			Quat q = Quat::sFromTo(v1, v2);
			CHECK_APPROX_EQUAL(q, Quat::sIdentity());
		}

		{
			// Perpendicular vectors
			Vec3 v1(10, 0, 0);
			Vec3 v2(0, 20, 0);
			Quat q = Quat::sFromTo(v1, v2);
			CHECK_APPROX_EQUAL(v2.Normalized(), (q * v1).Normalized());
		}

		{
			// Vectors with 180 degree angle
			Vec3 v1(10, 0, 0);
			Vec3 v2(-20, 0, 0);
			Quat q = Quat::sFromTo(v1, v2);
			CHECK_APPROX_EQUAL(v2.Normalized(), (q * v1).Normalized());
		}

		{
			// Test v1 zero
			Vec3 v1 = Vec3::sZero();
			Vec3 v2(10, 0, 0);
			Quat q = Quat::sFromTo(v1, v2);
			CHECK(q == Quat::sIdentity());
		}

		{
			// Test v2 zero
			Vec3 v1(10, 0, 0);
			Vec3 v2 = Vec3::sZero();
			Quat q = Quat::sFromTo(v1, v2);
			CHECK(q == Quat::sIdentity());
		}

		{
			// Length of a vector is squared inside the function: try with sqrt(FLT_MIN) to see if that still returns a valid rotation
			Vec3 v1(0, sqrt(FLT_MIN), 0);
			Vec3 v2(1, 0, 0);
			Quat q = Quat::sFromTo(v1, v2);
			CHECK_APPROX_EQUAL(v2.Normalized(), (q * v1).Normalized());
		}
	}

	TEST_CASE("TestQuatRotationFromToRandom")
	{
		UnitTestRandom random;
		uniform_real_distribution<float> one_to_ten(1.0f, 10.0f);
		for (int i = 0; i < 1000; ++i)
		{
			Vec3 v1 = one_to_ten(random) * Vec3::sRandom(random);
			Vec3 v2 = one_to_ten(random) * Vec3::sRandom(random);

			Quat q = Quat::sFromTo(v1, v2);

			Vec3 v1t = (q * v1).Normalized();
			Vec3 v2t = v2.Normalized();
			CHECK_APPROX_EQUAL(v2t, v1t, 1.0e-5f);
		}
	}

	TEST_CASE("TestQuatConvertToString")
	{
		Quat v(1, 2, 3, 4);
		CHECK(ConvertToString(v) == "1, 2, 3, 4");
	}

	TEST_CASE("TestQuatLERP")
	{
		Quat v1(1, 2, 3, 4);
		Quat v2(5, 6, 7, 8);
		CHECK(v1.LERP(v2, 0.25f) == Quat(2, 3, 4, 5));
	}

	TEST_CASE("TestQuatSLERP")
	{
		Quat v1 = Quat::sIdentity();
		Quat v2 = Quat::sRotation(Vec3::sAxisX(), 0.99f * JPH_PI);
		CHECK_APPROX_EQUAL(v1.SLERP(v2, 0.25f), Quat::sRotation(Vec3::sAxisX(), 0.25f * 0.99f * JPH_PI));

		// Check that we ignore the sign
		Quat v3 = Quat(1, 2, 3, 4).Normalized();
		CHECK_APPROX_EQUAL(v3.SLERP(-v3, 0.5f), v3);
	}

	TEST_CASE("TestQuatMultiplyImaginary")
	{
		UnitTestRandom random;
		for (int i = 0; i < 1000; ++i)
		{
			Vec3 imaginary = Vec3::sRandom(random);
			Quat quat = Quat::sRandom(random);

			Quat r1 = Quat::sMultiplyImaginary(imaginary, quat);
			Quat r2 = Quat(Vec4(imaginary, 0)) * quat;
			CHECK_APPROX_EQUAL(r1, r2);
		}
	}
}
