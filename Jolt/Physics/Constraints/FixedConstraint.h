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

	// See: ConstraintSettings::SaveBinaryState
	virtual void				SaveBinaryState(StreamOut &inStream) const override;

	/// Create an an instance of this constraint
	virtual TwoBodyConstraint *	Create(Body &inBody1, Body &inBody2) const override;

	/// Simple way of calculating the parameters below to fixate two bodies in their current relative position/orientation
	void						SetPoint(const Body &inBody1, const Body &inBody2);

	/// This determines in which space the constraint is setup, all properties below should be in the specified space
	EConstraintSpace			mSpace = EConstraintSpace::WorldSpace;

	/// Body 1 constraint reference frame (space determined by mSpace)
	Vec3						mPoint1 = Vec3::sZero();
	Vec3						mAxisX1 = Vec3::sAxisX();
	Vec3						mAxisY1 = Vec3::sAxisY();

	/// Body 2 constraint reference frame (space determined by mSpace)
	Vec3						mPoint2 = Vec3::sZero();
	Vec3						mAxisX2 = Vec3::sAxisX();
	Vec3						mAxisY2 = Vec3::sAxisY();

protected:
	// See: ConstraintSettings::RestoreBinaryState
	virtual void				RestoreBinaryState(StreamIn &inStream) override;
};

/// A fixed constraint welds two bodies together removing all degrees of freedom between them.
/// This variant uses euler angles for the rotation constraint.
class FixedConstraint final : public TwoBodyConstraint
{
public:
	/// Constructor
								FixedConstraint(Body &inBody1, Body &inBody2, const FixedConstraintSettings &inSettings);

	// Generic interface of a constraint
	virtual EConstraintSubType	GetSubType() const override									{ return EConstraintSubType::Fixed; }
	virtual void				SetupVelocityConstraint(float inDeltaTime) override;
	virtual void				WarmStartVelocityConstraint(float inWarmStartImpulseRatio) override;
	virtual bool				SolveVelocityConstraint(float inDeltaTime) override;
	virtual bool				SolvePositionConstraint(float inDeltaTime, float inBaumgarte) override;
#ifdef JPH_DEBUG_RENDERER
	virtual void				DrawConstraint(DebugRenderer *inRenderer) const override;
#endif // JPH_DEBUG_RENDERER
	virtual void				SaveState(StateRecorder &inStream) const override;
	virtual void				RestoreState(StateRecorder &inStream) override;
	virtual Ref<ConstraintSettings> GetConstraintSettings() const override;

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
