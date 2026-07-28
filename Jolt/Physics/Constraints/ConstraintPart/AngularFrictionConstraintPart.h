// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Physics/Constraints/ConstraintPart/ContactConstraintPart.h>

JPH_NAMESPACE_BEGIN

/// This is a copy of AngleConstraintPart, specialized to handle contact constraints. See the documentation of AngleConstraintPart for more documentation behind the math.
template <EMotionType Type1, EMotionType Type2>
class AngularFrictionConstraintPart
{
public:
	static inline float	sGetEffectiveMass(Mat44Arg inInvI1, Mat44Arg inInvI2, Vec3Arg inWorldSpaceAxis)
	{
		float inv_effective_mass = 0.0f;
		if constexpr (Type1 == EMotionType::Dynamic && Type2 == EMotionType::Dynamic)
			inv_effective_mass = inWorldSpaceAxis.Dot(inInvI1.Multiply3x3(inWorldSpaceAxis) + inInvI2.Multiply3x3(inWorldSpaceAxis));
		else if constexpr (Type1 == EMotionType::Dynamic)
			inv_effective_mass = inWorldSpaceAxis.Dot(inInvI1.Multiply3x3(inWorldSpaceAxis));
		else if constexpr (Type2 == EMotionType::Dynamic)
			inv_effective_mass = inWorldSpaceAxis.Dot(inInvI2.Multiply3x3(inWorldSpaceAxis));
		else
			JPH_ASSERT(false); // Static vs static is nonsensical!

		return 1.0f / (inv_effective_mass + 1.0e-24f);
	}

	inline float		GetInvEffectiveMass(Mat44Arg inInvI1, Mat44Arg inInvI2, Vec3Arg inWorldSpaceAxis)
	{
		if constexpr (Type1 == EMotionType::Dynamic)
			mInvI1_Axis = inInvI1.Multiply3x3(inWorldSpaceAxis);
		if constexpr (Type2 == EMotionType::Dynamic)
			mInvI2_Axis = inInvI2.Multiply3x3(inWorldSpaceAxis);

		float inv_effective_mass = 0.0f;
		if constexpr (Type1 == EMotionType::Dynamic && Type2 == EMotionType::Dynamic)
			inv_effective_mass = inWorldSpaceAxis.Dot(mInvI1_Axis + mInvI2_Axis);
		else if constexpr (Type1 == EMotionType::Dynamic)
			inv_effective_mass = inWorldSpaceAxis.Dot(mInvI1_Axis);
		else if constexpr (Type2 == EMotionType::Dynamic)
			inv_effective_mass = inWorldSpaceAxis.Dot(mInvI2_Axis);
		else
			JPH_ASSERT(false); // Static vs static is nonsensical!

		return inv_effective_mass + 1.0e-24f; // Prevent having to check for division by zero
	}

	static inline float	sGetMinusJV(Vec3Arg inAngularVelocity1, Vec3Arg inAngularVelocity2, Vec3Arg inWorldSpaceAxis)
	{
		float jv;
		if constexpr (Type1 != EMotionType::Static && Type2 != EMotionType::Static)
			jv = inWorldSpaceAxis.Dot(inAngularVelocity1 - inAngularVelocity2);
		else if constexpr (Type1 != EMotionType::Static)
			jv = inWorldSpaceAxis.Dot(inAngularVelocity1);
		else if constexpr (Type2 != EMotionType::Static)
			jv = -inWorldSpaceAxis.Dot(inAngularVelocity2);
		else
			JPH_ASSERT(false); // Static vs static is nonsensical!
		return jv;
	}

	static inline float	sGetMinusJV(const BodyState *inState1, const BodyState *inState2, Vec3Arg inWorldSpaceAxis)
	{
		float jv;
		if constexpr (Type1 != EMotionType::Static && Type2 != EMotionType::Static)
			jv = inWorldSpaceAxis.Dot(inState1->mAngularVelocity - inState2->mAngularVelocity);
		else if constexpr (Type1 != EMotionType::Static)
			jv = inWorldSpaceAxis.Dot(inState1->mAngularVelocity);
		else if constexpr (Type2 != EMotionType::Static)
			jv = -inWorldSpaceAxis.Dot(inState2->mAngularVelocity);
		else
			JPH_ASSERT(false); // Static vs static is nonsensical!
		return jv;
	}

	inline void			ApplyImpulse(Vec3 &ioAngularVelocity1, Vec3 &ioAngularVelocity2, float inLambda) const
	{
		if constexpr (Type1 == EMotionType::Dynamic)
			ioAngularVelocity1 -= inLambda * mInvI1_Axis;
		if constexpr (Type2 == EMotionType::Dynamic)
			ioAngularVelocity2 += inLambda * mInvI2_Axis;
	}

private:
	Vec3				mInvI1_Axis;
	Vec3				mInvI2_Axis;
};

JPH_NAMESPACE_END
