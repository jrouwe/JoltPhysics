// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Physics/PhysicsSettings.h>
#include <Physics/Body/Body.h>
#include <Physics/Constraints/ConstraintPart/SpringPart.h>
#include <Physics/StateRecorder.h>

namespace JPH {

/// Constraint that constrains motion along 1 axis
///
/// @see "Constraints Derivation for Rigid Body Simulation in 3D" - Daniel Chappuis, section 2.1.1 
/// (we're not using the approximation of eq 27 but instead add the U term as in eq 55)
///
/// Constraint equation (eq 25):
///
/// \f[C = (p_2 - p_1) \cdot n\f]
///
/// Jacobian (eq 28):
///
/// \f[J = \begin{bmatrix} -n^T & (-(r_1 + u) \times n)^T & n^T & (r_2 \times n)^T \end{bmatrix}\f]
///
/// Used terms (here and below, everything in world space):\n
/// n = constraint axis (normalized).\n
/// p1, p2 = constraint points.\n
/// r1 = p1 - x1.\n
/// r2 = p2 - x2.\n
/// u = x2 + r2 - x1 - r1 = p2 - p1.\n
/// x1, x2 = center of mass for the bodies.\n
/// v = [v1, w1, v2, w2].\n
/// v1, v2 = linear velocity of body 1 and 2.\n
/// w1, w2 = angular velocity of body 1 and 2.\n
/// M = mass matrix, a diagonal matrix of the mass and inertia with diagonal [m1, I1, m2, I2].\n
/// \f$K^{-1} = \left( J M^{-1} J^T \right)^{-1}\f$ = effective mass.\n
/// b = velocity bias.\n
/// \f$\beta\f$ = baumgarte constant.
class AxisConstraintPart
{
	/// Internal helper function to update velocities of bodies after Lagrange multiplier is calculated
	JPH_INLINE bool				ApplyVelocityStep(Body &ioBody1, Body &ioBody2, Vec3Arg inWorldSpaceAxis, float inLambda)
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
			if (ioBody1.IsDynamic())
			{
				MotionProperties *mp1 = ioBody1.GetMotionProperties();
				mp1->SubLinearVelocityStep((inLambda * mp1->GetInverseMass()) * inWorldSpaceAxis);
				mp1->SubAngularVelocityStep(inLambda * Vec3::sLoadFloat3Unsafe(mInvI1_R1PlusUxAxis));
			}
			if (ioBody2.IsDynamic())
			{
				MotionProperties *mp2 = ioBody2.GetMotionProperties();
				mp2->AddLinearVelocityStep((inLambda * mp2->GetInverseMass()) * inWorldSpaceAxis);
				mp2->AddAngularVelocityStep(inLambda * Vec3::sLoadFloat3Unsafe(mInvI2_R2xAxis));
			}
			return true;
		}

		return false;
	}

public:
	/// Calculate properties used during the functions below
	/// @param inDeltaTime Time step
	/// @param inBody1 The first body that this constraint is attached to
	/// @param inBody2 The second body that this constraint is attached to
	/// @param inR1PlusU See equations above (r1 + u)
	/// @param inR2 See equations above (r2)
	/// @param inWorldSpaceAxis Axis along which the constraint acts (normalized, pointing from body 1 to 2)
	/// @param inBias Bias term (b) for the constraint impulse: lambda = J v + b
	///	@param inC Value of the constraint equation (C). Set to zero if you don't want to drive the constraint to zero with a spring.
	///	@param inFrequency Oscillation frequency (Hz). Set to zero if you don't want to drive the constraint to zero with a spring.
	///	@param inDamping Damping factor (0 = no damping, 1 = critical damping). Set to zero if you don't want to drive the constraint to zero with a spring.
	inline void					CalculateConstraintProperties(float inDeltaTime, const Body &inBody1, Vec3Arg inR1PlusU, const Body &inBody2, Vec3Arg inR2, Vec3Arg inWorldSpaceAxis, float inBias = 0.0f, float inC = 0.0f, float inFrequency = 0.0f, float inDamping = 0.0f)
	{
		JPH_ASSERT(inWorldSpaceAxis.IsNormalized(1.0e-5f));

		// Calculate properties used below
		Vec3 r1_plus_u_x_axis = inR1PlusU.Cross(inWorldSpaceAxis);
		r1_plus_u_x_axis.StoreFloat3(&mR1PlusUxAxis);
		Vec3 r2_x_axis = inR2.Cross(inWorldSpaceAxis);
		r2_x_axis.StoreFloat3(&mR2xAxis);

		// Calculate inverse effective mass: K = J M^-1 J^T
		float inv_effective_mass;

		if (inBody1.IsDynamic())
		{
			const MotionProperties *mp1 = inBody1.GetMotionProperties();
			Vec3 invi1_r1_plus_u_x_axis = mp1->MultiplyWorldSpaceInverseInertiaByVector(inBody1.GetRotation(), r1_plus_u_x_axis);
			invi1_r1_plus_u_x_axis.StoreFloat3(&mInvI1_R1PlusUxAxis);
			inv_effective_mass = mp1->GetInverseMass() + invi1_r1_plus_u_x_axis.Dot(r1_plus_u_x_axis);
		}
		else
		{
			JPH_IF_DEBUG(Vec3::sNaN().StoreFloat3(&mInvI1_R1PlusUxAxis);)
			inv_effective_mass = 0.0f;
		}

		if (inBody2.IsDynamic())
		{
			const MotionProperties *mp2 = inBody2.GetMotionProperties();
			Vec3 invi2_r2_x_axis = mp2->MultiplyWorldSpaceInverseInertiaByVector(inBody2.GetRotation(), r2_x_axis);
			invi2_r2_x_axis.StoreFloat3(&mInvI2_R2xAxis);
			inv_effective_mass += mp2->GetInverseMass() + invi2_r2_x_axis.Dot(r2_x_axis);
		}
		else
		{
			JPH_IF_DEBUG(Vec3::sNaN().StoreFloat3(&mInvI2_R2xAxis);)
		}

		// Calculate effective mass and spring properties
		mSpringPart.CalculateSpringProperties(inDeltaTime, inv_effective_mass, inBias, inC, inFrequency, inDamping, mEffectiveMass);
	}

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

	/// Must be called from the WarmStartVelocityConstraint call to apply the previous frame's impulses
	/// @param ioBody1 The first body that this constraint is attached to
	/// @param ioBody2 The second body that this constraint is attached to
	/// @param inWorldSpaceAxis Axis along which the constraint acts (normalized)
	/// @param inWarmStartImpulseRatio Ratio of new step to old time step (dt_new / dt_old) for scaling the lagrange multiplier of the previous frame
	inline void					WarmStart(Body &ioBody1, Body &ioBody2, Vec3Arg inWorldSpaceAxis, float inWarmStartImpulseRatio)
	{
		mTotalLambda *= inWarmStartImpulseRatio;
		ApplyVelocityStep(ioBody1, ioBody2, inWorldSpaceAxis, mTotalLambda);
	}

	/// Iteratively update the velocity constraint. Makes sure d/dt C(...) = 0, where C is the constraint equation.
	/// @param ioBody1 The first body that this constraint is attached to
	/// @param ioBody2 The second body that this constraint is attached to
	/// @param inWorldSpaceAxis Axis along which the constraint acts (normalized)
	/// @param inMinLambda Minimum value of constraint impulse to apply (N s)
	/// @param inMaxLambda Maximum value of constraint impulse to apply (N s)
	inline bool					SolveVelocityConstraint(Body &ioBody1, Body &ioBody2, Vec3Arg inWorldSpaceAxis, float inMinLambda, float inMaxLambda)
	{
		// Lagrange multiplier is:
		//
		// lambda = -K^-1 (J v + b)
		float lambda = mEffectiveMass * (inWorldSpaceAxis.Dot(ioBody1.GetLinearVelocity() - ioBody2.GetLinearVelocity()) + Vec3::sLoadFloat3Unsafe(mR1PlusUxAxis).Dot(ioBody1.GetAngularVelocity()) - Vec3::sLoadFloat3Unsafe(mR2xAxis).Dot(ioBody2.GetAngularVelocity()) - mSpringPart.GetBias(mTotalLambda));
		float new_lambda = Clamp(mTotalLambda + lambda, inMinLambda, inMaxLambda); // Clamp impulse
		lambda = new_lambda - mTotalLambda; // Lambda potentially got clamped, calculate the new impulse to apply
		mTotalLambda = new_lambda; // Store accumulated impulse

		return ApplyVelocityStep(ioBody1, ioBody2, inWorldSpaceAxis, lambda);
	}

	/// Iteratively update the position constraint. Makes sure C(...) = 0.
	/// @param ioBody1 The first body that this constraint is attached to
	/// @param ioBody2 The second body that this constraint is attached to
	/// @param inWorldSpaceAxis Axis along which the constraint acts (normalized)
	/// @param inC Value of the constraint equation (C)
	/// @param inBaumgarte Baumgarte constant (fraction of the error to correct)
	inline bool					SolvePositionConstraint(Body &ioBody1, Body &ioBody2, Vec3Arg inWorldSpaceAxis, float inC, float inBaumgarte)
	{
		// Only apply position constraint when the constraint is hard, otherwise the velocity bias will fix the constraint
		if (inC != 0.0f && !mSpringPart.IsActive())
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
			if (ioBody1.IsDynamic())
			{
				ioBody1.SubPositionStep((lambda * ioBody1.GetMotionProperties()->GetInverseMass()) * inWorldSpaceAxis);
				ioBody1.SubRotationStep(lambda * Vec3::sLoadFloat3Unsafe(mInvI1_R1PlusUxAxis));
			}
			if (ioBody2.IsDynamic())
			{
				ioBody2.AddPositionStep((lambda * ioBody2.GetMotionProperties()->GetInverseMass()) * inWorldSpaceAxis);
				ioBody2.AddRotationStep(lambda * Vec3::sLoadFloat3Unsafe(mInvI2_R2xAxis));
			}
			return true;
		}

		return false;
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

	/// Save state of this constraint part
	void						SaveState(StateRecorder &inStream) const
	{
		inStream.Write(mTotalLambda);
	}

	/// Restore state of this constraint part
	void						RestoreState(StateRecorder &inStream)
	{
		inStream.Read(mTotalLambda);
	}

private:
	Float3						mR1PlusUxAxis;
	Float3						mR2xAxis;
	Float3						mInvI1_R1PlusUxAxis;
	Float3						mInvI2_R2xAxis;
	float						mEffectiveMass = 0.0f;
	SpringPart					mSpringPart;
	float						mTotalLambda = 0.0f;
};

} // JPH