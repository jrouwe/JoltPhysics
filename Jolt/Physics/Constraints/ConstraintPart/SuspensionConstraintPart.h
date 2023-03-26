// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Constraints/ConstraintPart/SpringPart.h>
#include <Jolt/Physics/StateRecorder.h>

JPH_NAMESPACE_BEGIN

/// Constraint that constrains motion along 1 axis with a spring that operates along another axis
/// This class is heavily based on AxisConstraintPart, see that class for more details on the math.
///
/// The suspension forces look like this:
///
/// S \     Fc
///    \  _-^
/// Fc1 ^-  |\
///      \  | \
///       \ |  \
///  alpha \| _-> Fc2
///         +-
///
/// S = Suspension direction (top left to bottom right)
/// Fc = Contact force from ground on wheel pushing the suspension up (along the contact normal, pointing up)
/// Fc1 = Component of Fc along S
/// Fc2 = Component of Fc perpendicular to S
/// alpha = angle between S and Fc2
///
/// From the image we can see that:
///
/// Fc1 = cos(angle) Fc
/// Fc2 = sin(angle) Fc
///
/// We ignore parts of Fc1 and Fc2 that are perpendicular to the contact normal (we only apply tire friction in that direction):
///
/// S \
///    \   Fp1
/// Fc1 ^---^
///      \  |
///       \ |
///    Fp2 \^-_-> Fc2
///         +-
///
/// Fp1 = Fc1 projected back on the contact normal
/// Fp2 = Fc2 projected back on the contact normal
///
/// From the image we can see that:
///
/// Fp1 = cos(angle) Fc1
/// Fp2 = sin(angle) Fc2
///
/// So:
///
/// Fp1 = cos(angle)^2 Fc
/// Fp2 = sin(angle)^2 Fc
///
/// To verify:
///
/// Fp1 + Fp2 = cos(angle)^2 Fc + sin(angle)^2 Fc = Fc (using the identity sin(angle)^2 + cos(angle)^2 = 1)
///
/// So if we calculate the normal impulse along the contact normal, we need to multiply by cos(angle)^2 in order to get the correct impulse.
/// For the parallel impulse we need to multiply by sin(angle)^2.
///
/// We combine both of these forces in this class.
class SuspensionConstraintPart
{
	/// Internal helper function to update velocities of bodies after Lagrange multiplier is calculated
	JPH_INLINE bool				ApplyVelocityStep(EMotionType inMotionType1, MotionProperties *ioMotionProperties1, EMotionType inMotionType2, MotionProperties *ioMotionProperties2, Vec3Arg inWorldSpaceAxis, float inLambda) const
	{
		// Apply impulse if delta is not zero
		if (inLambda != 0.0f)
		{
			// Calculate velocity change due to constraint
			//
			// Impulse:
			// P = J^T lambda
			//
			// Euler velocity integration: 
			// v' = v + M^-1 P
			if (inMotionType1 == EMotionType::Dynamic)
			{
				ioMotionProperties1->SubLinearVelocityStep((inLambda * ioMotionProperties1->GetInverseMass()) * inWorldSpaceAxis);
				ioMotionProperties1->SubAngularVelocityStep(inLambda * mInvI1_R1PlusUxAxis);
			}
			if (inMotionType2 == EMotionType::Dynamic)
			{
				ioMotionProperties2->AddLinearVelocityStep((inLambda * ioMotionProperties2->GetInverseMass()) * inWorldSpaceAxis);
				ioMotionProperties2->AddAngularVelocityStep(inLambda * mInvI2_R2xAxis);
			}
			return true;
		}

		return false;
	}

public:
	/// See: AxisConstraintPart::CalculateConstraintProperties
	/// Extra parameter inWorldSpaceSuspensionAxis Axis along which the suspension acts (normalized)
	inline void					CalculateConstraintProperties(float inDeltaTime, const Body &inBody1, Vec3Arg inR1PlusU, const Body &inBody2, Vec3Arg inR2, Vec3Arg inWorldSpaceAxis, Vec3Arg inWorldSpaceSuspensionAxis, float inBias = 0.0f, float inC = 0.0f, float inFrequency = 0.0f, float inDamping = 0.0f)
	{
		JPH_ASSERT(inWorldSpaceAxis.IsNormalized(1.0e-5f));

		// Calculate properties used below
		mR1PlusUxAxis = inR1PlusU.Cross(inWorldSpaceAxis);
		mR2xAxis = inR2.Cross(inWorldSpaceAxis);

		// Calculate inverse effective mass: K = J M^-1 J^T
		float inv_effective_mass;
		if (inBody1.GetMotionType() == EMotionType::Dynamic)
		{
			const MotionProperties *motion_properties1 = inBody1.GetMotionPropertiesUnchecked();
			mInvI1_R1PlusUxAxis = motion_properties1->MultiplyWorldSpaceInverseInertiaByVector(inBody1.GetRotation(), mR1PlusUxAxis);
			inv_effective_mass = motion_properties1->GetInverseMass() + mInvI1_R1PlusUxAxis.Dot(mR1PlusUxAxis);
		}
		else
		{
			JPH_IF_DEBUG(mInvI1_R1PlusUxAxis = Vec3::sNaN();)
			inv_effective_mass = 0.0f;
		}

		if (inBody2.GetMotionType() == EMotionType::Dynamic)
		{
			const MotionProperties *motion_properties2 = inBody2.GetMotionPropertiesUnchecked();
			mInvI2_R2xAxis = motion_properties2->MultiplyWorldSpaceInverseInertiaByVector(inBody2.GetRotation(), mR2xAxis);
			inv_effective_mass += motion_properties2->GetInverseMass() + mInvI2_R2xAxis.Dot(mR2xAxis);
		}
		else
		{
			JPH_IF_DEBUG(mInvI2_R2xAxis = Vec3::sNaN();)
		}

		// Calculate effective mass and spring properties
		mEffectiveMassPerpendicular = 1.0f / inv_effective_mass;
		mSpringPartParallel.CalculateSpringProperties(inDeltaTime, inv_effective_mass, inBias, inC, inFrequency, inDamping, mEffectiveMassParallel);

		// Scale the effective masses according to the angle betweeen contact normal and suspension,
		// this is the main multiplier to calculate the lambdas so we're effectively scaling the lambdas here.
		float cos_angle_sq = Square(inWorldSpaceAxis.Dot(inWorldSpaceSuspensionAxis));
		mEffectiveMassParallel *= cos_angle_sq;
		mEffectiveMassPerpendicular *= (1.0f - cos_angle_sq);
	}

	/// Deactivate this constraint
	inline void					Deactivate()
	{
		mEffectiveMassParallel = 0.0f;
		mTotalLambdaParallel = 0.0f;

		mEffectiveMassPerpendicular = 0.0f;
		mTotalLambdaPerpendicular = 0.0f;
	}

	/// Check if constraint is active
	inline bool					IsActive() const
	{
		return mEffectiveMassParallel != 0.0f || mEffectiveMassPerpendicular != 0.0f;
	}

	/// See AxisConstraint::WarmStart
	inline void					WarmStart(Body &ioBody1, Body &ioBody2, Vec3Arg inWorldSpaceAxis, float inWarmStartImpulseRatio)
	{
		mTotalLambdaParallel *= inWarmStartImpulseRatio;
		mTotalLambdaPerpendicular *= inWarmStartImpulseRatio;

		ApplyVelocityStep(ioBody1.GetMotionType(), ioBody1.GetMotionPropertiesUnchecked(), ioBody2.GetMotionType(), ioBody2.GetMotionPropertiesUnchecked(), inWorldSpaceAxis, GetTotalLambda());
	}

	/// See AxisConstraint::SolveVelocityConstraint
	inline bool					SolveVelocityConstraint(Body &ioBody1, Body &ioBody2, Vec3Arg inWorldSpaceAxis)
	{
		EMotionType motion_type1 = ioBody1.GetMotionType();
		MotionProperties *motion_properties1 = ioBody1.GetMotionPropertiesUnchecked();

		EMotionType motion_type2 = ioBody2.GetMotionType();
		MotionProperties *motion_properties2 = ioBody2.GetMotionPropertiesUnchecked();

		// Calculate jacobian multiplied by linear/angular velocity
		float jv = 0.0f;
		if (motion_type1 != EMotionType::Static)
			jv = inWorldSpaceAxis.Dot(motion_properties1->GetLinearVelocity()) + mR1PlusUxAxis.Dot(motion_properties1->GetAngularVelocity());
		if (motion_type2 != EMotionType::Static)
			jv -= inWorldSpaceAxis.Dot(motion_properties2->GetLinearVelocity()) + mR2xAxis.Dot(motion_properties2->GetAngularVelocity());

		// Lagrange multiplier for the spring
		float lambda_parallel = mEffectiveMassParallel * (jv - mSpringPartParallel.GetBias(mTotalLambdaParallel));
		float new_lambda_parallel = max(mTotalLambdaParallel + lambda_parallel, 0.0f); // Only push, don't pull
		lambda_parallel = new_lambda_parallel - mTotalLambdaParallel; // Lambda could have been clamped, recalculate
		mTotalLambdaParallel = new_lambda_parallel;

		// Lagrange multiplier for the infinitely stiff spring perpendicular to the spring
		float lambda_perpendicular = mEffectiveMassPerpendicular * jv;
		float new_lambda_perpendicular = max(mTotalLambdaPerpendicular + lambda_perpendicular, 0.0f);
		lambda_perpendicular = new_lambda_perpendicular - mTotalLambdaPerpendicular;
		mTotalLambdaPerpendicular = new_lambda_perpendicular;

		return ApplyVelocityStep(motion_type1, motion_properties1, motion_type2, motion_properties2, inWorldSpaceAxis, lambda_parallel + lambda_perpendicular);
	}

	/// Return lagrange multiplier
	inline float				GetTotalLambda() const
	{
		return mTotalLambdaParallel + mTotalLambdaPerpendicular;
	}

	/// Save state of this constraint part
	void						SaveState(StateRecorder &inStream) const
	{
		inStream.Write(mTotalLambdaParallel);
		inStream.Write(mTotalLambdaPerpendicular);
	}

	/// Restore state of this constraint part
	void						RestoreState(StateRecorder &inStream)
	{
		inStream.Read(mTotalLambdaParallel);
		inStream.Read(mTotalLambdaPerpendicular);
	}

private:
	Vec3						mR1PlusUxAxis;
	Vec3						mR2xAxis;
	Vec3						mInvI1_R1PlusUxAxis;
	Vec3						mInvI2_R2xAxis;

	// Properties along the spring direction
	float						mEffectiveMassParallel = 0.0f;
	SpringPart					mSpringPartParallel;
	float						mTotalLambdaParallel = 0.0f;

	// Properties perpendicular to the spring direction
	float						mEffectiveMassPerpendicular = 0.0f;
	float						mTotalLambdaPerpendicular = 0.0f;
};

JPH_NAMESPACE_END
