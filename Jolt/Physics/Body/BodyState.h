// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

JPH_NAMESPACE_BEGIN

/// State of a body, internally used during the simulation step to have a compact representation of the relevant properties
class BodyState
{
public:
	void			AddRotationStep(Vec3Arg inAngularVelocityTimesDeltaTime)
	{
		// This used to use the equation: d/dt R(t) = 1/2 * w(t) * R(t) so that R(t + dt) = R(t) + 1/2 * w(t) * R(t) * dt
		// See: Appendix B of An Introduction to Physically Based Modeling: Rigid Body Simulation II-Nonpenetration Constraints
		// URL: https://www.cs.cmu.edu/~baraff/sigcourse/notesd2.pdf
		// But this is a first order approximation and does not work well for kinematic ragdolls that are driven to a new
		// pose if the poses differ enough. So now we split w(t) * dt into an axis and angle part and create a quaternion with it.
		float len = inAngularVelocityTimesDeltaTime.Length();
		if (len > 1.0e-6f)
		{
			mDeltaRotation = Quat::sRotation(inAngularVelocityTimesDeltaTime / len, len) * mDeltaRotation;
			JPH_ASSERT(!mDeltaRotation.IsNaN());
		}
	}

	Vec3			mLinearVelocity;
	Vec3			mAngularVelocity;
	Vec3			mDeltaPosition;
	Quat			mDeltaRotation;
};

JPH_NAMESPACE_END
