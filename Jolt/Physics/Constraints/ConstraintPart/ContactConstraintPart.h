// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Physics/Body/Body.h>

JPH_NAMESPACE_BEGIN

/// This is a copy of AxisConstraintPart, specialized to handle contact constraints. See the documentation of AxisConstraintPart for more documentation behind the math.
template <EMotionType Type1, EMotionType Type2>
class ContactConstraintPart
{
public:
	static inline float		sGetEffectiveMass(Vec3Arg inR1, Vec3Arg inR2, float inInvM1, Mat44Arg inInvI1, float inInvM2, Mat44Arg inInvI2, Vec3Arg inWorldSpaceAxis)
	{
		// Calculate inverse effective mass: K = J M^-1 J^T
		float inv_effective_mass;

		if constexpr (Type1 == EMotionType::Dynamic)
		{
			Vec3 r1_cross_axis = inR1.Cross(inWorldSpaceAxis);
			Vec3 inv_i1_r1_cross_axis = inInvI1.Multiply3x3(r1_cross_axis);
			inv_effective_mass = inInvM1 + inv_i1_r1_cross_axis.Dot(r1_cross_axis);
		}
		else
			inv_effective_mass = 0.0f;

		if constexpr (Type2 == EMotionType::Dynamic)
		{
			Vec3 r2_cross_axis = inR2.Cross(inWorldSpaceAxis);
			Vec3 inv_i2_r2_cross_axis = inInvI2.Multiply3x3(r2_cross_axis);
			inv_effective_mass += inInvM2 + inv_i2_r2_cross_axis.Dot(r2_cross_axis);
		}

		return 1.0f / (inv_effective_mass + 1.0e-24f);
	}

	static inline float		sGetMinusJV(Vec3Arg inR1, Vec3Arg inR2, Vec3Arg inLinearVelocity1, Vec3Arg inAngularVelocity1, Vec3Arg inLinearVelocity2, Vec3Arg inAngularVelocity2, Vec3Arg inWorldSpaceAxis)
	{
		// Calculate jacobian multiplied by linear velocity
		float jv;
		if constexpr (Type1 != EMotionType::Static && Type2 != EMotionType::Static)
			jv = inWorldSpaceAxis.Dot(inLinearVelocity1 - inLinearVelocity2);
		else if constexpr (Type1 != EMotionType::Static)
			jv = inWorldSpaceAxis.Dot(inLinearVelocity1);
		else if constexpr (Type2 != EMotionType::Static)
			jv = inWorldSpaceAxis.Dot(-inLinearVelocity2);
		else
			JPH_ASSERT(false); // Static vs static is nonsensical!

		// Calculate jacobian multiplied by angular velocity
		if constexpr (Type1 != EMotionType::Static)
			jv += inR1.Cross(inWorldSpaceAxis).Dot(inAngularVelocity1);
		if constexpr (Type2 != EMotionType::Static)
			jv -= inR2.Cross(inWorldSpaceAxis).Dot(inAngularVelocity2);

		return jv;
	}

	static inline float		sGetMinusJV(const BodyState *inState1, const BodyState *inState2, Vec3Arg inR1, Vec3Arg inR2, Vec3Arg inWorldSpaceAxis)
	{
		// Calculate jacobian multiplied by linear velocity
		float jv;
		if constexpr (Type1 != EMotionType::Static && Type2 != EMotionType::Static)
			jv = inWorldSpaceAxis.Dot(inState1->mLinearVelocity - inState2->mLinearVelocity);
		else if constexpr (Type1 != EMotionType::Static)
			jv = inWorldSpaceAxis.Dot(inState1->mLinearVelocity);
		else if constexpr (Type2 != EMotionType::Static)
			jv = -inWorldSpaceAxis.Dot(inState2->mLinearVelocity);
		else
			JPH_ASSERT(false); // Static vs static is nonsensical!

		// Calculate jacobian multiplied by angular velocity
		if constexpr (Type1 != EMotionType::Static)
			jv += inR1.Cross(inWorldSpaceAxis).Dot(inState1->mAngularVelocity);
		if constexpr (Type2 != EMotionType::Static)
			jv -= inR2.Cross(inWorldSpaceAxis).Dot(inState2->mAngularVelocity);

		return jv;
	}

	static inline float	sGetImpulse(const BodyState *inState1, const BodyState *inState2, Vec3Arg inR1, Vec3Arg inR2, float inEffectiveMass, Vec3Arg inWorldSpaceAxis, float inBias = 0.0f)
	{
		float jv = sGetMinusJV(inState1, inState2, inR1, inR2, inWorldSpaceAxis);
		return inEffectiveMass * (jv - inBias);
	}

	inline float		GetInvEffectiveMass(Vec3Arg inR1, Vec3Arg inR2, float inInvM1, Mat44Arg inInvI1, float inInvM2, Mat44Arg inInvI2, Vec3Arg inWorldSpaceAxis)
	{
		// Calculate inverse effective mass: K = J M^-1 J^T
		float inv_effective_mass;

		if constexpr (Type1 != EMotionType::Static)
		{
			mR1xAxis = inR1.Cross(inWorldSpaceAxis);

			if constexpr (Type1 == EMotionType::Dynamic)
			{
				mInvI1_R1xAxis = inInvI1.Multiply3x3(mR1xAxis);
				inv_effective_mass = inInvM1 + mInvI1_R1xAxis.Dot(mR1xAxis);
			}
			else
				inv_effective_mass = 0.0f;
		}
		else
			inv_effective_mass = 0.0f;

		if constexpr (Type2 != EMotionType::Static)
		{
			mR2xAxis = inR2.Cross(inWorldSpaceAxis);

			if constexpr (Type2 == EMotionType::Dynamic)
			{
				mInvI2_R2xAxis = inInvI2.Multiply3x3(mR2xAxis);
				inv_effective_mass += inInvM2 + mInvI2_R2xAxis.Dot(mR2xAxis);
			}
		}

		return inv_effective_mass + 1.0e-24f; // Prevent having to check for division by zero
	}

	inline float		GetMinusJV(Vec3Arg inLinearVelocity1, Vec3Arg inAngularVelocity1, Vec3Arg inLinearVelocity2, Vec3Arg inAngularVelocity2, Vec3Arg inWorldSpaceAxis) const
	{
		// Calculate jacobian multiplied by linear velocity
		float jv;
		if constexpr (Type1 != EMotionType::Static && Type2 != EMotionType::Static)
			jv = inWorldSpaceAxis.Dot(inLinearVelocity1 - inLinearVelocity2);
		else if constexpr (Type1 != EMotionType::Static)
			jv = inWorldSpaceAxis.Dot(inLinearVelocity1);
		else if constexpr (Type2 != EMotionType::Static)
			jv = -inWorldSpaceAxis.Dot(inLinearVelocity2);
		else
			JPH_ASSERT(false); // Static vs static is nonsensical!

		// Calculate jacobian multiplied by angular velocity
		if constexpr (Type1 != EMotionType::Static)
			jv += mR1xAxis.Dot(inAngularVelocity1);
		if constexpr (Type2 != EMotionType::Static)
			jv -= mR2xAxis.Dot(inAngularVelocity2);

		return jv;
	}

	inline float		GetImpulse(Vec3Arg inR1, Vec3Arg inR2, Vec3Arg inLinearVelocity1, Vec3Arg inAngularVelocity1, Vec3Arg inLinearVelocity2, Vec3Arg inAngularVelocity2, float inInvMass1, Mat44Arg inInvI1, float inInvMass2, Mat44Arg inInvI2, Vec3Arg inWorldSpaceAxis, float inBias = 0.0f)
	{
		float inv_effective_mass = GetInvEffectiveMass(inR1, inR2, inInvMass1, inInvI1, inInvMass2, inInvI2, inWorldSpaceAxis);
		float jv = GetMinusJV(inLinearVelocity1, inAngularVelocity1, inLinearVelocity2, inAngularVelocity2, inWorldSpaceAxis);
		return (jv - inBias) / inv_effective_mass;
	}

	inline void			ApplyImpulse(Vec3 &ioLinearVelocity1, Vec3 &ioAngularVelocity1, Vec3 &ioLinearVelocity2, Vec3 &ioAngularVelocity2, float inInvMass1, float inInvMass2, float inLambda, Vec3Arg inWorldSpaceAxis) const
	{
		Vec3 impulse = inLambda * inWorldSpaceAxis;
		if constexpr (Type1 == EMotionType::Dynamic)
		{
			ioLinearVelocity1 -= inInvMass1 * impulse;
			ioAngularVelocity1 -= inLambda * mInvI1_R1xAxis;
		}
		if constexpr (Type2 == EMotionType::Dynamic)
		{
			ioLinearVelocity2 += inInvMass2 * impulse;
			ioAngularVelocity2 += inLambda * mInvI2_R2xAxis;
		}
	}

private:
	Vec3				mR1xAxis;
	Vec3				mR2xAxis;
	Vec3				mInvI1_R1xAxis;
	Vec3				mInvI2_R2xAxis;
};

JPH_NAMESPACE_END
