// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Physics/Body/Body.h>

JPH_NAMESPACE_BEGIN

/// Decide which members this constraint part needs based on motion type
template <EMotionType Type1>
class ContactConstraintPart1
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
	// Note: Constructor will not be called
	float						mEffectiveMass;
	float						mTotalLambda;
	float						mBias;
};

template <>
class ContactConstraintPart1<EMotionType::Kinematic> : public ContactConstraintPart1<EMotionType::Static>
{
protected:
	// Note: Constructor will not be called
	Float3						mR1PlusUxAxis;
};

template <>
class ContactConstraintPart1<EMotionType::Dynamic> : public ContactConstraintPart1<EMotionType::Kinematic>
{
protected:
	// Note: Constructor will not be called
	Float3						mInvI1_R1PlusUxAxis;
};

template <EMotionType Type2>
class ContactConstraintPart2
{
};

template <>
class ContactConstraintPart2<EMotionType::Kinematic> : public ContactConstraintPart2<EMotionType::Static>
{
protected:
	// Note: Constructor will not be called
	Float3						mR2xAxis;
};

template <>
class ContactConstraintPart2<EMotionType::Dynamic> : public ContactConstraintPart2<EMotionType::Kinematic>
{
protected:
	// Note: Constructor will not be called
	Float3						mInvI2_R2xAxis;
};

static_assert(sizeof(ContactConstraintPart1<EMotionType::Kinematic>) == 3 * sizeof(float) + sizeof(Float3));
static_assert(sizeof(ContactConstraintPart1<EMotionType::Dynamic>) == 3 * sizeof(float) + 2 * sizeof(Float3));
static_assert(sizeof(ContactConstraintPart2<EMotionType::Kinematic>) == sizeof(Float3));
static_assert(sizeof(ContactConstraintPart2<EMotionType::Dynamic>) == 2 * sizeof(Float3));

/// This is a copy of AxisConstraintPart, specialized to handle contact constraints. See the documentation of AxisConstraintPart for more documentation behind the math.
/// Warning: Make sure there is 1 float of padding after this class because we read using Vec3::sLoadFloat3Unsafe and a Float3 is the last member.
template <EMotionType Type1, EMotionType Type2>
class ContactConstraintPart : public ContactConstraintPart1<Type1>, public ContactConstraintPart2<Type2>
{
private:
	/// See AxisConstraintPart::ApplyVelocityStep
	JPH_INLINE bool				ApplyVelocityStep(MotionProperties *ioMotionProperties1, float inInvMass1, MotionProperties *ioMotionProperties2, float inInvMass2, Vec3Arg inWorldSpaceAxis, float inLambda) const
	{
		if (inLambda != 0.0f)
		{
			if constexpr (Type1 == EMotionType::Dynamic)
			{
				ioMotionProperties1->SubLinearVelocityStep((inLambda * inInvMass1) * inWorldSpaceAxis);
				ioMotionProperties1->SubAngularVelocityStep(inLambda * Vec3::sLoadFloat3Unsafe(this->mInvI1_R1PlusUxAxis));
			}
			if constexpr (Type2 == EMotionType::Dynamic)
			{
				ioMotionProperties2->AddLinearVelocityStep((inLambda * inInvMass2) * inWorldSpaceAxis);
				ioMotionProperties2->AddAngularVelocityStep(inLambda * Vec3::sLoadFloat3Unsafe(this->mInvI2_R2xAxis));
			}
			return true;
		}

		return false;
	}

public:
	/// See AxisConstraintPart::CalculateConstraintProperties
	JPH_INLINE void				CalculateConstraintProperties(float inInvMass1, Mat44Arg inInvI1, Vec3Arg inR1PlusU, float inInvMass2, Mat44Arg inInvI2, Vec3Arg inR2, Vec3Arg inWorldSpaceAxis, float inBias = 0.0f)
	{
		JPH_ASSERT(inWorldSpaceAxis.IsNormalized(1.0e-5f));
				
		// Calculate inverse effective mass: K = J M^-1 J^T
		float inv_effective_mass;

		if constexpr (Type1 != EMotionType::Static)
		{
			Vec3 r1_plus_u_x_axis = inR1PlusU.Cross(inWorldSpaceAxis);
			r1_plus_u_x_axis.StoreFloat3(&this->mR1PlusUxAxis);

			if constexpr (Type1 == EMotionType::Dynamic)
			{
				Vec3 invi1_r1_plus_u_x_axis = inInvI1.Multiply3x3(r1_plus_u_x_axis);
				invi1_r1_plus_u_x_axis.StoreFloat3(&this->mInvI1_R1PlusUxAxis);

				inv_effective_mass = inInvMass1 + invi1_r1_plus_u_x_axis.Dot(r1_plus_u_x_axis);
			}
			else
				inv_effective_mass = 0.0f;
		}
		else
			inv_effective_mass = 0.0f;

		if constexpr (Type2 != EMotionType::Static)
		{
			Vec3 r2_x_axis = inR2.Cross(inWorldSpaceAxis);
			r2_x_axis.StoreFloat3(&this->mR2xAxis);

			if constexpr (Type2 == EMotionType::Dynamic)
			{
				Vec3 invi2_r2_x_axis = inInvI2.Multiply3x3(r2_x_axis);
				invi2_r2_x_axis.StoreFloat3(&this->mInvI2_R2xAxis);

				inv_effective_mass += inInvMass2 + invi2_r2_x_axis.Dot(r2_x_axis);
			}
		}

		if (inv_effective_mass== 0.0f)
			this->Deactivate();
		else
		{
			this->mEffectiveMass = 1.0f / inv_effective_mass;
			this->mBias = inBias;
		}
	}

	/// See AxisConstraintPart::WarmStart
	inline void					WarmStart(MotionProperties *ioMotionProperties1, float inInvMass1, MotionProperties *ioMotionProperties2, float inInvMass2, Vec3Arg inWorldSpaceAxis, float inWarmStartImpulseRatio)
	{
		this->mTotalLambda *= inWarmStartImpulseRatio;

		ApplyVelocityStep(ioMotionProperties1, inInvMass1, ioMotionProperties2, inInvMass2, inWorldSpaceAxis, this->mTotalLambda);
	}

	/// Part 1 of AxisConstraint::SolveVelocityConstraint: get the total lambda
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
			jv += Vec3::sLoadFloat3Unsafe(this->mR1PlusUxAxis).Dot(ioMotionProperties1->GetAngularVelocity());
		if constexpr (Type2 != EMotionType::Static)
			jv -= Vec3::sLoadFloat3Unsafe(this->mR2xAxis).Dot(ioMotionProperties2->GetAngularVelocity());

		// Lagrange multiplier is:
		//
		// lambda = -K^-1 (J v + b)
		float lambda = this->mEffectiveMass * (jv - this->mBias);

		// Return the total accumulated lambda
		return this->mTotalLambda + lambda;
	}

	/// Part 2 of AxisConstraint::SolveVelocityConstraint: apply new lambda
	JPH_INLINE bool				SolveVelocityConstraintApplyLambda(MotionProperties *ioMotionProperties1, float inInvMass1, MotionProperties *ioMotionProperties2, float inInvMass2, Vec3Arg inWorldSpaceAxis, float inTotalLambda)
	{
		float delta_lambda = inTotalLambda - this->mTotalLambda; // Calculate change in lambda
		this->mTotalLambda = inTotalLambda; // Store accumulated impulse

		return ApplyVelocityStep(ioMotionProperties1, inInvMass1, ioMotionProperties2, inInvMass2, inWorldSpaceAxis, delta_lambda);
	}

	/// See: AxisConstraintPart::SolveVelocityConstraint
	JPH_INLINE bool				SolveVelocityConstraint(MotionProperties *ioMotionProperties1, float inInvMass1, MotionProperties *ioMotionProperties2, float inInvMass2, Vec3Arg inWorldSpaceAxis, float inMinLambda, float inMaxLambda)
	{
		float total_lambda = SolveVelocityConstraintGetTotalLambda(ioMotionProperties1, ioMotionProperties2, inWorldSpaceAxis);

		// Clamp impulse to specified range
		total_lambda = Clamp(total_lambda, inMinLambda, inMaxLambda);

		return SolveVelocityConstraintApplyLambda(ioMotionProperties1, inInvMass1, ioMotionProperties2, inInvMass2, inWorldSpaceAxis, total_lambda);
	}

	/// See: AxisConstraintPart::SolvePositionConstraint
	inline bool					SolvePositionConstraint(Body &ioBody1, float inInvMass1, Body &ioBody2, float inInvMass2, Vec3Arg inWorldSpaceAxis, float inC, float inBaumgarte) const
	{
		if (inC != 0.0f)
		{
			float lambda = -this->mEffectiveMass * inBaumgarte * inC;
			if constexpr (Type1 == EMotionType::Dynamic)
			{
				ioBody1.SubPositionStep((lambda * inInvMass1) * inWorldSpaceAxis);
				ioBody1.SubRotationStep(lambda * Vec3::sLoadFloat3Unsafe(this->mInvI1_R1PlusUxAxis));
			}
			if constexpr (Type2 == EMotionType::Dynamic)
			{
				ioBody2.AddPositionStep((lambda * inInvMass2) * inWorldSpaceAxis);
				ioBody2.AddRotationStep(lambda * Vec3::sLoadFloat3Unsafe(this->mInvI2_R2xAxis));
			}
			return true;
		}

		return false;
	}
};

static_assert(sizeof(ContactConstraintPart<EMotionType::Dynamic, EMotionType::Dynamic>) == 3 * sizeof(float) + 4 * sizeof(Float3));
static_assert(sizeof(ContactConstraintPart<EMotionType::Dynamic, EMotionType::Kinematic>) == 3 * sizeof(float) + 3 * sizeof(Float3));
static_assert(sizeof(ContactConstraintPart<EMotionType::Dynamic, EMotionType::Static>) == 3 * sizeof(float) + 2 * sizeof(Float3));
static_assert(sizeof(ContactConstraintPart<EMotionType::Kinematic, EMotionType::Kinematic>) == 3 * sizeof(float) + 2 * sizeof(Float3));
static_assert(sizeof(ContactConstraintPart<EMotionType::Kinematic, EMotionType::Static>) == 3 * sizeof(float) + sizeof(Float3));
static_assert(sizeof(ContactConstraintPart<EMotionType::Static, EMotionType::Dynamic>) == 3 * sizeof(float) + 2 * sizeof(Float3));
static_assert(sizeof(ContactConstraintPart<EMotionType::Static, EMotionType::Kinematic>) == 3 * sizeof(float) + sizeof(Float3));
static_assert(sizeof(ContactConstraintPart<EMotionType::Static, EMotionType::Static>) == 3 * sizeof(float));

/// Concrete contact constraint part that dispatches to the correct templated form based on the motion types of the bodies
class ConcreteContactConstraintPart
{
public:
	/// Constructor / destructor
								ConcreteContactConstraintPart() : mDD() { }
								~ConcreteContactConstraintPart() = default;

	inline bool					SolveVelocityConstraint(Body &inBody1, float inInvMass1, float inInvInertiaScale1, Vec3Arg inR1PlusU, Body &inBody2, float inInvMass2, float inInvInertiaScale2, Vec3Arg inR2, Vec3Arg inWorldSpaceAxis, float inBias, float inMinLambda, float inMaxLambda)
	{
		// Dispatch to the correct templated form
		switch (inBody1.GetMotionType())
		{
		case EMotionType::Dynamic:
			{
				Mat44 inv_i1 = inInvInertiaScale1 * inBody1.GetInverseInertia();
				switch (inBody2.GetMotionType())
				{
				case EMotionType::Dynamic:
					mDD.CalculateConstraintProperties(inInvMass1, inv_i1, inR1PlusU, inInvMass2, inInvInertiaScale2 * inBody2.GetInverseInertia(), inR2, inWorldSpaceAxis, inBias);
					return mDD.SolveVelocityConstraint(inBody1.GetMotionPropertiesUnchecked(), inInvMass1, inBody2.GetMotionPropertiesUnchecked(), inInvMass2, inWorldSpaceAxis, inMinLambda, inMaxLambda);
	
				case EMotionType::Kinematic:
					mDK.CalculateConstraintProperties(inInvMass1, inv_i1, inR1PlusU, 0 /* Will not be used */, Mat44() /* Will not be used */, inR2, inWorldSpaceAxis, inBias);
					return mDK.SolveVelocityConstraint(inBody1.GetMotionPropertiesUnchecked(), inInvMass1, inBody2.GetMotionPropertiesUnchecked(), inInvMass2, inWorldSpaceAxis, inMinLambda, inMaxLambda);

				case EMotionType::Static:
					mDS.CalculateConstraintProperties(inInvMass1, inv_i1, inR1PlusU, 0 /* Will not be used */, Mat44() /* Will not be used */, inR2, inWorldSpaceAxis, inBias);
					return mDS.SolveVelocityConstraint(inBody1.GetMotionPropertiesUnchecked(), inInvMass1, nullptr /* Unused */, 0.0f /* Unused */, inWorldSpaceAxis, inMinLambda, inMaxLambda);

				default:
					JPH_ASSERT(false);
					break;
				}
				break;
			}

		case EMotionType::Kinematic:
			JPH_ASSERT(inBody2.IsDynamic());
			mKD.CalculateConstraintProperties(0 /* Will not be used */, Mat44() /* Will not be used */, inR1PlusU, inInvMass2, inInvInertiaScale2 * inBody2.GetInverseInertia(), inR2, inWorldSpaceAxis, inBias);
			return mKD.SolveVelocityConstraint(inBody1.GetMotionPropertiesUnchecked(), inInvMass1, inBody2.GetMotionPropertiesUnchecked(), inInvMass2, inWorldSpaceAxis, inMinLambda, inMaxLambda);

		case EMotionType::Static:
			JPH_ASSERT(inBody2.IsDynamic());
			mSD.CalculateConstraintProperties(0 /* Will not be used */, Mat44() /* Will not be used */, inR1PlusU, inInvMass2, inInvInertiaScale2 * inBody2.GetInverseInertia(), inR2, inWorldSpaceAxis, inBias);
			return mSD.SolveVelocityConstraint(nullptr /* Unused */, 0.0f /* Unused */, inBody2.GetMotionPropertiesUnchecked(), inInvMass2, inWorldSpaceAxis, inMinLambda, inMaxLambda);

		default:
			JPH_ASSERT(false);
			break;
		}

		return false;
	}

	inline float				GetTotalLambda() const
	{
		return mDD.GetTotalLambda();
	}

private:
	union
	{
		ContactConstraintPart<EMotionType::Dynamic, EMotionType::Dynamic>	mDD;
		ContactConstraintPart<EMotionType::Dynamic, EMotionType::Kinematic>	mDK;
		ContactConstraintPart<EMotionType::Dynamic, EMotionType::Static>	mDS;
		ContactConstraintPart<EMotionType::Kinematic, EMotionType::Dynamic>	mKD;
		ContactConstraintPart<EMotionType::Static, EMotionType::Dynamic>	mSD;
	};
	[[maybe_unused]] float		mPadding; // Pad an extra float so that we can use Vec3::sLoadFloat3Unsafe on the last Float3 member without worrying about reading past the end of the struct
};

JPH_NAMESPACE_END
