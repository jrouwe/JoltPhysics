// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Physics/Body/Body.h>

JPH_NAMESPACE_BEGIN

class ContactConstraintPartShared
{
public:
	/// Deactivate this constraint
	inline void					Deactivate()
	{
		mEffectiveMass = 0.0f;
		mTotalLambda = 0.0f;
	}

	/// Check if constraint is active
	inline bool					IsActive() const
	{
		return mEffectiveMass != 0.0f;
	}

	/// Set the inverse of the effective mass
	inline void					SetInvEffectiveMass(float inInvEffectiveMass, float inBias)
	{
		if (inInvEffectiveMass == 0.0f)
			Deactivate();
		else
		{
			mEffectiveMass = 1.0f / inInvEffectiveMass;
			mBias = inBias;
		}
	}

	/// Override total lagrange multiplier, can be used to set the initial value for warm starting
	inline void					SetTotalLambda(float inLambda)
	{
		mTotalLambda = inLambda;
	}

	/// Return lagrange multiplier
	inline float				GetTotalLambda() const
	{
		return mTotalLambda;
	}

protected:
	float						mEffectiveMass = 0.0f;
	float						mTotalLambda = 0.0f;
	float						mBias;
};

/// This is a copy of AxisConstraintPart, specialized to handle contact constraints. See the documentation of AxisConstraintPart for more documentation behind the math.
template <EMotionType Type1, EMotionType Type2>
class TemplatedContactConstraintPart : public ContactConstraintPartShared
{
private:
	/// Internal helper function to update velocities of bodies after Lagrange multiplier is calculated
	JPH_INLINE bool				ApplyVelocityStep(MotionProperties *ioMotionProperties1, float inInvMass1, MotionProperties *ioMotionProperties2, float inInvMass2, Vec3Arg inWorldSpaceAxis, float inLambda) const
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
			if constexpr (Type1 == EMotionType::Dynamic)
			{
				ioMotionProperties1->SubLinearVelocityStep((inLambda * inInvMass1) * inWorldSpaceAxis);
				ioMotionProperties1->SubAngularVelocityStep(inLambda * Vec3::sLoadFloat3Unsafe(mInvI1_R1PlusUxAxis));
			}
			if constexpr (Type2 == EMotionType::Dynamic)
			{
				ioMotionProperties2->AddLinearVelocityStep((inLambda * inInvMass2) * inWorldSpaceAxis);
				ioMotionProperties2->AddAngularVelocityStep(inLambda * Vec3::sLoadFloat3Unsafe(mInvI2_R2xAxis));
			}
			return true;
		}

		return false;
	}

public:
	/// Internal helper function to calculate the inverse effective mass
	JPH_INLINE float			CalculateInverseEffectiveMass(float inInvMass1, Mat44Arg inInvI1, Vec3Arg inR1PlusU, float inInvMass2, Mat44Arg inInvI2, Vec3Arg inR2, Vec3Arg inWorldSpaceAxis)
	{
		JPH_ASSERT(inWorldSpaceAxis.IsNormalized(1.0e-5f));
				
		// Calculate inverse effective mass: K = J M^-1 J^T
		float inv_effective_mass;

		if constexpr (Type1 == EMotionType::Dynamic)
		{
			Vec3 r1_plus_u_x_axis = inR1PlusU.Cross(inWorldSpaceAxis);
			r1_plus_u_x_axis.StoreFloat3(&mR1PlusUxAxis);

			Vec3 invi1_r1_plus_u_x_axis = inInvI1.Multiply3x3(r1_plus_u_x_axis);
			invi1_r1_plus_u_x_axis.StoreFloat3(&mInvI1_R1PlusUxAxis);

			inv_effective_mass = inInvMass1 + invi1_r1_plus_u_x_axis.Dot(r1_plus_u_x_axis);
		}
		else
		{
			JPH_IF_DEBUG(Vec3::sNaN().StoreFloat3(&mR1PlusUxAxis);)
			JPH_IF_DEBUG(Vec3::sNaN().StoreFloat3(&mInvI1_R1PlusUxAxis);)

			inv_effective_mass = 0.0f;
		}

		if constexpr (Type2 == EMotionType::Dynamic)
		{
			Vec3 r2_x_axis = inR2.Cross(inWorldSpaceAxis);
			r2_x_axis.StoreFloat3(&mR2xAxis);

			Vec3 invi2_r2_x_axis = inInvI2.Multiply3x3(r2_x_axis);
			invi2_r2_x_axis.StoreFloat3(&mInvI2_R2xAxis);

			inv_effective_mass += inInvMass2 + invi2_r2_x_axis.Dot(r2_x_axis);
		}
		else
		{
			JPH_IF_DEBUG(Vec3::sNaN().StoreFloat3(&mR2xAxis);)
			JPH_IF_DEBUG(Vec3::sNaN().StoreFloat3(&mInvI2_R2xAxis);)
		}

		return inv_effective_mass;
	}

public:
	/// Templated form of CalculateConstraintProperties with the motion types baked in
	JPH_INLINE void				CalculateConstraintProperties(float inInvMass1, Mat44Arg inInvI1, Vec3Arg inR1PlusU, float inInvMass2, Mat44Arg inInvI2, Vec3Arg inR2, Vec3Arg inWorldSpaceAxis, float inBias = 0.0f)
	{
		float inv_effective_mass = CalculateInverseEffectiveMass(inInvMass1, inInvI1, inR1PlusU, inInvMass2, inInvI2, inR2, inWorldSpaceAxis);

		SetInvEffectiveMass(inv_effective_mass, inBias);
	}

	/// Templated form of WarmStart with the motion types baked in
	inline void					WarmStart(MotionProperties *ioMotionProperties1, float inInvMass1, MotionProperties *ioMotionProperties2, float inInvMass2, Vec3Arg inWorldSpaceAxis, float inWarmStartImpulseRatio)
	{
		mTotalLambda *= inWarmStartImpulseRatio;

		ApplyVelocityStep(ioMotionProperties1, inInvMass1, ioMotionProperties2, inInvMass2, inWorldSpaceAxis, mTotalLambda);
	}

	/// Templated form of SolveVelocityConstraint with the motion types baked in, part 1: get the total lambda
	JPH_INLINE float			SolveVelocityConstraintGetTotalLambda(const MotionProperties *ioMotionProperties1, const MotionProperties *ioMotionProperties2, Vec3Arg inWorldSpaceAxis) const
	{
		// Calculate jacobian multiplied by linear velocity
		float jv;
		if constexpr (Type1 != EMotionType::Static && Type2 != EMotionType::Static)
			jv = inWorldSpaceAxis.Dot(ioMotionProperties1->GetLinearVelocity() - ioMotionProperties2->GetLinearVelocity());
		else if constexpr (Type1 != EMotionType::Static)
			jv = inWorldSpaceAxis.Dot(ioMotionProperties1->GetLinearVelocity());
		else if constexpr (Type2 != EMotionType::Static)
			jv = inWorldSpaceAxis.Dot(-ioMotionProperties2->GetLinearVelocity());
		else
			JPH_ASSERT(false); // Static vs static is nonsensical!

		// Calculate jacobian multiplied by angular velocity
		if constexpr (Type1 != EMotionType::Static)
			jv += Vec3::sLoadFloat3Unsafe(mR1PlusUxAxis).Dot(ioMotionProperties1->GetAngularVelocity());
		if constexpr (Type2 != EMotionType::Static)
			jv -= Vec3::sLoadFloat3Unsafe(mR2xAxis).Dot(ioMotionProperties2->GetAngularVelocity());

		// Lagrange multiplier is:
		//
		// lambda = -K^-1 (J v + b)
		float lambda = mEffectiveMass * (jv - mBias);

		// Return the total accumulated lambda
		return mTotalLambda + lambda;
	}

	/// Templated form of SolveVelocityConstraint with the motion types baked in, part 2: apply new lambda
	JPH_INLINE bool				SolveVelocityConstraintApplyLambda(MotionProperties *ioMotionProperties1, float inInvMass1, MotionProperties *ioMotionProperties2, float inInvMass2, Vec3Arg inWorldSpaceAxis, float inTotalLambda)
	{
		float delta_lambda = inTotalLambda - mTotalLambda; // Calculate change in lambda
		mTotalLambda = inTotalLambda; // Store accumulated impulse

		return ApplyVelocityStep(ioMotionProperties1, inInvMass1, ioMotionProperties2, inInvMass2, inWorldSpaceAxis, delta_lambda);
	}

	/// Templated form of SolveVelocityConstraint with the motion types baked in
	inline bool					SolveVelocityConstraint(MotionProperties *ioMotionProperties1, float inInvMass1, MotionProperties *ioMotionProperties2, float inInvMass2, Vec3Arg inWorldSpaceAxis, float inMinLambda, float inMaxLambda)
	{
		float total_lambda = SolveVelocityConstraintGetTotalLambda(ioMotionProperties1, ioMotionProperties2, inWorldSpaceAxis);

		// Clamp impulse to specified range
		total_lambda = Clamp(total_lambda, inMinLambda, inMaxLambda);

		return SolveVelocityConstraintApplyLambda(ioMotionProperties1, inInvMass1, ioMotionProperties2, inInvMass2, inWorldSpaceAxis, total_lambda);
	}

	/// Iteratively update the position constraint. Makes sure C(...) = 0.
	/// @param ioBody1 The first body that this constraint is attached to
	/// @param ioBody2 The second body that this constraint is attached to
	/// @param inInvMass1 The inverse mass of body 1 (only used when body 1 is dynamic)
	/// @param inInvMass2 The inverse mass of body 2 (only used when body 2 is dynamic)
	/// @param inWorldSpaceAxis Axis along which the constraint acts (normalized)
	/// @param inC Value of the constraint equation (C)
	/// @param inBaumgarte Baumgarte constant (fraction of the error to correct)
	inline bool					SolvePositionConstraint(Body &ioBody1, float inInvMass1, Body &ioBody2, float inInvMass2, Vec3Arg inWorldSpaceAxis, float inC, float inBaumgarte) const
	{
		// Only apply position constraint when the constraint is hard, otherwise the velocity bias will fix the constraint
		if (inC != 0.0f)
		{
			// Calculate lagrange multiplier (lambda) for Baumgarte stabilization:
			//
			// lambda = -K^-1 * beta / dt * C
			//
			// We should divide by inDeltaTime, but we should multiply by inDeltaTime in the Euler step below so they're cancelled out
			float lambda = -mEffectiveMass * inBaumgarte * inC;

			// Directly integrate velocity change for one time step
			//
			// Euler velocity integration:
			// dv = M^-1 P
			//
			// Impulse:
			// P = J^T lambda
			//
			// Euler position integration:
			// x' = x + dv * dt
			//
			// Note we don't accumulate velocities for the stabilization. This is using the approach described in 'Modeling and
			// Solving Constraints' by Erin Catto presented at GDC 2007. On slide 78 it is suggested to split up the Baumgarte
			// stabilization for positional drift so that it does not actually add to the momentum. We combine an Euler velocity
			// integrate + a position integrate and then discard the velocity change.
			if (Type1 == EMotionType::Dynamic)
			{
				ioBody1.SubPositionStep((lambda * inInvMass1) * inWorldSpaceAxis);
				ioBody1.SubRotationStep(lambda * Vec3::sLoadFloat3Unsafe(mInvI1_R1PlusUxAxis));
			}
			if (Type2 == EMotionType::Dynamic)
			{
				ioBody2.AddPositionStep((lambda * inInvMass2) * inWorldSpaceAxis);
				ioBody2.AddRotationStep(lambda * Vec3::sLoadFloat3Unsafe(mInvI2_R2xAxis));
			}
			return true;
		}

		return false;
	}

private:
	Float3						mR1PlusUxAxis;
	Float3						mR2xAxis;
	Float3						mInvI1_R1PlusUxAxis;
	Float3						mInvI2_R2xAxis;
};

/// Concrete contact constraint part that dispatches to the correct templated form based on the motion types of the bodies
class ContactConstraintPart
{
public:
	/// Constructor / destructor
								ContactConstraintPart() : mShared() { }
								~ContactConstraintPart() { }

	inline void					CalculateConstraintProperties(const Body &inBody1, float inInvMass1, float inInvInertiaScale1, Vec3Arg inR1PlusU, const Body &inBody2, float inInvMass2, float inInvInertiaScale2, Vec3Arg inR2, Vec3Arg inWorldSpaceAxis, float inBias = 0.0f)
	{
		float inv_effective_mass = 0.0f;

		// Dispatch to the correct templated form
		switch (inBody1.GetMotionType())
		{
		case EMotionType::Dynamic:
			{
				Mat44 inv_i1 = inInvInertiaScale1 * inBody1.GetInverseInertia();
				switch (inBody2.GetMotionType())
				{
				case EMotionType::Dynamic:
					inv_effective_mass = mDD.CalculateInverseEffectiveMass(inInvMass1, inv_i1, inR1PlusU, inInvMass2, inInvInertiaScale2 * inBody2.GetInverseInertia(), inR2, inWorldSpaceAxis);
					break;

				case EMotionType::Kinematic:
					inv_effective_mass = mDK.CalculateInverseEffectiveMass(inInvMass1, inv_i1, inR1PlusU, 0 /* Will not be used */, Mat44() /* Will not be used */, inR2, inWorldSpaceAxis);
					break;

				case EMotionType::Static:
					inv_effective_mass = mDS.CalculateInverseEffectiveMass(inInvMass1, inv_i1, inR1PlusU, 0 /* Will not be used */, Mat44() /* Will not be used */, inR2, inWorldSpaceAxis);
					break;

				default:
					JPH_ASSERT(false);
					break;
				}
				break;
			}

		case EMotionType::Kinematic:
			JPH_ASSERT(inBody2.IsDynamic());
			inv_effective_mass = mKD.CalculateInverseEffectiveMass(0 /* Will not be used */, Mat44() /* Will not be used */, inR1PlusU, inInvMass2, inInvInertiaScale2 * inBody2.GetInverseInertia(), inR2, inWorldSpaceAxis);
			break;

		case EMotionType::Static:
			JPH_ASSERT(inBody2.IsDynamic());
			inv_effective_mass = mSD.CalculateInverseEffectiveMass(0 /* Will not be used */, Mat44() /* Will not be used */, inR1PlusU, inInvMass2, inInvInertiaScale2 * inBody2.GetInverseInertia(), inR2, inWorldSpaceAxis);
			break;

		default:
			JPH_ASSERT(false);
			break;
		}

		mShared.SetInvEffectiveMass(inv_effective_mass, inBias);
	}

	inline bool					SolveVelocityConstraint(Body &ioBody1, float inInvMass1, Body &ioBody2, float inInvMass2, Vec3Arg inWorldSpaceAxis, float inMinLambda, float inMaxLambda)
	{
		EMotionType motion_type1 = ioBody1.GetMotionType();
		MotionProperties *motion_properties1 = ioBody1.GetMotionPropertiesUnchecked();

		EMotionType motion_type2 = ioBody2.GetMotionType();
		MotionProperties *motion_properties2 = ioBody2.GetMotionPropertiesUnchecked();

		// Dispatch to the correct templated form
		switch (motion_type1)
		{
		case EMotionType::Dynamic:
			switch (motion_type2)
			{
			case EMotionType::Dynamic:
				return mDD.SolveVelocityConstraint(motion_properties1, inInvMass1, motion_properties2, inInvMass2, inWorldSpaceAxis, inMinLambda, inMaxLambda);

			case EMotionType::Kinematic:
				return mDK.SolveVelocityConstraint(motion_properties1, inInvMass1, motion_properties2, 0.0f /* Unused */, inWorldSpaceAxis, inMinLambda, inMaxLambda);

			case EMotionType::Static:
				return mDS.SolveVelocityConstraint(motion_properties1, inInvMass1, motion_properties2, 0.0f /* Unused */, inWorldSpaceAxis, inMinLambda, inMaxLambda);

			default:
				JPH_ASSERT(false);
				break;
			}
			break;

		case EMotionType::Kinematic:
			JPH_ASSERT(motion_type2 == EMotionType::Dynamic);
			return mKD.SolveVelocityConstraint(motion_properties1, 0.0f /* Unused */, motion_properties2, inInvMass2, inWorldSpaceAxis, inMinLambda, inMaxLambda);

		case EMotionType::Static:
			JPH_ASSERT(motion_type2 == EMotionType::Dynamic);
			return mSD.SolveVelocityConstraint(motion_properties1, 0.0f /* Unused */, motion_properties2, inInvMass2, inWorldSpaceAxis, inMinLambda, inMaxLambda);

		default:
			JPH_ASSERT(false);
			break;
		}

		return false;
	}

	inline bool					SolvePositionConstraint(Body &ioBody1, float inInvMass1, Body &ioBody2, float inInvMass2, Vec3Arg inWorldSpaceAxis, float inC, float inBaumgarte) const
	{
		EMotionType motion_type1 = ioBody1.GetMotionType();
		EMotionType motion_type2 = ioBody2.GetMotionType();

		// Dispatch to the correct templated form
		switch (motion_type1)
		{
		case EMotionType::Dynamic:
			switch (motion_type2)
			{
			case EMotionType::Dynamic:
				return mDD.SolvePositionConstraint(ioBody1, inInvMass1, ioBody2, inInvMass2, inWorldSpaceAxis, inC, inBaumgarte);

			case EMotionType::Kinematic:
				return mDK.SolvePositionConstraint(ioBody1, inInvMass1, ioBody2, 0.0f /* Unused */, inWorldSpaceAxis, inC, inBaumgarte);

			case EMotionType::Static:
				return mDS.SolvePositionConstraint(ioBody1, inInvMass1, ioBody2, 0.0f /* Unused */, inWorldSpaceAxis, inC, inBaumgarte);

			default:
				JPH_ASSERT(false);
				break;
			}
			break;

		case EMotionType::Kinematic:
			JPH_ASSERT(motion_type2 == EMotionType::Dynamic);
			return mKD.SolvePositionConstraint(ioBody1, 0.0f /* Unused */, ioBody2, inInvMass2, inWorldSpaceAxis, inC, inBaumgarte);

		case EMotionType::Static:
			JPH_ASSERT(motion_type2 == EMotionType::Dynamic);
			return mSD.SolvePositionConstraint(ioBody1, 0.0f /* Unused */, ioBody2, inInvMass2, inWorldSpaceAxis, inC, inBaumgarte);

		default:
			JPH_ASSERT(false);
			break;
		}

		return false;
	}

	inline void					Deactivate()
	{
		mShared.Deactivate();
	}

	inline bool					IsActive() const
	{
		return mShared.IsActive();
	}

	inline void					SetTotalLambda(float inLambda)
	{
		mShared.SetTotalLambda(inLambda);
	}

	inline float				GetTotalLambda() const
	{
		return mShared.GetTotalLambda();
	}

private:
	union
	{
		ContactConstraintPartShared														mShared;
		TemplatedContactConstraintPart<EMotionType::Dynamic, EMotionType::Dynamic>		mDD;
		TemplatedContactConstraintPart<EMotionType::Dynamic, EMotionType::Kinematic>	mDK;
		TemplatedContactConstraintPart<EMotionType::Dynamic, EMotionType::Static>		mDS;
		TemplatedContactConstraintPart<EMotionType::Kinematic, EMotionType::Dynamic>	mKD;
		TemplatedContactConstraintPart<EMotionType::Static, EMotionType::Dynamic>		mSD;
	};
};

JPH_NAMESPACE_END
