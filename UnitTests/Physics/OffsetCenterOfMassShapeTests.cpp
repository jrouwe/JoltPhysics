// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include "PhysicsTestContext.h"
#include "Layers.h"
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/OffsetCenterOfMassShape.h>

TEST_SUITE("OffsetCenterOfMassShapeTests")
{
	TEST_CASE("TestAddAngularImpulseCOMZero")
	{
		PhysicsTestContext c;
		c.ZeroGravity();

		// Create box
		const Vec3 cHalfExtent = Vec3(0.5f, 1.0f, 1.5f);
		BoxShapeSettings box(cHalfExtent);
		box.SetEmbedded();

		// Create body with COM offset 0
		OffsetCenterOfMassShapeSettings com(Vec3::sZero(), &box);
		com.SetEmbedded();
		Body &body = c.CreateBody(&com, RVec3::sZero(), Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING, EActivation::DontActivate);

		// Check mass and inertia calculated correctly
		float mass = (8.0f * cHalfExtent.GetX() * cHalfExtent.GetY() * cHalfExtent.GetZ()) * box.mDensity;
		CHECK_APPROX_EQUAL(body.GetMotionProperties()->GetInverseMass(), 1.0f / mass);
		float inertia_y = mass / 12.0f * (Square(2.0f * cHalfExtent.GetX()) + Square(2.0f * cHalfExtent.GetZ())); // See: https://en.wikipedia.org/wiki/List_of_moments_of_inertia
		CHECK_APPROX_EQUAL(body.GetMotionProperties()->GetInverseInertiaForRotation(Mat44::sIdentity())(1, 1), 1.0f / inertia_y);

		// Add impulse
		Vec3 cImpulse(0, 10000, 0);
		CHECK(!body.IsActive());
		c.GetBodyInterface().AddAngularImpulse(body.GetID(), cImpulse);
		CHECK(body.IsActive());

		// Check resulting velocity change
		// dv = I^-1 * L
		float delta_v = (1.0f / inertia_y) * cImpulse.GetY();
		CHECK_APPROX_EQUAL(body.GetLinearVelocity(), Vec3::sZero());
		CHECK_APPROX_EQUAL(body.GetAngularVelocity(), Vec3(0, delta_v, 0));
	}

	TEST_CASE("TestAddAngularImpulseCOMOffset")
	{
		PhysicsTestContext c;
		c.ZeroGravity();

		// Create box
		const Vec3 cHalfExtent = Vec3(0.5f, 1.0f, 1.5f);
		BoxShapeSettings box(cHalfExtent);
		box.SetEmbedded();

		// Create body with COM offset
		const Vec3 cCOMOffset(5.0f, 0, 0);
		OffsetCenterOfMassShapeSettings com(cCOMOffset, &box);
		com.SetEmbedded();
		Body &body = c.CreateBody(&com, RVec3::sZero(), Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING, EActivation::DontActivate);

		// Check mass and inertia calculated correctly
		float mass = (8.0f * cHalfExtent.GetX() * cHalfExtent.GetY() * cHalfExtent.GetZ()) * box.mDensity;
		CHECK_APPROX_EQUAL(body.GetMotionProperties()->GetInverseMass(), 1.0f / mass);
		float inertia_y = mass / 12.0f * (Square(2.0f * cHalfExtent.GetX()) + Square(2.0f * cHalfExtent.GetZ())) + mass * Square(cCOMOffset.GetX()); // See: https://en.wikipedia.org/wiki/List_of_moments_of_inertia & https://en.wikipedia.org/wiki/Parallel_axis_theorem
		CHECK_APPROX_EQUAL(body.GetMotionProperties()->GetInverseInertiaForRotation(Mat44::sIdentity())(1, 1), 1.0f / inertia_y);

		// Add impulse
		Vec3 cImpulse(0, 10000, 0);
		CHECK(!body.IsActive());
		c.GetBodyInterface().AddAngularImpulse(body.GetID(), cImpulse);
		CHECK(body.IsActive());

		// Check resulting velocity change
		// dv = I^-1 * L
		float delta_v = (1.0f / inertia_y) * cImpulse.GetY();
		CHECK_APPROX_EQUAL(body.GetLinearVelocity(), Vec3::sZero());
		CHECK_APPROX_EQUAL(body.GetAngularVelocity(), Vec3(0, delta_v, 0));
	}

	TEST_CASE("TestAddTorqueCOMZero")
	{
		PhysicsTestContext c;
		c.ZeroGravity();

		// Create box
		const Vec3 cHalfExtent = Vec3(0.5f, 1.0f, 1.5f);
		BoxShapeSettings box(cHalfExtent);
		box.SetEmbedded();

		// Create body with COM offset 0
		OffsetCenterOfMassShapeSettings com(Vec3::sZero(), &box);
		com.SetEmbedded();
		Body &body = c.CreateBody(&com, RVec3::sZero(), Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING, EActivation::DontActivate);

		// Check mass and inertia calculated correctly
		float mass = (8.0f * cHalfExtent.GetX() * cHalfExtent.GetY() * cHalfExtent.GetZ()) * box.mDensity;
		CHECK_APPROX_EQUAL(body.GetMotionProperties()->GetInverseMass(), 1.0f / mass);
		float inertia_y = mass / 12.0f * (Square(2.0f * cHalfExtent.GetX()) + Square(2.0f * cHalfExtent.GetZ())); // See: https://en.wikipedia.org/wiki/List_of_moments_of_inertia
		CHECK_APPROX_EQUAL(body.GetMotionProperties()->GetInverseInertiaForRotation(Mat44::sIdentity())(1, 1), 1.0f / inertia_y);

		// Add torque
		Vec3 cTorque(0, 100000, 0);
		CHECK(!body.IsActive());
		c.GetBodyInterface().AddTorque(body.GetID(), cTorque);
		CHECK(body.IsActive());
		CHECK(body.GetAngularVelocity() == Vec3::sZero()); // Angular velocity change should come after the next time step
		c.SimulateSingleStep();

		// Check resulting velocity change
		// dv = I^-1 * T * dt
		float delta_v = (1.0f / inertia_y) * cTorque.GetY() * c.GetDeltaTime();
		CHECK_APPROX_EQUAL(body.GetLinearVelocity(), Vec3::sZero());
		CHECK_APPROX_EQUAL(body.GetAngularVelocity(), Vec3(0, delta_v, 0));
	}

	TEST_CASE("TestAddTorqueCOMOffset")
	{
		PhysicsTestContext c;
		c.ZeroGravity();

		// Create box
		const Vec3 cHalfExtent = Vec3(0.5f, 1.0f, 1.5f);
		BoxShapeSettings box(cHalfExtent);
		box.SetEmbedded();

		// Create body with COM offset
		const Vec3 cCOMOffset(5.0f, 0, 0);
		OffsetCenterOfMassShapeSettings com(cCOMOffset, &box);
		com.SetEmbedded();
		Body &body = c.CreateBody(&com, RVec3::sZero(), Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING, EActivation::DontActivate);

		// Check mass and inertia calculated correctly
		float mass = (8.0f * cHalfExtent.GetX() * cHalfExtent.GetY() * cHalfExtent.GetZ()) * box.mDensity;
		CHECK_APPROX_EQUAL(body.GetMotionProperties()->GetInverseMass(), 1.0f / mass);
		float inertia_y = mass / 12.0f * (Square(2.0f * cHalfExtent.GetX()) + Square(2.0f * cHalfExtent.GetZ())) + mass * Square(cCOMOffset.GetX()); // See: https://en.wikipedia.org/wiki/List_of_moments_of_inertia & https://en.wikipedia.org/wiki/Parallel_axis_theorem
		CHECK_APPROX_EQUAL(body.GetMotionProperties()->GetInverseInertiaForRotation(Mat44::sIdentity())(1, 1), 1.0f / inertia_y);

		// Add torque
		Vec3 cTorque(0, 100000, 0);
		CHECK(!body.IsActive());
		c.GetBodyInterface().AddTorque(body.GetID(), cTorque);
		CHECK(body.IsActive());
		CHECK(body.GetAngularVelocity() == Vec3::sZero()); // Angular velocity change should come after the next time step
		c.SimulateSingleStep();

		// Check resulting velocity change
		// dv = I^-1 * T * dt
		float delta_v = (1.0f / inertia_y) * cTorque.GetY() * c.GetDeltaTime();
		CHECK_APPROX_EQUAL(body.GetLinearVelocity(), Vec3::sZero());
		CHECK_APPROX_EQUAL(body.GetAngularVelocity(), Vec3(0, delta_v, 0));
	}
}
