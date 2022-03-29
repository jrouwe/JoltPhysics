// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Physics/Constraints/TwoBodyConstraint.h>
#include <Jolt/Physics/Constraints/ConstraintPart/RotationEulerConstraintPart.h>
#include <Jolt/Physics/Constraints/ConstraintPart/PointConstraintPart.h>

JPH_NAMESPACE_BEGIN

/// Fixed constraint settings, used to create a fixed constraint
class FixedConstraintSettings final : public TwoBodyConstraintSettings
{
public:
	JPH_DECLARE_SERIALIZABLE_VIRTUAL(FixedConstraintSettings)

	/// Create an an instance of this constraint
	virtual TwoBodyConstraint *	Create(Body &inBody1, Body &inBody2) const override;
};

/// A fixed constraint welds two bodies together removing all degrees of freedom between them.
/// This variant uses euler angles for the rotation constraint.
class FixedConstraint final : public TwoBodyConstraint
{
public:
	/// Constructor
								FixedConstraint(Body &inBody1, Body &inBody2, const FixedConstraintSettings &inSettings);

	// Generic interface of a constraint
	virtual EConstraintType		GetType() const override					{ return EConstraintType::Fixed; }
	virtual void				SetupVelocityConstraint(float inDeltaTime) override;
	virtual void				WarmStartVelocityConstraint(float inWarmStartImpulseRatio) override;
	virtual bool				SolveVelocityConstraint(float inDeltaTime) override;
	virtual bool				SolvePositionConstraint(float inDeltaTime, float inBaumgarte) override;
#ifdef JPH_DEBUG_RENDERER
	virtual void				DrawConstraint(DebugRenderer *inRenderer) const override;
#endif // JPH_DEBUG_RENDERER
	virtual void				SaveState(StateRecorder &inStream) const override;
	virtual void				RestoreState(StateRecorder &inStream) override;

	// See: TwoBodyConstraint
	virtual Mat44				GetConstraintToBody1Matrix() const override					{ return Mat44::sTranslation(mLocalSpacePosition1); }
	virtual Mat44				GetConstraintToBody2Matrix() const override					{ return Mat44::sRotationTranslation(mInvInitialOrientation, mLocalSpacePosition2); }

	///@name Get Lagrange multiplier from last physics update (relates to how much force/torque was applied to satisfy the constraint)
	inline Vec3					GetTotalLambdaPosition() const								{ return mPointConstraintPart.GetTotalLambda(); }
	inline Vec3					GetTotalLambdaRotation() const								{ return mRotationConstraintPart.GetTotalLambda(); }

private:
	// CONFIGURATION PROPERTIES FOLLOW
		
	// Local space constraint positions
	Vec3						mLocalSpacePosition1;
	Vec3						mLocalSpacePosition2;

	// Inverse of initial rotation from body 1 to body 2 in body 1 space
	Quat						mInvInitialOrientation;

	// RUN TIME PROPERTIES FOLLOW

	// The constraint parts
	RotationEulerConstraintPart	mRotationConstraintPart;
	PointConstraintPart			mPointConstraintPart;
};

JPH_NAMESPACE_END
